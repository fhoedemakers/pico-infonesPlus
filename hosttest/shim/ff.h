// Host-build shim for FatFs. Backed by host stdio in stubs.cpp so that the
// FDS BIOS / sidecar paths actually work against files on the host filesystem.
// Only the subset of FatFs the InfoNES core uses is declared here.
#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int UINT;
typedef uint64_t FSIZE_t;

typedef enum {
    FR_OK = 0,
    FR_DISK_ERR = 1,
    FR_NO_FILE = 4,
    FR_INVALID_NAME = 6,
    FR_DENIED = 7,
    FR_NOT_ENOUGH_CORE = 17,
} FRESULT;

#define FA_READ          0x01
#define FA_WRITE         0x02
#define FA_CREATE_ALWAYS 0x08
#define FA_OPEN_ALWAYS   0x10

typedef struct {
    FSIZE_t fsize;
    void   *fp;     // host stdio FILE*
} FIL;

typedef struct {
    FSIZE_t fsize;
    char    fname[256];
} FILINFO;

typedef struct { int dummy; } DIR;

FRESULT f_open  (FIL *fp, const char *path, uint8_t mode);
FRESULT f_close (FIL *fp);
FRESULT f_read  (FIL *fp, void *buf, UINT btr, UINT *br);
FRESULT f_write (FIL *fp, const void *buf, UINT btw, UINT *bw);
FRESULT f_stat  (const char *path, FILINFO *fno);
FRESULT f_unlink(const char *path);
FRESULT f_mkdir (const char *path);

static inline FSIZE_t f_size(FIL *fp) { return fp ? fp->fsize : 0; }

#ifdef __cplusplus
}
#endif
