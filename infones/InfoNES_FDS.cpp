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
static BYTE fds_scan_start     = 0;   /* $4025 bit 6 */

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

/* Phase 6: dirty-page bitmap for save-data writeback. FDS disks are
   writable; games (Zelda, Faria, Tobidase Daisakusen, ...) save game
   progress by writing bytes into the disk image. We capture those
   writes into PSRAM, mark the touched 4 KB pages dirty, and on save
   trigger flush only the dirty pages to /SAVES/<rom>.fds.sav. */
#define FDS_PAGE_SIZE         4096
#define FDS_PAGES_PER_SIDE    ((FDS_SIDE_SIZE + FDS_PAGE_SIZE - 1) / FDS_PAGE_SIZE)
#define FDS_MAX_PAGES         (FDS_PAGES_PER_SIDE * FDS_MAX_SIDES)
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

/* Tracing knob. Off in normal builds — tracing every $4025/$4031
   read and byte production floods the UART hard enough to stall
   emulation. Flip to 1 only when investigating a regression. */
#define FDS_TRACE 0
#if FDS_TRACE
#define FDS_LOG(...) printf("[FDS] " __VA_ARGS__)
#else
#define FDS_LOG(...) do {} while (0)
#endif

static int fds_block_count = 0;

/*
 *  Wire-format state machine. .fds files contain only block payloads
 *  back-to-back. The drive hardware that the BIOS talks to actually
 *  exposes a wider stream:
 *
 *      <block payload> <CRC byte 1=0x91> <CRC byte 2=0x88>
 *      <inter-block gap of 0x00 bytes> <next block payload> ...
 *
 *  All of those bytes drive byte-ready / IRQs; the BIOS reads them
 *  via $4031 in order. Skipping the gap (or the CRC values) breaks
 *  the BIOS state machine that uses the gap-of-zeros to know it is
 *  between blocks. Nestopia's drive emulator does the same thing:
 *  it injects 0x91/0x88 as CRC and then ~120 bytes of zeros before
 *  the next block's type byte (its BYTES_GAP_NEXT constant).
 *
 *  Bit 4 of $4030 (CRC error) is never raised — the BIOS gets CRC-OK
 *  whatever bytes we choose for the placeholder CRC.
 */
#define FDS_CRC_BYTE_1   0x00
#define FDS_CRC_BYTE_2   0x00
#define FDS_GAP_BYTES    120          /* approx Nestopia BYTES_GAP_NEXT */

typedef enum {
  FDS_PH_IDLE = 0,
  FDS_PH_BLOCK,   /* feeding payload bytes from FDS_DiskImage */
  FDS_PH_CRC,     /* feeding 2 placeholder CRC bytes */
  FDS_PH_GAP,     /* feeding zero gap bytes between blocks */
  FDS_PH_END      /* end of side / unknown block type */
} fds_phase_t;

static fds_phase_t fds_phase        = FDS_PH_IDLE;
static int         fds_block_remain = 0;
static int         fds_crc_remain   = 0;
static int         fds_gap_remain   = 0;
static WORD        fds_next_data_sz = 0;  /* from block-3 to block-4 */

static void fdsEnterBlock()
{
  if (fds_byte_pos >= FDS_SIDE_SIZE)
  {
    fds_end_of_head = 1;
    fds_phase = FDS_PH_END;
    FDS_LOG("END pos=%lu\n", (unsigned long)fds_byte_pos);
    return;
  }
  BYTE type = FDS_DiskImage[(DWORD)FDS_CurrentSide * FDS_SIDE_SIZE + fds_byte_pos];
  int sz = 0;
  switch (type)
  {
  case 0x01: sz = 56; break;            /* disk info */
  case 0x02: sz = 2;  break;            /* file amount */
  case 0x03:                            /* file header */
    sz = 16;
    if (fds_byte_pos + 14 < FDS_SIDE_SIZE)
    {
      DWORD base = (DWORD)FDS_CurrentSide * FDS_SIDE_SIZE + fds_byte_pos;
      fds_next_data_sz = FDS_DiskImage[base + 13] |
                         ((WORD)FDS_DiskImage[base + 14] << 8);
    }
    else
    {
      fds_next_data_sz = 0;
    }
    break;
  case 0x04: sz = 1 + fds_next_data_sz; break;  /* file data */
  default:
    /* Garbage / past-end-of-disk: stop. */
    fds_end_of_head = 1;
    fds_phase = FDS_PH_END;
    FDS_LOG("END pos=%lu\n", (unsigned long)fds_byte_pos);
    return;
  }
  if ((DWORD)(fds_byte_pos + sz) > FDS_SIDE_SIZE) sz = FDS_SIDE_SIZE - fds_byte_pos;
  fds_block_remain = sz;
  fds_block_start_pos = fds_byte_pos;   /* anchor for transfer-reset rewind */
  fds_phase = FDS_PH_BLOCK;
  fds_block_count++;
  if (fds_block_count <= 200) {
    FDS_LOG("block#%d type=%02X pos=%lu sz=%d nextSz=%u\n",
            fds_block_count, type, (unsigned long)fds_byte_pos, sz,
            (unsigned)fds_next_data_sz);
  }
}

static BYTE fdsProduceByte()
{
  switch (fds_phase)
  {
  case FDS_PH_BLOCK:
  {
    BYTE b = FDS_DiskImage[(DWORD)FDS_CurrentSide * FDS_SIDE_SIZE + fds_byte_pos];
    if (fds_block_count <= 20 &&
        (fds_byte_pos - fds_block_start_pos) < 20)
    {
      FDS_LOG("  byte[%lu]=%02X (off %ld in block#%d)\n",
              (unsigned long)fds_byte_pos, b,
              (long)(fds_byte_pos - fds_block_start_pos),
              fds_block_count);
    }
    fds_byte_pos++;
    fds_block_remain--;
    if (fds_block_remain <= 0)
    {
      fds_phase = FDS_PH_CRC;
      fds_crc_remain = 2;
    }
    return b;
  }
  case FDS_PH_CRC:
  {
    /* Two placeholder CRC bytes are injected between each block's
       payload and the inter-block gap. Real BIOS reads them via
       $4031 just like data bytes — they raise byte-ready and fire
       IRQ. The specific values 0x91 / 0x88 are what Nestopia's drive
       emulator returns and match what the BIOS expects to ignore. */
    int idx = fds_crc_remain;
    fds_crc_remain--;
    if (fds_crc_remain <= 0)
    {
      fds_phase = FDS_PH_GAP;
      fds_gap_remain = FDS_GAP_BYTES;
    }
    return (idx == 2) ? FDS_CRC_BYTE_1 : FDS_CRC_BYTE_2;
  }
  case FDS_PH_GAP:
    fds_gap_remain--;
    if (fds_gap_remain <= 0)
    {
      /* Auto-advance into the next block. byte_pos is already at the
         start of that block — CRC + GAP do not advance byte_pos. */
      fdsEnterBlock();
    }
    return 0x00;
  default:
    return 0x00; /* IDLE / END */
  }
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
  fds_phase = FDS_PH_IDLE;
  fds_block_remain = 0;
  fds_crc_remain = 0;
  fds_gap_remain = 0;
  fds_next_data_sz = 0;
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
  if (fr != FR_OK)
  {
    InfoNES_Error("FDS BIOS not found at %s", FDS_BIOS_PATH);
    return false;
  }
  if (fno.fsize != FDS_BIOS_SIZE)
  {
    InfoNES_Error("FDS BIOS at %s must be %d bytes (is %lu)",
                  FDS_BIOS_PATH, (int)FDS_BIOS_SIZE, (unsigned long)fno.fsize);
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
    InfoNES_Error("Cannot open FDS BIOS %s (%d)", FDS_BIOS_PATH, fr);
    return false;
  }
  UINT br = 0;
  fr = f_read(&fil, dst, FDS_BIOS_SIZE, &br);
  f_close(&fil);
  if (fr != FR_OK || br != FDS_BIOS_SIZE)
  {
    InfoNES_Error("Failed reading FDS BIOS (%d, %u/%d)", fr, br, FDS_BIOS_SIZE);
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
  DWORD total = (DWORD)FDS_NumSides * FDS_SIDE_SIZE;
  return (int)((total + FDS_PAGE_SIZE - 1) / FDS_PAGE_SIZE);
}
#endif

bool fdsLoadSidecar(const char *path)
{
#if !PICO_RP2350
  (void)path;
  return false;
#else
  if (!IsFDS || !FDS_DiskImage || !path) return false;

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

  DWORD totalBytes = (DWORD)FDS_NumSides * FDS_SIDE_SIZE;
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
      if (f_read(&fil, FDS_DiskImage + off, bytes, &br) != FR_OK ||
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
  if (!IsFDS || !FDS_DiskImage || !path) return false;
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
    DWORD totalBytes = (DWORD)FDS_NumSides * FDS_SIDE_SIZE;
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
        if (f_write(&fil, FDS_DiskImage + off, bytes, &bw) != FR_OK ||
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

  case 0x4024: /* Disk write data (write-back deferred to phase 6) */
    fds_write_buf = byData;
    fds_byte_irq_pend = 0;
    break;

  case 0x4025: /* Disk control */
  {
    BYTE prev_motor    = fds_motor_on;
    BYTE prev_xfer_rst = fds_xfer_reset;
    BYTE prev_scan     = fds_scan_start;
    FDS_LOG("4025 = %02X (mot=%d rst=%d rd=%d mir=%d crc=%d scan=%d irq=%d) pos=%lu phase=%d\n",
            byData,
            byData & 1, (byData >> 1) & 1, (byData >> 2) & 1,
            (byData >> 3) & 1, (byData >> 4) & 1,
            (byData >> 6) & 1, (byData >> 7) & 1,
            (unsigned long)fds_byte_pos, (int)fds_phase);

    fds_motor_on    = byData & 0x01;
    fds_xfer_reset  = (byData >> 1) & 0x01;
    fds_read_mode   = (byData >> 2) & 0x01;
    /* bit 3: 1 = horizontal mirroring, 0 = vertical (opposite of NES). */
    InfoNES_Mirroring((byData & 0x08) ? 0 : 1);
    /* bit 4: CRC ctrl, ignored (we always report CRC OK). */
    fds_scan_start  = (byData >> 6) & 0x01;
    fds_byte_irq_en = (byData >> 7) & 0x01;

    /* Per Mesen2 (Fds.cpp $4025 handler): every $4025 write clears
       the FDS-disk byte-ready IRQ source. Without this, a stale
       byte_irq_pend left over from a CRC byte produced BETWEEN the
       BIOS reading the last data byte and writing $4025 to set up
       the next block read fires an IRQ that the BIOS sees as the
       first byte of the next block — which is what was tripping
       ERR.23 here. */
    fds_byte_irq_pend = 0;

    /* Motor turning on: head returns to start of disk. */
    if (!prev_motor && fds_motor_on)
    {
      fdsResetTransferPosition();
      fds_motor_spinup = 50;
    }
    /* Transfer reset asserted: stop feeding. byte_pos stays where
       it is — CRC + GAP do not advance byte_pos, so we are already
       at the start of the next block. */
    if (!prev_xfer_rst && fds_xfer_reset)
    {
      fds_phase = FDS_PH_IDLE;
    }
    /* Transfer reset deasserted: enter the block at byte_pos. */
    if (prev_xfer_rst && !fds_xfer_reset)
    {
      fdsEnterBlock();
      fds_cycle_acc = 0;
    }
    /* Scan-start dropped (1→0): per Mesen2's _gapEnded reset, the
       BIOS uses this transition as "I'm done with the current block,
       skip past the CRC + gap to the next block's first byte." Fast-
       forward past CRC/GAP so that when scan_start is asserted again
       the very next byte produced is the next block's type byte —
       not a CRC byte that the BIOS would reject as the wrong block
       type. */
    if (prev_scan && !fds_scan_start &&
        (fds_phase == FDS_PH_CRC || fds_phase == FDS_PH_GAP))
    {
      fds_phase = FDS_PH_IDLE;
      fds_crc_remain = 0;
      fds_gap_remain = 0;
      fds_cycle_acc = 0;
    }
    /* Scan-start asserted (0→1) while idle at a block boundary:
       enter the next block so production resumes with the type byte. */
    if (!prev_scan && fds_scan_start && fds_phase == FDS_PH_IDLE &&
        !fds_xfer_reset)
    {
      fdsEnterBlock();
      fds_cycle_acc = 0;
    }
    /* Bit 7 only gates the IRQ pin; the byte-ready flag visible at
       $4030 bit 1 is independent and must remain set so polling code
       can see it. (Earlier we cleared it here, which broke block-3
       reads in BIOS routines that poll instead of using IRQs.) */
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
    if (r) FDS_LOG("  4030 -> %02X (pos=%lu phase=%d)\n", r,
                   (unsigned long)fds_byte_pos, (int)fds_phase);
    return r;
  }

  case 0x4031: /* Disk read data */
    if (fds_phase != FDS_PH_GAP)
    {
      FDS_LOG("  4031 -> %02X (pos=%lu phase=%d pend=%d)\n",
              fds_read_buf, (unsigned long)fds_byte_pos, (int)fds_phase,
              (int)fds_byte_irq_pend);
    }
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
  else if (fds_motor_on && !fds_xfer_reset && fds_scan_start &&
           FDS_DiskInserted && FDS_DiskImage &&
           fds_phase != FDS_PH_IDLE && fds_phase != FDS_PH_END)
  {
    fds_cycle_acc += STEP_PER_SCANLINE;
    while (fds_cycle_acc >= FDS_CYC_PER_BYTE)
    {
      fds_cycle_acc -= FDS_CYC_PER_BYTE;
      if (fds_read_mode)
      {
        /* Nestopia/FCEUX: every byte of the expanded stream — payload,
           placeholder CRC, AND inter-block gap zeros — raises byte-ready
           and updates $4031. The BIOS uses the run of gap zeros as its
           "between blocks" signal, so we cannot skip them. */
        fds_read_buf = fdsProduceByte();
        fds_byte_irq_pend = 1;
      }
      else
      {
        /* Write mode: the BIOS staged a byte via $4024; drop it onto the
           disk image at the current head position. We only commit during
           BLOCK phase (the actual file payload). CRC bytes are computed
           by the controller and gap bytes don't carry data, so we just
           advance the state machine for those phases without writing. */
        if (fds_phase == FDS_PH_BLOCK && FDS_DiskImage)
        {
          DWORD off = (DWORD)FDS_CurrentSide * FDS_SIDE_SIZE + fds_byte_pos;
          if (off < (DWORD)FDS_NumSides * FDS_SIDE_SIZE)
          {
            FDS_DiskImage[off] = fds_write_buf;
            fdsMarkDirty(off);
          }
        }
        (void)fdsProduceByte(); /* advance phase counters */
        fds_byte_irq_pend = 1;
      }
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
  /* FDS_DiskImage is a pointer into the loader-owned PSRAM buffer
     (ROM_FILE_ADDR); it is freed by the existing ROM unload path. */
  FDS_DiskImage = nullptr;
  FDS_NumSides = 0;
  FDS_CurrentSide = 0;
  fdsClearDirtyBitmap();
}
