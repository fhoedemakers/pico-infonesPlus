// Host-build shim. The real FrensHelpers.h pulls in DVI, FatFs wrappers,
// PSRAM helpers, Pico mutexes etc. — none of that is needed for headless
// emulation. This shim declares only the symbols the InfoNES core links
// against, plus the SCREENWIDTH/HEIGHT macros some code references.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>   // uint
#include "pico.h"        // brings __not_in_flash, __not_in_flash_func, etc.
#include "InfoNES_Types.h"

#define SCREENWIDTH  320
#define SCREENHEIGHT 240

#ifndef FRAMEBUFFERISPOSSIBLE
#define FRAMEBUFFERISPOSSIBLE 1
#endif
#ifndef HSTX
#define HSTX 0
#endif

namespace Frens
{
    extern WORD framebuffer[];   // 320 * 240, defined in stubs.cpp

    void *f_malloc(size_t size);
    void  f_free(void *p);

    bool  isPsramEnabled();
    uint  GetAvailableMemory();
    bool  isFrameBufferUsed();

    void  getextensionfromfilename(const char *filename, char *extension, size_t extSize);
}
