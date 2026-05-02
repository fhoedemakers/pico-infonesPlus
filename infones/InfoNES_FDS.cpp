/*===================================================================*/
/*                                                                   */
/*  InfoNES_FDS.cpp : Famicom Disk System support                    */
/*                                                                   */
/*  Phase 1: detection, preflight gate (RP2350 + PSRAM + BIOS file + */
/*  memory), buffer allocation, BIOS load. Mapper 20 init, drive     */
/*  emulation, disk swap UI, save-data flush, audio and save-states  */
/*  are added in later phases.                                       */
/*                                                                   */
/*===================================================================*/

#include <string.h>
#include <stdio.h>

#include "InfoNES_FDS.h"
#include "InfoNES.h"
#include "InfoNES_System.h"
#include "FrensHelpers.h"
#include "ff.h"
#include "K6502.h"

/*-------------------------------------------------------------------*/
/*  FDS state                                                        */
/*-------------------------------------------------------------------*/

BYTE *FDS_Bios       = nullptr;
BYTE *FDS_PrgRam     = nullptr;
BYTE *FDS_ChrRam     = nullptr;
BYTE *FDS_DiskImage  = nullptr;
int   FDS_NumSides   = 0;
int   FDS_CurrentSide = 0;
bool  FDS_DiskInserted = true;

/*-------------------------------------------------------------------*/
/*  Disk drive emulation state                                       */
/*-------------------------------------------------------------------*/
/*
 *  Disk timing: a real FDS reads ~10.4k bytes/s, which on NTSC CPU
 *  (1.789773 MHz) is ~149 cycles/byte. We accumulate STEP_PER_SCANLINE
 *  CPU cycles per HSync and emit a disk byte every FDS_CYC_PER_BYTE.
 */
#define FDS_CYC_PER_BYTE  149

/* $4020/$4021 timer reload ($4022 starts/stops). */
static WORD fds_timer_reload   = 0;
static WORD fds_timer_counter  = 0;
static BYTE fds_timer_irq_en   = 0;   /* $4022 bit 1 */
static BYTE fds_timer_repeat   = 0;   /* $4022 bit 0 */
static BYTE fds_timer_irq_pend = 0;

/* $4023 master enable. Bit 0 = disk regs, bit 1 = sound regs. */
static BYTE fds_master_enable  = 0;

/* $4025 disk control bits, decoded. */
static BYTE fds_motor_on       = 0;
static BYTE fds_xfer_reset     = 1;   /* hold at gap until BIOS releases */
static BYTE fds_read_mode      = 1;
static BYTE fds_byte_irq_en    = 0;
static BYTE fds_scan_start     = 0;   /* $4025 bit 6 (== Mesen2's _diskReady) */
static BYTE fds_crc_control    = 0;   /* $4025 bit 4 — drive emits/checks CRC */
/* fds_gap_ended is defined later, alongside the linear-walk model. */
/* Mesen2 behavior: once the drive has *started* a scan since the last
   motor toggle, rst=1 alone no longer pauses it. BIOS file-scan
   routines (e.g. Metroid) rely on this — they set rst=1 + scan=1 to
   fast-forward past blocks they don't want, and would deadlock if
   rst=1 stopped the drive cold. Set this on the first produced byte
   and clear it on motor 0→1. */
static BYTE fds_scanning_started = 0;

/* Disk transfer state. */
static DWORD fds_byte_pos        = 0;  /* payload index inside current side */
static DWORD fds_block_start_pos = 0;  /* byte_pos at start of current block */
static BYTE  fds_read_buf      = 0;
static BYTE  fds_byte_irq_pend = 0;
static BYTE  fds_end_of_head   = 0;
static WORD  fds_cycle_acc     = 0;
static int   fds_motor_spinup  = 0;   /* scanlines to wait after motor on */

/* $4024 staged write byte (real write-back lands in phase 6). */
static BYTE  fds_write_buf     = 0;

/* Phase 5: disk swap state. fds_eject_counter is in HSync ticks; while
   it is positive, FDS_DiskInserted is forced to false so the BIOS
   sees disk-not-inserted at $4032 bit 0. When it reaches zero, we
   re-insert with fds_pending_side as the new FDS_CurrentSide. -1 in
   fds_pending_side means "stay ejected". */
#define FDS_SWAP_EJECT_HSYNCS  (60 * 262)  /* ~1 s of NTSC hsyncs */
static int   fds_eject_counter = 0;
static int   fds_pending_side  = -1;

/* Mesen2-style expanded disk image. The raw .fds file contains only
   block payloads back to back; the BIOS expects a richer byte stream
   that includes inter-block gaps and sync marks so its file-scan
   routine can fast-forward (rst=1 + scan=1) past blocks it doesn't
   want. We build that expanded layout once at load time and walk it
   linearly at runtime — no phase tracking, no auto-transitions.

     Per side: [3537 zero gap]
               for each block: [0x80 mark][payload][0x4D 0x62 fake CRC]
                                [122 zero gap]
*/
#define FDS_INITIAL_GAP_BYTES  3537   /* 28300 bits / 8 */
#define FDS_INTER_BLOCK_GAP    122    /* 976 bits / 8 */
#define FDS_MARK_BYTE          0x80
#define FDS_FAKE_CRC_HI        0x4D
#define FDS_FAKE_CRC_LO        0x62

static BYTE  *fds_expanded            = nullptr;
static DWORD  fds_expanded_total_size = 0;
static DWORD  fds_side_offsets[FDS_MAX_SIDES] = {0};
static DWORD  fds_side_sizes[FDS_MAX_SIDES]   = {0};

static inline BYTE *fdsCurrentSideBuf()
{
  return fds_expanded ? fds_expanded + fds_side_offsets[FDS_CurrentSide] : nullptr;
}

static inline DWORD fdsCurrentSideSize()
{
  return fds_side_sizes[FDS_CurrentSide];
}

/* Phase 6: dirty-page bitmap for save-data writeback. Per 4 KB page
   on the EXPANDED image (not the raw .fds), since that's what we
   actually mutate at runtime. */
#define FDS_PAGE_SIZE         4096
/* Worst-case expanded total: 8 sides * (3537 + ~30 blocks * 125 + 65500)
   ≈ 600 KB. 152 pages covers it with margin. */
#define FDS_MAX_PAGES         160
static BYTE fds_dirty_bitmap[(FDS_MAX_PAGES + 7) / 8] = {0};

static inline void fdsMarkDirty(DWORD byteOffset)
{
  unsigned page = (unsigned)(byteOffset / FDS_PAGE_SIZE);
  if (page >= (unsigned)FDS_MAX_PAGES) return;
  fds_dirty_bitmap[page >> 3] |= (BYTE)(1u << (page & 7));
}

static inline bool fdsPageDirty(unsigned page)
{
  if (page >= (unsigned)FDS_MAX_PAGES) return false;
  return (fds_dirty_bitmap[page >> 3] >> (page & 7)) & 1u;
}

static inline void fdsClearDirtyBitmap()
{
  for (size_t i = 0; i < sizeof(fds_dirty_bitmap); ++i)
    fds_dirty_bitmap[i] = 0;
}

/* Tracing knobs. The full per-byte trace floods the UART so hard
   it stalls the emulator; the slim trace only emits block-boundary
   transitions and $4025 writes — that's enough to diagnose ERR.24
   without overwhelming the UART. */
#define FDS_TRACE      0   /* slim: block#X / 4025 / END */
#define FDS_TRACE_BYTE 0   /* per-byte read/write — only enable for deep dives */
#if FDS_TRACE
#define FDS_LOG(...) printf("[FDS] " __VA_ARGS__)
#else
#define FDS_LOG(...) do {} while (0)
#endif
#if FDS_TRACE_BYTE
#define FDS_BYTELOG(...) printf("[FDS] " __VA_ARGS__)
#else
#define FDS_BYTELOG(...) do {} while (0)
#endif

static int fds_block_count = 0;

/*
 *  Linear-walk drive emulation (Mesen2-style).
 *
 *  fdsBuildExpandedBuffer() expands the raw .fds at load time into a
 *  wire-format buffer that contains gaps and fake CRC slots. The
 *  drive walks this buffer linearly — one byte per FDS_CYC_PER_BYTE
 *  cycles. No phase machine, no auto-transitions.
 *
 *  Per side:
 *    [FDS_INITIAL_GAP_BYTES zeros]
 *    For each block: [payload from .fds][2 fake CRC bytes][FDS_INTER_BLOCK_GAP zeros]
 *
 *  fds_block_starts[s][i] holds the offset (within side s) of the
 *  i-th block's first PAYLOAD byte (the block-type byte), used by
 *  the drive to know where gap zeros end and emission begins.
 *
 *  fds_gap_ended distinguishes "walking through gap zeros silently"
 *  (false) from "emitting bytes via byte-ready IRQ" (true). It flips
 *  to true the moment pos lands on a known block-start offset, and
 *  back to false on rst=1 (the BIOS's bit-synchroniser reset).
 *
 *  We deliberately do NOT place a 0x80 sync mark byte in the buffer:
 *  the FDS BIOS waits for the first non-zero byte after a gap as the
 *  block's type byte (0x01-0x04). A literal 0x80 between gap and
 *  payload would be read as an invalid block type → ERR.23.
 */
#define FDS_MAX_BLOCKS_PER_SIDE 80

static DWORD fds_block_starts[FDS_MAX_SIDES][FDS_MAX_BLOCKS_PER_SIDE] = {{0}};
/* fds_block_ends[s][i] = position one past the last CRC byte of block i.
   When emit advances pos to this value, we've just finished block i — flip
   gap_ended=0 so the inter-block gap zeros are walked silently rather than
   emitted to BIOS as bogus data bytes. */
static DWORD fds_block_ends[FDS_MAX_SIDES][FDS_MAX_BLOCKS_PER_SIDE] = {{0}};
static int   fds_block_counts[FDS_MAX_SIDES] = {0};

static BYTE  fds_gap_ended = 0;   /* 0 = walking through gap zeros + 0x80 mark, 1 = emitting */

/*-------------------------------------------------------------------*/
/*  fdsBuildExpandedBuffer: walk the raw .fds for each side, emit    */
/*  the wire-format expanded buffer with gaps + marks + CRC slots,   */
/*  and record per-side block-start offsets for seek operations.     */
/*-------------------------------------------------------------------*/
static bool fdsBuildExpandedBuffer(BYTE *rawDisk, int sides)
{
  if (fds_expanded) { Frens::f_free(fds_expanded); fds_expanded = nullptr; }

  /* Worst case: each side adds initial gap + (gap+mark+payload+CRC) per block.
     Side payload is at most FDS_SIDE_SIZE bytes, plus per-block overhead of
     ~125 bytes for ~30 blocks = ~3750 bytes. Add initial gap = ~7500 bytes
     of overhead per side. Allocate FDS_SIDE_SIZE + 8KB per side to be safe. */
  DWORD perSideMax = FDS_SIDE_SIZE + 8192;
  DWORD totalMax   = (DWORD)sides * perSideMax;

  fds_expanded = (BYTE *)Frens::f_malloc(totalMax);
  if (!fds_expanded)
  {
    InfoNES_Error("FDS: cannot allocate %u bytes for expanded buffer", (unsigned)totalMax);
    return false;
  }
  memset(fds_expanded, 0, totalMax);

  DWORD writePos = 0;
  for (int s = 0; s < sides; ++s)
  {
    fds_side_offsets[s] = writePos;
    DWORD sideStart = writePos;

    /* Initial gap. */
    writePos += FDS_INITIAL_GAP_BYTES;

    DWORD srcBase = (DWORD)s * FDS_SIDE_SIZE;
    DWORD srcPos  = 0;
    int   blkIdx  = 0;
    WORD  lastFileSize = 0;

    while (srcPos < FDS_SIDE_SIZE && blkIdx < FDS_MAX_BLOCKS_PER_SIDE)
    {
      BYTE type = rawDisk[srcBase + srcPos];
      int  blockSize = 0;
      switch (type)
      {
      case 0x01: blockSize = 56; break;
      case 0x02: blockSize = 2;  break;
      case 0x03:
        blockSize = 16;
        if (srcPos + 14 < FDS_SIDE_SIZE)
        {
          lastFileSize = rawDisk[srcBase + srcPos + 13] |
                         ((WORD)rawDisk[srcBase + srcPos + 14] << 8);
        }
        break;
      case 0x04: blockSize = 1 + lastFileSize; break;
      default:
        /* End of valid blocks (rest of side is unallocated 0x00 filler). */
        goto side_done;
      }

      if ((DWORD)(srcPos + blockSize) > FDS_SIDE_SIZE) goto side_done;

      /* Record block-start offset (points at first payload byte). */
      fds_block_starts[s][blkIdx] = writePos - sideStart;

      /* Block payload. */
      memcpy(&fds_expanded[writePos], &rawDisk[srcBase + srcPos], (size_t)blockSize);
      writePos += blockSize;
      srcPos   += blockSize;

      /* 2 fake CRC bytes. */
      fds_expanded[writePos++] = FDS_FAKE_CRC_HI;
      fds_expanded[writePos++] = FDS_FAKE_CRC_LO;

      /* Record end offset: one past the last CRC byte. */
      fds_block_ends[s][blkIdx] = writePos - sideStart;
      blkIdx++;

      /* Inter-block gap zeros (the buffer is already zeroed). */
      writePos += FDS_INTER_BLOCK_GAP;
    }
  side_done:
    fds_block_counts[s] = blkIdx;
    fds_side_sizes[s]   = writePos - sideStart;

    /* Pad the rest of this side's reserved space so the next side starts
       at a known offset (sideStart + perSideMax). Then bump writePos to
       the next side boundary for clean addressing. */
    writePos = sideStart + perSideMax;

    FDS_LOG("side %d: %d block(s), expanded size %lu B\n",
            s, blkIdx, (unsigned long)fds_side_sizes[s]);
  }

  fds_expanded_total_size = (DWORD)sides * perSideMax;
  return true;
}

/*-------------------------------------------------------------------*/
/*  Helpers                                                          */
/*-------------------------------------------------------------------*/

static inline void fdsResetTransferPosition()
{
  fds_byte_pos = 0;
  fds_block_start_pos = 0;
  fds_cycle_acc = 0;
  fds_end_of_head = 0;
  fds_gap_ended = 0;
  fds_scanning_started = 0;
}

bool fdsIsFdsFilename(const char *filename)
{
  if (!filename) return false;
  char ext[8];
  Frens::getextensionfromfilename(filename, ext, sizeof(ext));
  return strcasecmp(ext, ".fds") == 0;
}

/*-------------------------------------------------------------------*/
/*  Preflight checks                                                 */
/*-------------------------------------------------------------------*/

#if PICO_RP2350
static bool fdsBiosFileOk(size_t *outSize)
{
  FILINFO fno;
  FRESULT fr = f_stat(FDS_BIOS_PATH, &fno);
  if (fr != FR_OK )
  {
    InfoNES_Error("No FDS BIOS at %s", FDS_BIOS_PATH);
    return false;
  }
  if (fno.fsize != FDS_BIOS_SIZE )
  {
    InfoNES_Error("FDS BIOS must be %d bytes (is %lu)",
                  (int)FDS_BIOS_SIZE, (unsigned long)fno.fsize);
    return false;
  }
  if (outSize) *outSize = (size_t)fno.fsize;
  return true;
}

static bool fdsLoadBios(BYTE *dst)
{
  FIL fil;
  FRESULT fr = f_open(&fil, FDS_BIOS_PATH, FA_READ);
  if (fr != FR_OK)
  {
    InfoNES_Error("Cannot open FDS BIOS (%d)", fr);
    return false;
  }
  UINT br = 0;
  fr = f_read(&fil, dst, FDS_BIOS_SIZE, &br);
  f_close(&fil);
  if (fr != FR_OK || br != FDS_BIOS_SIZE)
  {
    InfoNES_Error("Failed reading FDS BIOS (%d)", fr);
    return false;
  }
  return true;
}
#endif

/*-------------------------------------------------------------------*/
/*  fdsParse: gate + allocate + load BIOS                            */
/*-------------------------------------------------------------------*/

bool fdsParse(BYTE *fdsImage, size_t fdsImageSize)
{
  /* Always start from a clean slate so a previous failed launch leaves
     no dangling pointers. */
  fdsRelease();
  IsFDS = false;
  FDS_DiskImage = nullptr;
  FDS_NumSides = 0;
  FDS_CurrentSide = 0;

  /* Zero NesHeader so InfoNES_SetupPPU sees byVRomSize == 0 and
     enables CHR-RAM writes. Otherwise leftover values from a prior
     iNES game would silently drop PPU $0000-$1FFF writes. */
  memset(&NesHeader, 0, sizeof(NesHeader));

#if !PICO_RP2350
  (void)fdsImage; (void)fdsImageSize;
  InfoNES_Error("FDS support requires RP2350");
  return false;
#else
  if (!Frens::isPsramEnabled())
  {
    InfoNES_Error("FDS support requires PSRAM");
    return false;
  }

  if (!fdsImage || fdsImageSize == 0)
  {
    InfoNES_Error("FDS image is empty");
    return false;
  }

  /* Strip optional 16-byte fwNES header. */
  BYTE *diskBytes = fdsImage;
  size_t diskSize = fdsImageSize;
  if (diskSize >= FDS_FWNES_HDR_SIZE && memcmp(diskBytes, "FDS\x1a", 4) == 0)
  {
    diskBytes += FDS_FWNES_HDR_SIZE;
    diskSize  -= FDS_FWNES_HDR_SIZE;
  }

  if (diskSize == 0 || (diskSize % FDS_SIDE_SIZE) != 0)
  {
    InfoNES_Error("FDS image size %u is not a multiple of %d",
                  (unsigned)diskSize, (int)FDS_SIDE_SIZE);
    return false;
  }
  int sides = (int)(diskSize / FDS_SIDE_SIZE);
  if (sides < 1 || sides > FDS_MAX_SIDES)
  {
    InfoNES_Error("FDS image has unsupported side count %d", sides);
    return false;
  }

  size_t need = FDS_BIOS_SIZE + FDS_PRG_RAM_SIZE + FDS_CHR_RAM_SIZE;
  uint avail = Frens::GetAvailableMemory();
  if (avail < need)
  {
    InfoNES_Error("Not enough PSRAM for FDS (%u < %u)",
                  (unsigned)avail, (unsigned)need);
    return false;
  }

  if (!fdsBiosFileOk(nullptr))
  {
    /* InfoNES_Error already populated. */
    return false;
  }

  FDS_Bios   = (BYTE *)Frens::f_malloc(FDS_BIOS_SIZE);
  FDS_PrgRam = (BYTE *)Frens::f_malloc(FDS_PRG_RAM_SIZE);
  FDS_ChrRam = (BYTE *)Frens::f_malloc(FDS_CHR_RAM_SIZE);
  if (!FDS_Bios || !FDS_PrgRam || !FDS_ChrRam)
  {
    InfoNES_Error("FDS PSRAM allocation failed");
    fdsRelease();
    return false;
  }

  memset(FDS_PrgRam, 0, FDS_PRG_RAM_SIZE);
  memset(FDS_ChrRam, 0, FDS_CHR_RAM_SIZE);

  if (!fdsLoadBios(FDS_Bios))
  {
    fdsRelease();
    return false;
  }

  FDS_DiskImage = diskBytes;
  FDS_NumSides = sides;
  FDS_CurrentSide = 0;

  if (!fdsBuildExpandedBuffer(diskBytes, sides))
  {
    fdsRelease();
    return false;
  }

  IsFDS = true;
  fds_eject_counter = 0;
  fds_pending_side  = -1;
  fdsClearDirtyBitmap();

  printf("FDS: image %u bytes, %d side(s); PSRAM avail before alloc %u, after %u\n",
         (unsigned)diskSize, sides, (unsigned)avail,
         (unsigned)Frens::GetAvailableMemory());
  return true;
#endif
}

/*-------------------------------------------------------------------*/
/*  Disk swap UI hooks (phase 5).                                    */
/*-------------------------------------------------------------------*/
void fdsRequestSwap(int newSide)
{
  if (!IsFDS) return;
  if (newSide < 0 || newSide >= FDS_NumSides) return;
  /* Hold the disk ejected for ~1 s of game time, then re-insert with
     the requested side. The BIOS samples $4032 bit 0 to detect the
     disk being pulled and re-seated; without this pulse it would not
     re-read the file table for the new side. */
  FDS_DiskInserted = false;
  fds_pending_side = newSide;
  fds_eject_counter = FDS_SWAP_EJECT_HSYNCS;
}

void fdsRequestEject()
{
  if (!IsFDS) return;
  FDS_DiskInserted = false;
  fds_pending_side = -1;
  fds_eject_counter = 0;
}

int fdsCurrentSwapValue()
{
  if (!IsFDS) return 0;
  /* While the eject pulse is in flight, report the pending target so
     the menu reflects the user's choice immediately rather than
     flicking to "Eject" for the duration of the pulse. After the
     pulse resolves, FDS_CurrentSide is the source of truth. */
  if (fds_eject_counter > 0 && fds_pending_side >= 0 &&
      fds_pending_side < FDS_NumSides)
  {
    return fds_pending_side;
  }
  if (!FDS_DiskInserted) return FDS_NumSides;
  return FDS_CurrentSide;
}

int fdsGetNumSides()
{
  return FDS_NumSides;
}

/*-------------------------------------------------------------------*/
/*  Sidecar (save-data) persistence (phase 6).                       */
/*  File format:                                                     */
/*    bytes 0..3 : "FDSV" magic                                      */
/*    byte  4    : version = 1                                       */
/*    byte  5    : sides (must equal FDS_NumSides at load time)      */
/*    byte  6    : bitmap byte count (= ceil(num_pages / 8))         */
/*    byte  7    : reserved (0)                                      */
/*    bytes 8..  : bitmap, 1 bit per FDS_PAGE_SIZE page              */
/*    bytes ..   : for each set bit (in order), FDS_PAGE_SIZE bytes  */
/*                 of disk-image content.                            */
/*  Page indexing is flat across all sides: page p covers disk image */
/*  bytes [p*PAGE_SIZE .. p*PAGE_SIZE+PAGE_SIZE-1]. The last page    */
/*  may extend past total_disk_bytes; we save FDS_PAGE_SIZE anyway   */
/*  (zero-padded) so file layout stays predictable.                  */
/*-------------------------------------------------------------------*/

#define FDS_SIDECAR_MAGIC0 'F'
#define FDS_SIDECAR_MAGIC1 'D'
#define FDS_SIDECAR_MAGIC2 'S'
#define FDS_SIDECAR_MAGIC3 'V'
#define FDS_SIDECAR_VERSION 1

bool fdsHasDirtyPages()
{
  for (size_t i = 0; i < sizeof(fds_dirty_bitmap); ++i)
    if (fds_dirty_bitmap[i]) return true;
  return false;
}

#if PICO_RP2350
static int fdsTotalPages()
{
  /* Page count covers the expanded buffer (which is what we actually
     mutate at runtime), not the raw .fds. */
  return (int)((fds_expanded_total_size + FDS_PAGE_SIZE - 1) / FDS_PAGE_SIZE);
}
#endif

bool fdsLoadSidecar(const char *path)
{
#if !PICO_RP2350
  (void)path;
  return false;
#else
  if (!IsFDS || !fds_expanded || !path) return false;

  FILINFO fno;
  if (f_stat(path, &fno) != FR_OK) return true; /* absent is OK */

  FIL fil;
  if (f_open(&fil, path, FA_READ) != FR_OK) return false;

  BYTE hdr[8];
  UINT br = 0;
  if (f_read(&fil, hdr, sizeof(hdr), &br) != FR_OK || br != sizeof(hdr) ||
      hdr[0] != FDS_SIDECAR_MAGIC0 || hdr[1] != FDS_SIDECAR_MAGIC1 ||
      hdr[2] != FDS_SIDECAR_MAGIC2 || hdr[3] != FDS_SIDECAR_MAGIC3 ||
      hdr[4] != FDS_SIDECAR_VERSION || hdr[5] != FDS_NumSides)
  {
    printf("FDS sidecar header mismatch — ignoring.\n");
    f_close(&fil);
    return false;
  }

  int numPages = fdsTotalPages();
  int bmBytes = hdr[6];
  if (bmBytes != (numPages + 7) / 8)
  {
    printf("FDS sidecar bitmap size mismatch — ignoring.\n");
    f_close(&fil);
    return false;
  }

  BYTE bm[(FDS_MAX_PAGES + 7) / 8] = {0};
  if (f_read(&fil, bm, bmBytes, &br) != FR_OK || br != (UINT)bmBytes)
  {
    f_close(&fil);
    return false;
  }

  DWORD totalBytes = fds_expanded_total_size;
  int loaded = 0;
  for (int p = 0; p < numPages; ++p)
  {
    if (!((bm[p >> 3] >> (p & 7)) & 1)) continue;
    DWORD off = (DWORD)p * FDS_PAGE_SIZE;
    DWORD bytes = FDS_PAGE_SIZE;
    if (off >= totalBytes) bytes = 0;
    else if (off + bytes > totalBytes) bytes = totalBytes - off;

    if (bytes)
    {
      if (f_read(&fil, fds_expanded + off, bytes, &br) != FR_OK ||
          br != bytes)
      {
        f_close(&fil);
        return false;
      }
      fdsMarkDirty(off);
    }
    /* Skip any pad bytes we wrote on save to keep the page slot at a
       fixed PAGE_SIZE in the file. */
    if (bytes < FDS_PAGE_SIZE)
    {
      BYTE skip[64];
      DWORD remain = FDS_PAGE_SIZE - bytes;
      while (remain)
      {
        DWORD chunk = remain < sizeof(skip) ? remain : sizeof(skip);
        if (f_read(&fil, skip, chunk, &br) != FR_OK) { f_close(&fil); return false; }
        remain -= chunk;
      }
    }
    ++loaded;
  }
  f_close(&fil);
  printf("FDS sidecar: applied %d dirty page(s) from %s\n", loaded, path);
  return true;
#endif
}

bool fdsSaveSidecar(const char *path)
{
#if !PICO_RP2350
  (void)path;
  return false;
#else
  if (!IsFDS || !fds_expanded || !path) return false;
  if (!fdsHasDirtyPages())
  {
    printf("FDS sidecar: no dirty pages, skipping write.\n");
    return true;
  }

  int numPages = fdsTotalPages();
  int bmBytes  = (numPages + 7) / 8;

  FIL fil;
  FRESULT fr = f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
  {
    printf("FDS sidecar: cannot open %s for write (%d)\n", path, fr);
    return false;
  }

  BYTE hdr[8] = {
    FDS_SIDECAR_MAGIC0, FDS_SIDECAR_MAGIC1,
    FDS_SIDECAR_MAGIC2, FDS_SIDECAR_MAGIC3,
    FDS_SIDECAR_VERSION,
    (BYTE)FDS_NumSides,
    (BYTE)bmBytes,
    0
  };
  UINT bw = 0;
  if (f_write(&fil, hdr, sizeof(hdr), &bw) != FR_OK || bw != sizeof(hdr)) goto fail;
  if (f_write(&fil, fds_dirty_bitmap, bmBytes, &bw) != FR_OK || bw != (UINT)bmBytes) goto fail;

  {
    DWORD totalBytes = fds_expanded_total_size;
    int saved = 0;
    for (int p = 0; p < numPages; ++p)
    {
      if (!fdsPageDirty((unsigned)p)) continue;
      DWORD off = (DWORD)p * FDS_PAGE_SIZE;
      DWORD bytes = FDS_PAGE_SIZE;
      if (off >= totalBytes) bytes = 0;
      else if (off + bytes > totalBytes) bytes = totalBytes - off;

      if (bytes)
      {
        if (f_write(&fil, fds_expanded + off, bytes, &bw) != FR_OK ||
            bw != bytes) goto fail;
      }
      if (bytes < FDS_PAGE_SIZE)
      {
        static const BYTE zeros[64] = {0};
        DWORD remain = FDS_PAGE_SIZE - bytes;
        while (remain)
        {
          DWORD chunk = remain < sizeof(zeros) ? remain : sizeof(zeros);
          if (f_write(&fil, zeros, chunk, &bw) != FR_OK || bw != chunk) goto fail;
          remain -= chunk;
        }
      }
      ++saved;
    }
    f_close(&fil);
    printf("FDS sidecar: wrote %d dirty page(s) to %s\n", saved, path);
    return true;
  }

fail:
  printf("FDS sidecar: write failed for %s\n", path);
  f_close(&fil);
  return false;
#endif
}

/*-------------------------------------------------------------------*/
/*  fdsResetDrive: clear all drive state. Called from Map20_Init.    */
/*-------------------------------------------------------------------*/
void fdsResetDrive()
{
  fds_timer_reload   = 0;
  fds_timer_counter  = 0;
  fds_timer_irq_en   = 0;
  fds_timer_repeat   = 0;
  fds_timer_irq_pend = 0;

  fds_master_enable  = 0;

  fds_motor_on   = 0;
  fds_xfer_reset = 1;
  fds_read_mode  = 1;
  fds_byte_irq_en = 0;
  fds_scan_start = 0;

  fds_byte_pos      = 0;
  fds_read_buf      = 0;
  fds_byte_irq_pend = 0;
  fds_end_of_head   = 0;
  fds_cycle_acc     = 0;
  fds_motor_spinup  = 0;
  fds_scanning_started = 0;
  fds_crc_control   = 0;
  fds_gap_ended     = 0;

  /* Cancel any in-flight disk swap so a reset during the eject pulse
     doesn't leave the disk permanently ejected. */
  fds_eject_counter = 0;
  fds_pending_side  = -1;
  FDS_DiskInserted  = true;

  fds_write_buf = 0;
  fds_block_count = 0;
  FDS_LOG("reset drive\n");
}

/*-------------------------------------------------------------------*/
/*  fdsApuWrite: $4020-$4026 disk drive registers.                   */
/*  Called for every write in $4018-$5FFF; ignore other addresses.   */
/*-------------------------------------------------------------------*/
void fdsApuWrite(WORD wAddr, BYTE byData)
{
  switch (wAddr)
  {
  case 0x4020: /* IRQ reload low byte */
    fds_timer_reload = (fds_timer_reload & 0xFF00) | byData;
    FDS_LOG("4020 = %02X (reload=%04X)\n", byData, fds_timer_reload);
    break;

  case 0x4021: /* IRQ reload high byte */
    fds_timer_reload = (fds_timer_reload & 0x00FF) | ((WORD)byData << 8);
    FDS_LOG("4021 = %02X (reload=%04X)\n", byData, fds_timer_reload);
    break;

  case 0x4022: /* IRQ control */
    fds_timer_repeat = byData & 0x01;
    fds_timer_irq_en = (byData >> 1) & 0x01;
    FDS_LOG("4022 = %02X (en=%d rep=%d reload=%04X)\n",
            byData, (int)fds_timer_irq_en, (int)fds_timer_repeat,
            fds_timer_reload);
    if (fds_timer_irq_en)
    {
      /* Load counter on enable, regardless of master_enable. Counter
         only counts down when master_enable bit 0 is set, but the
         load itself happens here so a BIOS that configures $4022
         before $4023 still ends up with a primed counter. */
      fds_timer_counter = fds_timer_reload;
    }
    else
    {
      /* Bit 1 = 0 acks the timer IRQ. */
      fds_timer_irq_pend = 0;
    }
    break;

  case 0x4023: /* Master enable */
    FDS_LOG("4023 = %02X\n", byData);
    fds_master_enable = byData & 0x03;
    if (!(fds_master_enable & 0x01))
    {
      /* Disabling disk regs only acks pending IRQs — fds_timer_irq_en
         must NOT be cleared here (BIOS uses $4023 to ack/re-enable). */
      fds_timer_irq_pend = 0;
      fds_byte_irq_pend = 0;
    }
    break;

  case 0x4024: /* Disk write data */
    fds_write_buf = byData;
    fds_byte_irq_pend = 0;
    /* Mirror the $4031 read: reset the byte-pacing accumulator so the
       next byte production is a full FDS_CYC_PER_BYTE cycles away.
       Without this, the drive emits multiple write IRQs faster than
       the BIOS can run its handler, so BIOS overshoots past payload
       into the CRC slots before it gets a chance to assert crc=1. */
    fds_cycle_acc = 0;
    break;

  case 0x4025: /* Disk control */
  {
    BYTE prev_motor    = fds_motor_on;
    BYTE prev_xfer_rst = fds_xfer_reset;
    BYTE prev_scan     = fds_scan_start;
    BYTE prev_crc_control = fds_crc_control;
    (void)prev_scan;
    if (fds_block_count <= 80) {
      FDS_LOG("4025 = %02X (mot=%d rst=%d rd=%d mir=%d crc=%d scan=%d irq=%d) pos=%lu ge=%d\n",
              byData,
              byData & 1, (byData >> 1) & 1, (byData >> 2) & 1,
              (byData >> 3) & 1, (byData >> 4) & 1,
              (byData >> 6) & 1, (byData >> 7) & 1,
              (unsigned long)fds_byte_pos, (int)fds_gap_ended);
    }

    fds_motor_on    = byData & 0x01;
    fds_xfer_reset  = (byData >> 1) & 0x01;
    fds_read_mode   = (byData >> 2) & 0x01;
    /* bit 3: 1 = horizontal mirroring, 0 = vertical (opposite of NES). */
    InfoNES_Mirroring((byData & 0x08) ? 0 : 1);
    fds_crc_control = (byData >> 4) & 0x01;
    fds_scan_start  = (byData >> 6) & 0x01;
    fds_byte_irq_en = (byData >> 7) & 0x01;

    /* Per Mesen2 (Fds.cpp $4025 handler): every $4025 write clears
       the FDS-disk byte-ready IRQ source. */
    fds_byte_irq_pend = 0;

    /* Motor turning on: head returns to start of disk. */
    if (!prev_motor && fds_motor_on)
    {
      fdsResetTransferPosition();
      fds_motor_spinup = 50;
    }
    if (!fds_motor_on)
    {
      fds_scanning_started = 0;
    }

    /* Transfer reset asserted (rst 0→1): the BIOS uses this to reset
       the bit-stream synchroniser before starting a new block scan.
       byte_pos is unchanged; gap_ended → 0 makes the next scan walk
       silently to the next block start, which is how file-skip and
       block-boundary resync both work. */
    if (!prev_xfer_rst && fds_xfer_reset)
    {
      fds_gap_ended = 0;
    }

    /* CRC control rising edge during a write: BIOS just signalled
       end-of-payload. For a synthesized block (one we created on the
       fly past the last existing block_end), this is the moment we
       learn how big it is. Set bends[lastBlock] = byte_pos + 2 so the
       auto-flip-ge=0 trigger fires after the 2 CRC slots are written.
       Existing blocks already have a baked-in bends from the buffer
       build and don't need updating. */
    if (!prev_crc_control && fds_crc_control && !fds_read_mode &&
        fds_gap_ended && IsFDS &&
        FDS_CurrentSide >= 0 && FDS_CurrentSide < FDS_MAX_SIDES)
    {
      int n = fds_block_counts[FDS_CurrentSide];
      if (n > 0)
      {
        DWORD lastStart = fds_block_starts[FDS_CurrentSide][n - 1];
        DWORD lastEnd   = fds_block_ends[FDS_CurrentSide][n - 1];
        if (lastStart == lastEnd && fds_byte_pos > lastStart)
        {
          fds_block_ends[FDS_CurrentSide][n - 1] = fds_byte_pos + 2;
        }
      }
    }
    break;
  }

  case 0x4026: /* External output, unused by most software */
    break;

  default:
    /* $4040-$4097 (FDS audio) lands here too; phase 7 will handle it. */
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  fdsApuRead: $4030-$4033 disk drive status / data.                */
/*-------------------------------------------------------------------*/
BYTE fdsApuRead(WORD wAddr)
{
  switch (wAddr)
  {
  case 0x4030: /* Disk status */
  {
    BYTE r = 0;
    if (fds_timer_irq_pend) r |= 0x01;
    if (fds_byte_irq_pend)  r |= 0x02;
    if (fds_end_of_head)    r |= 0x40;
    /* Reading $4030 acknowledges both the timer IRQ flag (bit 0) and
       the byte-ready flag (bit 1). Nestopia (Adapter::Peek_4030):
       `unit.status = 0; ClearIRQ(); return status; ` — both flags
       are cleared regardless of which one was set. */
    fds_timer_irq_pend = 0;
    fds_byte_irq_pend  = 0;
    if (r) FDS_LOG("  4030 -> %02X (pos=%lu ge=%d)\n", r,
                   (unsigned long)fds_byte_pos, (int)fds_gap_ended);
    return r;
  }

  case 0x4031: /* Disk read data */
    FDS_BYTELOG("  4031 -> %02X (pos=%lu ge=%d pend=%d)\n",
                fds_read_buf, (unsigned long)fds_byte_pos, (int)fds_gap_ended,
                (int)fds_byte_irq_pend);
    fds_byte_irq_pend = 0;
    /* Reset the byte-pacing accumulator so the next byte production
       is a full FDS_CYC_PER_BYTE cycles away. Our HSync-quantised IRQ
       delivery would otherwise hand the BIOS the next byte before it
       can finish its IRQ handler and write $4025 (which clears the
       IRQ source). Without this delay the BIOS handler runs back-to-
       back twice — first time it gets the last data byte, second
       time it gets the CRC byte that the BIOS interprets as the next
       block's type byte (→ ERR.23 / ERR.24). */
    fds_cycle_acc = 0;
    return fds_read_buf;

  case 0x4032: /* Drive status: bit0=disk-not-inserted, bit1=disk-not-ready, bit2=write-protect */
  {
    BYTE r = 0;
    if (!FDS_DiskInserted)  r |= 0x05; /* not-inserted and not-ready */
    else if (!fds_motor_on || fds_motor_spinup > 0) r |= 0x02;
    /* Bit 2 (write protect) left clear: simulating a writable disk. */
    return r;
  }

  case 0x4033: /* External input: battery good = $80 */
    return 0x80;

  default:
    return (BYTE)(wAddr >> 8); /* open bus */
  }
}

/*-------------------------------------------------------------------*/
/*  fdsHsync: advance timer + disk byte clocks once per scanline.    */
/*-------------------------------------------------------------------*/
void fdsHsync()
{
  /* Drive the disk-swap eject pulse before anything else: the BIOS
     reads $4032 in its disk-status loop and we want it to see the
     disk-not-inserted bit reliably while fds_eject_counter > 0. */
  if (fds_eject_counter > 0)
  {
    fds_eject_counter--;
    if (fds_eject_counter == 0 && fds_pending_side >= 0 &&
        fds_pending_side < FDS_NumSides)
    {
      FDS_CurrentSide = fds_pending_side;
      FDS_DiskInserted = true;
      fds_pending_side = -1;
      /* Simulate a fresh disk insertion: home the drive head to the
         start of the side, drop motor-ready for a few scanlines so the
         BIOS sees $4032 bit 1 set transiently, and clear any pending
         IRQ/byte state that was carrying over from the previous side.
         Without this, byte_pos is still pointing somewhere mid-disk on
         the old side and the BIOS reads the wrong bytes off side B. */
      fdsResetTransferPosition();
      fds_motor_spinup = 50;
      fds_byte_irq_pend = 0;
    }
  }

  /* Advance timer. Counts down at CPU rate; reloads if repeat=1.
     Loops so reload values smaller than a scanline still fire the
     correct number of IRQs (otherwise the WORD counter wraps and
     stalls). */
  if (fds_timer_irq_en && (fds_master_enable & 0x01))
  {
    int remaining = STEP_PER_SCANLINE;
    while (remaining > 0 && fds_timer_irq_en)
    {
      if (fds_timer_counter <= (WORD)remaining)
      {
        remaining -= fds_timer_counter;
        fds_timer_irq_pend = 1;
        if (fds_timer_repeat)
        {
          fds_timer_counter = fds_timer_reload;
          if (fds_timer_reload == 0) break; /* avoid infinite loop */
        }
        else
        {
          fds_timer_irq_en = 0;
          fds_timer_counter = 0;
        }
      }
      else
      {
        fds_timer_counter -= remaining;
        remaining = 0;
      }
    }
  }

  /* Drain motor spin-up before the drive starts feeding bytes. */
  if (fds_motor_spinup > 0)
  {
    fds_motor_spinup--;
  }
  else if (fds_motor_on && fds_scan_start && FDS_DiskInserted &&
           fds_expanded && !fds_end_of_head)
  {
    DWORD sideSize = fds_side_sizes[FDS_CurrentSide];
    BYTE *side     = fds_expanded + fds_side_offsets[FDS_CurrentSide];
    int   numBlocks = fds_block_counts[FDS_CurrentSide];
    DWORD *bstarts  = fds_block_starts[FDS_CurrentSide];
    DWORD *bends    = fds_block_ends[FDS_CurrentSide];

    fds_cycle_acc += STEP_PER_SCANLINE;
    while (fds_cycle_acc >= FDS_CYC_PER_BYTE && !fds_end_of_head)
    {
      fds_cycle_acc -= FDS_CYC_PER_BYTE;

      /* Linear walk. When gap_ended==0 the drive is hunting for the
         next block start; bytes between here and that start (gap
         zeros, leftover payload from rst-mid-block, fake CRC) are
         consumed silently — no byte-ready IRQ. When pos reaches a
         block start, gap_ended flips to 1 and emission begins on
         this same tick (the first emitted byte is the block-type
         byte at the block_start offset). From then on every byte
         fires byte-ready until rst=1 resets gap_ended. */
      if (!fds_gap_ended)
      {
        DWORD nextStart = sideSize;
        for (int i = 0; i < numBlocks; ++i)
        {
          if (bstarts[i] >= fds_byte_pos) { nextStart = bstarts[i]; break; }
        }
        /* Append support: in write mode, when we're past every existing
           block, synthesize a new block one inter-block-gap past the
           last block_end. The BIOS uses this to add a new file (e.g.
           Metroid's first save) — without it the drive walks silently
           toward end-of-head and BIOS errors out (ERR.24). If pos is
           already past lastEnd+gap (BIOS spent some time positioning),
           synth at the current pos so we don't keep walking forever. */
        if (!fds_read_mode && nextStart == sideSize && numBlocks > 0 &&
            numBlocks < FDS_MAX_BLOCKS_PER_SIDE)
        {
          DWORD lastEnd = bends[numBlocks - 1];
          if (fds_byte_pos >= lastEnd)
          {
            DWORD synthPos = lastEnd + FDS_INTER_BLOCK_GAP;
            if (synthPos < fds_byte_pos) synthPos = fds_byte_pos;
            if (synthPos < sideSize)
            {
              nextStart = synthPos;
            }
          }
        }
        if (fds_byte_pos < nextStart)
        {
          fds_byte_pos++;
          continue;
        }
        if (fds_byte_pos >= sideSize) { fds_end_of_head = 1; break; }
        /* At a known (or synthesized) block start: arm for emission. */
        fds_gap_ended = 1;
        /* Register a synthesized block so future seeks find it.
           Initial bends == bstarts is a placeholder that will be
           overwritten when the BIOS asserts crc=1 (write mode end-of-
           block trigger) or when 16 KB worst-case payload elapses. */
        if (!fds_read_mode && numBlocks < FDS_MAX_BLOCKS_PER_SIDE)
        {
          bool isNew = true;
          for (int i = 0; i < numBlocks; ++i)
          {
            if (bstarts[i] == fds_byte_pos) { isNew = false; break; }
          }
          if (isNew)
          {
            bstarts[numBlocks] = fds_byte_pos;
            bends[numBlocks]   = fds_byte_pos;   /* placeholder */
            fds_block_counts[FDS_CurrentSide]++;
            numBlocks = fds_block_counts[FDS_CurrentSide];
            FDS_LOG("synth block#%d at pos=%lu\n",
                    numBlocks, (unsigned long)fds_byte_pos);
          }
        }
      }

      fds_scanning_started = 1;

      if (fds_read_mode)
      {
        fds_read_buf = side[fds_byte_pos];
      }
      else
      {
        side[fds_byte_pos] = fds_write_buf;
        fdsMarkDirty(fds_side_offsets[FDS_CurrentSide] + fds_byte_pos);
      }
      fds_byte_pos++;
      fds_byte_irq_pend = 1;

      /* If we just emitted the last CRC byte of a block, flip gap_ended
         back to 0 so the inter-block gap zeros are walked silently —
         BIOS would otherwise read those zeros as the next block's type
         byte and reject the disk (ERR.23). */
      for (int i = 0; i < numBlocks; ++i)
      {
        if (bends[i] == fds_byte_pos) { fds_gap_ended = 0; break; }
      }

      if (fds_byte_pos >= sideSize) { fds_end_of_head = 1; break; }
    }
  }

  /* Timer IRQ fires unconditionally on underflow (bit 0). The byte-
     ready flag only asserts the IRQ line when bit 7 of $4025 is set. */
  if (fds_master_enable & 0x01)
  {
    if (fds_timer_irq_pend ||
        (fds_byte_irq_pend && fds_byte_irq_en))
    {
      IRQ_REQ;
    }
  }
}

void fdsRelease()
{
  if (FDS_Bios)   { Frens::f_free(FDS_Bios);   FDS_Bios   = nullptr; }
  if (FDS_PrgRam) { Frens::f_free(FDS_PrgRam); FDS_PrgRam = nullptr; }
  if (FDS_ChrRam) { Frens::f_free(FDS_ChrRam); FDS_ChrRam = nullptr; }
  if (fds_expanded) { Frens::f_free(fds_expanded); fds_expanded = nullptr; }
  fds_expanded_total_size = 0;
  for (int s = 0; s < FDS_MAX_SIDES; ++s) {
    fds_side_offsets[s] = 0;
    fds_side_sizes[s]   = 0;
    fds_block_counts[s] = 0;
  }
  /* FDS_DiskImage is a pointer into the loader-owned PSRAM buffer
     (ROM_FILE_ADDR); it is freed by the existing ROM unload path. */
  FDS_DiskImage = nullptr;
  FDS_NumSides = 0;
  FDS_CurrentSide = 0;
  fdsClearDirtyBitmap();
}
