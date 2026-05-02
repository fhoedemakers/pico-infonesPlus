/*===================================================================*/
/*                                                                   */
/*  InfoNES_NSF.h : Nintendo Sound Format (.nsf) playback support    */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_NSF_H_INCLUDED
#define InfoNES_NSF_H_INCLUDED

#include "InfoNES_Types.h"
#include <cstddef>

/*-------------------------------------------------------------------*/
/*  NSF Header (128 bytes)                                           */
/*-------------------------------------------------------------------*/

struct NsfHeader_tag
{
    BYTE byID[5];            /* "NESM\x1a" */
    BYTE byVersion;          /* NSF version (usually 1) */
    BYTE byTotalSongs;       /* Number of songs */
    BYTE byStartingSong;     /* 1-indexed starting song */
    WORD wLoadAddress;       /* Load address (>= $8000 or $6000 with banking) */
    WORD wInitAddress;       /* INIT routine address */
    WORD wPlayAddress;       /* PLAY routine address */
    char szSongName[32];     /* Song name (null-terminated) */
    char szArtistName[32];   /* Artist name */
    char szCopyright[32];    /* Copyright holder */
    WORD wPlaySpeedNtsc;     /* Play speed in microseconds (NTSC) */
    BYTE byBankSwitch[8];    /* Bank switching init values ($5FF8-$5FFF) */
    WORD wPlaySpeedPal;      /* Play speed in microseconds (PAL) */
    BYTE byFlags;            /* bit 0: PAL/NTSC (0=NTSC, 1=PAL) */
    BYTE bySoundChips;       /* Expansion audio chip flags */
    BYTE byReserved[4];      /* Reserved (zero) */
};

/* Expansion sound chip flags (bySoundChips) */
#define NSF_CHIP_VRC6    0x01
#define NSF_CHIP_VRC7    0x02
#define NSF_CHIP_FDS     0x04
#define NSF_CHIP_MMC5    0x08
#define NSF_CHIP_NAMCO   0x10
#define NSF_CHIP_SUNSOFT 0x20

/*-------------------------------------------------------------------*/
/*  NSF global state                                                 */
/*-------------------------------------------------------------------*/

/* True when the loaded image is an NSF file */
extern bool IsNSF;

/* Parsed NSF header */
extern struct NsfHeader_tag NsfHeader;

/* Current track number (0-indexed) */
extern BYTE NsfCurrentTrack;

/*-------------------------------------------------------------------*/
/*  NSF functions                                                    */
/*-------------------------------------------------------------------*/

/* Parse an NSF file from raw data. Returns true on success. */
bool nsfParse(const BYTE *data, size_t size);

/* Release NSF resources */
void nsfRelease();

/* Select a specific track (0-indexed). Triggers re-init on next reset. */
void nsfSelectTrack(BYTE track);

/* Advance to next track */
void nsfNextTrack();

/* Go to previous track */
void nsfPrevTrack();

/* Called after K6502_Reset() to set up CPU state for NSF playback */
void nsfSetupCpuState();

/* Peak VU level per channel (0-255), updated each frame */
extern BYTE NsfVuLevels[5];

/* Update VU levels from wave_buffers (call once per frame) */
void nsfUpdateVuLevels();

/* Playback state: true when track is actively playing */
extern bool NsfIsPlaying;

/* Frame counter for current track (incremented each frame while playing) */
extern int NsfFrameCounter;

/* Maximum track duration in frames (3 minutes at ~60fps) */
#define NSF_MAX_TRACK_FRAMES (180 * 60)

/* Number of consecutive silent frames before auto-advancing */
#define NSF_SILENCE_FRAMES  (3 * 60)

/* Start playing the current track */
void nsfStartPlayback();

/* Stop (pause) playback */
void nsfStopPlayback();

/* Call once per frame: updates counters, detects silence,
   auto-advances to next track. Returns true if a track change
   occurred (caller should set resetGame). */
bool nsfUpdatePlayback();

/* Get progress as 0-255 (elapsed / max duration) */
BYTE nsfGetProgress();

/*-------------------------------------------------------------------*/
/*  NSF Mapper (mapper 31) functions                                 */
/*-------------------------------------------------------------------*/

void MapNsf_Init();

#endif /* !InfoNES_NSF_H_INCLUDED */
