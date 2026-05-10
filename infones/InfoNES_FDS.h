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

/* Buffers allocated in fdsParse / freed in fdsRelease.
   With PSRAM these live in PSRAM; without PSRAM they use SRAM heap. */
extern BYTE *FDS_Bios;
extern BYTE *FDS_PrgRam;
extern BYTE *FDS_ChrRam;
extern BYTE *FDS_DiskImage;     /* points into PSRAM or flash (read-only) */
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
   basePath is the path stem without extension (e.g. "/saves/game_fds").
   Multi-side mode (PSRAM) appends ".SAV" for a single sidecar file.
   Single-side mode (no PSRAM) appends "_s0.SAV", "_s1.SAV", etc.
   fdsSetSaveBasePath stores the path for internal side-swap use. */
bool fdsHasDirtyPages();
void fdsSetSaveBasePath(const char *basePath);
bool fdsLoadSidecar(const char *basePath);
bool fdsSaveSidecar(const char *basePath);

/* Detect by extension on the loaded ROM filename. RP2350 only. */
bool fdsIsFdsFilename(const char *filename);

/* Run the preflight gate (RP2350 + BIOS file present + enough memory),
   then load BIOS, allocate PRG/CHR-RAM, and bind the disk image bytes at
   fdsImage / fdsImageSize (PSRAM or flash). With PSRAM all sides are
   expanded into memory; without PSRAM only one side at a time is kept
   and rebuilt on disk swap. Returns true on success. On failure writes a
   user-visible message via InfoNES_Error and frees any buffers. */
bool fdsParse(BYTE *fdsImage, size_t fdsImageSize);

/* Free BIOS / PRG-RAM / CHR-RAM. Disk image is owned by the loader and
   is freed via the existing ROM_FILE_ADDR free path. */
void fdsRelease();

/* Deferred disk-side rebuild: called from the main emulation loop
   (shallow stack) to perform SD I/O that would overflow the stack
   if done from within the fdsHsync call chain. */
void fdsCheckPendingRebuild();

/* FDS expansion audio: reset state + render n samples into fds_wave_buffer.
   fdsResetAudio() is called internally from fdsResetDrive(). */
void fdsResetAudio();
void fdsRenderAudio(unsigned int n);

/* FDS audio enable flag and output buffer.  Allocated in Map20_Init,
   freed in InfoNES_pAPUDone.  Buffer holds APU_MAX_SAMPLES_PER_SYNC bytes. */
extern BYTE  ApuFdsEnable;
extern BYTE *fds_wave_buffer;

#endif /* INFONES_FDS_H */
