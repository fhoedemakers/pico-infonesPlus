#ifndef INFONES_PALROMS_H_INCLUDED
#define INFONES_PALROMS_H_INCLUDED

#include <stdint.h>

/* CRC32 -> PAL lookup. Returns 2 when the given ROM CRC is on the Dendy
 * list, 1 when on the PAL list, 0 (NTSC) otherwise. The tables themselves
 * live in InfoNES_PalRoms.cpp — add new entries there. */
int isRomPal(uint32_t crc);

#endif /* INFONES_PALROMS_H_INCLUDED */
