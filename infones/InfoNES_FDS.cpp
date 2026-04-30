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

/* Tracing knob. Flip to 1, recapture UART, share the log. Off by default
   — tracing every $4025 write and block transition would otherwise
   flood the UART during the BIOS boot. */
#define FDS_TRACE 1
#if FDS_TRACE
#define FDS_LOG(...) printf("[FDS] " __VA_ARGS__)
#else
#define FDS_LOG(...) do {} while (0)
#endif

static int fds_block_count = 0;

/*
 *  Wire-format state machine. .fds files contain only block payloads
 *  back-to-back. We feed the payload bytes from the image and inject
 *  2 dummy CRC bytes ($00) after each block; byte_pos does not advance
 *  during the CRC phase, so the next block's first byte stays aligned
 *  on the next BLOCK entry. We do not inject a $80 mark byte: the
 *  drive hardware on a real FDS handles gap+mark synchronization
 *  internally, and the boot file-header / file-data read routines in
 *  the BIOS expect their first $4031 read to be the block-type byte
 *  directly. Bit 4 of $4030 (CRC error) is never raised, so the BIOS
 *  sees CRC-OK regardless of the dummy CRC value.
 */
typedef enum {
  FDS_PH_IDLE = 0,
  FDS_PH_BLOCK,   /* feeding payload bytes from FDS_DiskImage */
  FDS_PH_CRC,     /* feeding 2 dummy CRC bytes */
  FDS_PH_END      /* end of side / unknown block type */
} fds_phase_t;

static fds_phase_t fds_phase        = FDS_PH_IDLE;
static int         fds_block_remain = 0;
static int         fds_crc_remain   = 0;
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
    fds_crc_remain--;
    if (fds_crc_remain <= 0)
    {
      /* Auto-advance into the next block so games doing continuous
         reads (no $4025 bit 1 toggle between blocks) keep getting
         data. If the BIOS toggles transfer-reset, the 0->1 handler
         rewinds byte_pos to fds_block_start_pos so the over-shoot
         from any byte we produced between CRC and the toggle does
         not corrupt alignment. */
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

  printf("FDS: image %u bytes, %d side(s); PSRAM avail before alloc %u, after %u\n",
         (unsigned)diskSize, sides, (unsigned)avail,
         (unsigned)Frens::GetAvailableMemory());
  return true;
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
    break;

  case 0x4021: /* IRQ reload high byte */
    fds_timer_reload = (fds_timer_reload & 0x00FF) | ((WORD)byData << 8);
    break;

  case 0x4022: /* IRQ control */
    fds_timer_repeat = byData & 0x01;
    fds_timer_irq_en = (byData >> 1) & 0x01;
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

    /* Motor turning on: head returns to start of disk. */
    if (!prev_motor && fds_motor_on)
    {
      fdsResetTransferPosition();
      fds_motor_spinup = 50;  /* a few scanlines of seek time */
    }
    /* Transfer reset asserted: stop feeding and rewind byte_pos to
       the start of the current block. This undoes any over-shoot
       caused by the auto-advance after CRC producing the next
       block's first byte(s) before the BIOS toggled transfer-reset.
       Without the rewind, a slow BIOS would land mid-block when it
       re-enters via the 0->1 transition and trip end_of_head. */
    if (!prev_xfer_rst && fds_xfer_reset)
    {
      fds_phase = FDS_PH_IDLE;
      fds_byte_pos = fds_block_start_pos;
    }
    /* Transfer reset deasserted: peek block type at byte_pos and
       start feeding payload. (No $80 mark in stream — the drive's
       hardware sync handles that on real HW.) */
    if (prev_xfer_rst && !fds_xfer_reset)
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
    /* Reading $4030 acknowledges both IRQ flags. */
    fds_timer_irq_pend = 0;
    fds_byte_irq_pend  = 0;
    return r;
  }

  case 0x4031: /* Disk read data */
    fds_byte_irq_pend = 0;
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
           fds_read_mode && FDS_DiskInserted && FDS_DiskImage &&
           fds_phase != FDS_PH_IDLE)
  {
    fds_cycle_acc += STEP_PER_SCANLINE;
    while (fds_cycle_acc >= FDS_CYC_PER_BYTE)
    {
      fds_cycle_acc -= FDS_CYC_PER_BYTE;
      fds_read_buf = fdsProduceByte();
      /* $4030 bit 1 is set on every byte produced, regardless of IRQ
         enable — polling code reads this even with IRQs disabled. */
      fds_byte_irq_pend = 1;
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
}
