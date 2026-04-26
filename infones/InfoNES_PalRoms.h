#ifndef INFONES_PALROMS_H_INCLUDED
#define INFONES_PALROMS_H_INCLUDED

#include <stdint.h>

/* CRC32 -> PAL lookup. Returns true when the given ROM CRC is on the PAL
 * list, false (NTSC) otherwise. The table itself lives in InfoNES_PalRoms.cpp
 * — add new entries there. */
bool isRomPal(uint32_t crc);

#endif /* INFONES_PALROMS_H_INCLUDED */
