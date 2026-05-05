#ifndef INFONES_FDS_H
#define INFONES_FDS_H

#include "InfoNES_Types.h"

#define FDS_BIOS_PATH       "/bios/fds-bios.rom"
#define FDS_BIOS_SIZE       0x2000      /* 8 KB BIOS at $E000-$FFFF */
#define FDS_PRG_RAM_SIZE    0x8000      /* 32 KB PRG-RAM at $6000-$DFFF */
#define FDS_CHR_RAM_SIZE    0x2000      /* 8 KB CHR-RAM */
#define FDS_SIDE_SIZE       65500       /* bytes per FDS side */
#define FDS_FWNES_HDR_SIZE  16          /* optional fwNES header */
#define FDS_MAX_SIDES       8           /* upper bound for sanity checks */

/* PSRAM-backed buffers, allocated in fdsParse / freed in fdsRelease. */
extern BYTE *FDS_Bios;
extern BYTE *FDS_PrgRam;
extern BYTE *FDS_ChrRam;
extern BYTE *FDS_DiskImage;     /* points into the PSRAM buffer that holds the disk */
extern int   FDS_NumSides;
extern int   FDS_CurrentSide;
extern bool  FDS_DiskInserted;  /* set by phase 5 disk swap UI; defaults to true */
/* FDS_AutoInsertEnabled reads directly from settings.flags.autoSwapFDS */
#include "settings.h"
#define FDS_AutoInsertEnabled (settings.flags.autoSwapFDS)

/* Drive emulation hooks. Wired from Map20_Init. */
void fdsResetDrive();
void fdsApuWrite(WORD wAddr, BYTE byData);
BYTE fdsApuRead(WORD wAddr);
void fdsHsync();

/* Mesen2-style auto-disk-insert: called from K6502 step loop when
   PC == $E445 (BIOS disk-verify routine). Matches the 10-byte header
   buffer and auto-switches to the correct disk side. */
void fdsAutoInsertCheck();

/* Phase 5: disk swap UI hooks.
   - fdsRequestSwap(side): eject the current disk for ~1s of game time
     and then insert the requested side. The eject pulse is what makes
     the BIOS notice the change ($4032 bit 0).
   - fdsRequestEject(): eject and stay ejected.
   - fdsCurrentSwapValue(): UI helper — returns 0..N-1 for "Side N"
     or N for "Ejected" (pulse in flight or held). Used by the menu to
     render the current selection. */
void fdsRequestSwap(int newSide);
void fdsRequestEject();
int  fdsCurrentSwapValue();
int  fdsGetNumSides();

/* Phase 6: save-data persistence.
   - fdsHasDirtyPages(): true iff any 4 KB page of the disk image has
     been written since load. Skipping the SD round-trip when nothing
     changed avoids needless wear.
   - fdsLoadSidecar(path): if `path` exists and is a valid sidecar for
     the currently-loaded image, apply its dirty pages over the
     pristine .fds image in PSRAM. Returns true on success or absent.
   - fdsSaveSidecar(path): write a sparse sidecar (header + bitmap +
     dirty pages) to `path`. No-op if no pages dirty. */
bool fdsHasDirtyPages();
bool fdsLoadSidecar(const char *path);
bool fdsSaveSidecar(const char *path);

/* Detect by extension on the loaded ROM filename. RP2350 builds only. */
bool fdsIsFdsFilename(const char *filename);

/* Run the preflight gate (RP2350 + PSRAM + BIOS file present + memory),
   then load BIOS, allocate PRG/CHR-RAM, and bind the disk image bytes that
   already sit in PSRAM at fdsImage / fdsImageSize. Returns true on success.
   On failure writes a user-visible message via InfoNES_Error and frees any
   buffers it allocated so the caller can fall back to the menu. */
bool fdsParse(BYTE *fdsImage, size_t fdsImageSize);

/* Free BIOS / PRG-RAM / CHR-RAM. Disk image is owned by the loader and
   is freed via the existing ROM_FILE_ADDR free path. */
void fdsRelease();

/* FDS expansion audio: reset state + render n samples into fds_wave_buffer.
   fdsResetAudio() is called internally from fdsResetDrive(). */
void fdsResetAudio();
void fdsRenderAudio(unsigned int n);

/* FDS audio enable flag and output buffer.  Allocated in Map20_Init,
   freed in InfoNES_pAPUDone.  Buffer holds APU_MAX_SAMPLES_PER_SYNC bytes. */
extern BYTE  ApuFdsEnable;
extern BYTE *fds_wave_buffer;

#endif /* INFONES_FDS_H */
