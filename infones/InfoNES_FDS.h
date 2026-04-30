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

/* Drive emulation hooks. Wired from Map20_Init. */
void fdsResetDrive();
void fdsApuWrite(WORD wAddr, BYTE byData);
BYTE fdsApuRead(WORD wAddr);
void fdsHsync();

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

#endif /* INFONES_FDS_H */
