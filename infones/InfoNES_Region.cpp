/*
 * Region detection for NES ROMs.
 *
 * The NES emulator core itself is region-agnostic; the calling shell (e.g.
 * main.cpp) decides PAL vs NTSC vs Dendy and passes the result into
 * InfoNES_Main(). This file provides InfoNES_DetectRegion(), which uses
 * three layered heuristics — NES 2.0 header, CRC32 database, filename tag —
 * to pick the right timing for a given cartridge image.
 */

#include "InfoNES_Region.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

// CRC32 of the PRG+CHR payload for ROMs known to be PAL releases.
// Sourced from the No-Intro database; used as a fallback when the iNES
// header lacks a reliable region field. Kept sorted to ease maintenance.
static const uint32_t nes_pal_crcs[] = {
    0x001388B3,
    0x00A53242,
    0x00AD1189,
    0x015D4555,
    0x02E0ADA4,
    0x03B8DEFA,
    0x04142764,
    0x0504B007,
    0x05104517,
    0x06406EB9,
    0x06689AA4,
    0x083E4FC1,
    0x096D8364,
    0x0B8F8128,
    0x0C783F0C,
    0x0DA28A50,
    0x127D76F4,
    0x13332BFA,
    0x138862C5,
    0x13E01649,
    0x14255C57,
    0x18A04825,
    0x19CE7F12,
    0x1B932BEA,
    0x1BC686A8,
    0x1C212E9D,
    0x1ED5C801,
    0x21F2A1A6,
    0x22AB9694,
    0x231BC76E,
    0x23BEFF5E,
    0x23D7D48F,
    0x24598791,
    0x24BA12DD,
    0x25551F3F,
    0x256392F1,
    0x27777635,
    0x284E65E8,
    0x2B1497DC,
    0x2B20B022,
    0x2C088DC5,
    0x2FD2E632,
    0x304FA926,
    0x30C5E6CF,
    0x34629104,
    0x348D3FF1,
    0x34BB757B,
    0x34C1E893,
    0x358E29DD,
    0x36C3B13A,
    0x37A5EB52,
    0x3824F7A5,
    0x38DE7053,
    0x38FBCC85,
    0x39D43261,
    0x3A990EE0,
    0x3AC0830A,
    0x3BB31E38,
    0x3E81DD67,
    0x4022C94E,
    0x41D32FD7,
    0x441DE6D8,
    0x46135141,
    0x46480432,
    0x464A67AB,
    0x46931EA0,
    0x47B6A39F,
    0x47FD88CF,
    0x4864C304,
    0x48B8EE58,
    0x48F68D40,
    0x4BB9B840,
    0x4D1DF589,
    0x4D345422,
    0x4EC0FECC,
    0x4ECD4624,
    0x4F089E8A,
    0x4F48B240,
    0x5112DC21,
    0x51BF28AF,
    0x51C51C35,
    0x524A5A32,
    0x53A9B53A,
    0x5529431F,
    0x565B1BDB,
    0x56F05853,
    0x585BA83D,
    0x5B2B72CB,
    0x5D2B1962,
    0x5D99053D,
    0x5E36D3BE,
    0x5E6D9975,
    0x607BD020,
    0x60A59624,
    0x60EA98A0,
    0x63DDD219,
    0x654F4E90,
    0x656D4265,
    0x66066326,
    0x666BE5EC,
    0x668D1715,
    0x66EBDB64,
    0x66F6A39E,
    0x671F23A8,
    0x6720ABAC,
    0x67CBC0A0,
    0x68C62E50,
    0x68F9B5F5,
    0x694C801F,
    0x6B761858,
    0x6DCBAAFD,
    0x6F27300B,
    0x6F97C721,
    0x7002FE8D,
    0x70860FCA,
    0x709C9399,
    0x70F31D2C,
    0x719571B3,
    0x71C01B19,
    0x73298C87,
    0x759418D2,
    0x76C161E3,
    0x7751588D,
    0x791138D9,
    0x795BC424,
    0x79F688BC,
    0x7A9BE620,
    0x7AE5C002,
    0x7B55D481,
    0x7BA3F8AE,
    0x7CB0D70D,
    0x7DA77F11,
    0x7E4BA78F,
    0x7C16F819,
    0x7F08D0D9,
    0x7F801368,
    0x80250D64,
    0x8106E694,
    0x81210F63,
    0x81AF4AF9,
    0x81B2A3CD,
    0x8293803A,
    0x836685C4,
    0x836FE2C2,
    0x837A3D8A,
    0x84C4A12E,
    0x84F7FC31,
    0x8650BE49,
    0x86867830,
    0x86C495C6,
    0x8897A8F1,
    0x88C30FDA,
    0x8904149E,
    0x892434DD,
    0x89821E2B,
    0x898E4232,
    0x89984244,
    0x89A45446,
    0x89EC53C8,
    0x8A0C7337,
    0x8A65E57C,
    0x8C88536F,
    0x8D9AD3BF,
    0x8DA6667D,
    0x8ECBC577,
    0x8F197B0A,
    0x8FA6E92C,
    0x9198279E,
    0x91B4B1D7,
    0x9237B447,
    0x9247C38D,
    0x924CDE0B,
    0x92924548,
    0x93484CC9,
    0x9369A2F8,
    0x94476A70,
    0x95CE3B58,
    0x967011AD,
    0x96CFB4D8,
    0x972D2784,
    0x9735D267,
    0x976893D2,
    0x998422FC,
    0x99C88648,
    0x9A2DB086,
    0x9B05B278,
    0x9B506A48,
    0x9B568CC4,
    0x9BD3F3C2,
    0x9C304DEC,
    0x9C4589E3,
    0x9C924719,
    0x9E379698,
    0x9F01687D,
    0x9F119033,
    0xA0A5A0B9,
    0xA1A0C13F,
    0xA1C0DA00,
    0xA31142FF,
    0xA4BDCC1D,
    0xA4DCF72E,
    0xA6638CBA,
    0xA66D5150,
    0xA713DD30,
    0xA8D93537,
    0xA93527E2,
    0xA97567A4,
    0xAA2F7D91,
    0xAB2006B4,
    0xAC3E5677,
    0xAC609320,
    0xAEB2D754,
    0xAF65AA84,
    0xB0BC46D1,
    0xB13F00D4,
    0xB1C937C8,
    0xB2EF7F4B,
    0xB400172A,
    0xB6B5C372,
    0xB79C320D,
    0xB80192B7,
    0xBBB710D9,
    0xBC06543C,
    0xBC25A18B,
    0xBC9BFFCB,
    0xBCCFEF1C,
    0xBD154C3E,
    0xBE0E93C3,
    0xBF700470,
    0xBF888B75,
    0xC0103592,
    0xC05A63B2,
    0xC076D66F,
    0xC0EDEDD0,
    0xC0F251EA,
    0x27CA0679,
    0xC30848D3,
    0xC4C3949A,
    0xC4E81924,
    0xC53CF1D0,
    0xC5657C12,
    0xC8EBD977,
    0xC8F203F9,
    0xC92B814B,
    0xC99B690A,
    0xCCDCBFC6,
    0xCF7CA9BD,
    0xCF849F72,
    0xD029F841,
    0xD0F70E36,
    0xD153CAF6,
    0xD161888B,
    0xD20BB617,
    0xD364F816,
    0xD44B412E,
    0xD49DCA84,
    0xD630EE8F,
    0xD6AD4E9D,
    0xD6F7383E,
    0xD72560E1,
    0xD78BFB28,
    0xD7B35F7D,
    0xD81612F0,
    0xD9F0749F,
    0xDB99D0CB,
    0xDB9C072D,
    0xDC529482,
    0xDDA190F9,
    0xDDC6D9C9,
    0xDE7E4629,
    0xDF4EDC13,
    0xDF67DAA1,
    0xE043C6A5,
    0xE0AC6242,
    0xE0FFFBD2,
    0xE1C59D94,
    0xE3027EBE,
    0xE402B134,
    0xE54138A9,
    0xE57E5384,
    0xE5901A99,
    0xE592F53A,
    0xE5A8401B,
    0xE5A972BE,
    0xE5FCC4C1,
    0xE62E3382,
    0xE681B300,
    0xE71D034E,
    0xE94E883D,
    0xE9D352EB,
    0xED3FA60E,
    0xED77B453,
    0xEEE0C7F8,
    0xEFB2B7E8,
    0xF0C198FF,
    0xF184EB2D,
    0xF46EF39A,
    0xF4B70BFE,
    0xF59CFC3D,
    0xF5A1B8FB,
    0xF5B2AFCA,
    0xF732C8FD,
    0xF919795D,
    0xFA7EE642,
    0xFB98D46E,
    0xFBD48274,
    0xFC2DA286,
    0xFC2F9B2D,
    0xFC3236D1,
    0xFC5026EE,
    0xFCD772EB,
    0xFCEBCC5F,
    0xFD7E9A7E,
    0xFE08D602,
    0xFF24D794,
    0xFFFDC310,
    0x999584A8, // Galaga
};

const int nes_pal_crcs_count = 316;

// CRC32 of ROMs known to target the Dendy (Russian Famiclone) timing —
// 50 Hz like PAL but with a different CPU/PPU divider, so the emulator
// needs to distinguish it from true PAL.
const uint32_t nes_dendy_crcs[] = {
    0x0AA49929,
    0x1460EC7B,
    0x2AF8332A,
    0x368C19A8,
    0x41EF9AC4,
    0x513EB779,
    0x543AB532,
    0x5E073A1B,
    0x61721163,
    0x6733607A,
    0x7BDD12F3,
    0x7CCB8D1E,
    0x82F1FB96,
    0x903A95EB,
    0xA9C07FF3,
    0xB066111A,
    0xBC7364BB,
    0xBD14FCFB,
    0xC7EDBC2E,
    0xC9745875,
    0xDC621DD1,
    0xDF343384,
    0xE9A7FE9E,
    0xEAD40557,
    0xEB6F80E8,
    0xEBB56E10,
    0xEF4DB05E,
    0xFAAD108A,
    0xFAF3582F,
};

const int nes_dendy_crcs_count = 29;


// 16-byte iNES / NES 2.0 file header, laid out to match the on-disk format
// so it can be cast directly over the start of a ROM image.
typedef struct {
    uint8_t magic[4];   // "NES\x1A"
    uint8_t prg_banks;
    uint8_t chr_banks;
    uint8_t flags6;
    uint8_t flags7;
    uint8_t flags8;     // NES 2.0: high bits of PRG/CHR size
    uint8_t flags9;     // NES 2.0: PRG/CHR size MSB
    uint8_t flags10;    // iNES: TV system (unreliable); NES 2.0: PRG RAM
    uint8_t flags11;    // NES 2.0: CHR RAM
    uint8_t flags12;    // NES 2.0: timing/region
    uint8_t flags13;
    uint8_t flags14;
    uint8_t flags15;
} NESHeader;

// Region encoding used by the NES 2.0 flags12 field (bits 1:0).
typedef enum {
    REGION_NTSC  = 0,
    REGION_PAL   = 1,
    REGION_MULTI = 2,   // supports both
    REGION_DENDY = 3,   // Russian clone
} NESRegion;

// True when the header advertises NES 2.0 format. The spec encodes this in
// flags7 bits 3:2 as the pattern 0b10 — distinguishing it from plain iNES
// (which leaves these bits clear) and from corrupted/archaic dumps.
static bool is_nes2(const NESHeader *h) {
    return (h->flags7 & 0x0C) == 0x08;
}

// Heuristic region guess based on No-Intro style filename tags such as
// "(Europe)" or "(USA)". Used only as a last resort when the header and
// CRC database both come up empty. Defaults to NTSC on no match.
static NESRegion region_from_filename(const char *filename) {
    // No-Intro tags: (Europe), (Australia), (Sweden), etc. → PAL
    // (USA), (Japan) → NTSC
    static const char *pal_tags[]  = {"(Europe)", "(Australia)", "(Sweden)",
                                       "(Germany)", "(France)", "(Spain)",
                                       "(Italy)", "(Netherlands)", "(E)", NULL};
    static const char *ntsc_tags[] = {"(USA)", "(Japan)", "(U)", "(J)", NULL};
    printf("Checking filename for region tags: %s\n", filename);
    for (int i = 0; pal_tags[i]; i++) {
        if (strstr(filename, pal_tags[i])) {
            printf("Found PAL tag in filename: %s\n", pal_tags[i]);
            return REGION_PAL;
        }
    }

    for (int i = 0; ntsc_tags[i]; i++) {
        if (strstr(filename, ntsc_tags[i])) {
            printf("Found NTSC tag in filename: %s\n", ntsc_tags[i]);
            return REGION_NTSC;
        }
    }
    printf("No region tag found in filename, assuming NTSC.\n");
    return REGION_NTSC; // default assumption
}

// Decide which timing the emulator core should use for the ROM at `addr`.
//
// Parameters:
//   addr    - pointer (as an integer) to the first byte of the iNES image,
//             i.e. the start of the 16-byte header.
//   crc     - CRC32 of the PRG+CHR payload (header excluded), used for the
//             database lookup step.
//   romName - filename of the ROM, consulted only as a final fallback.
//
// Returns:
//   0 = NTSC (default), 1 = PAL, 2 = Dendy.
//
// Detection priority, highest to lowest:
//   1. NES 2.0 header flags12 region bits — authoritative when present.
//   2. CRC32 lookup against the curated PAL/Dendy tables above.
//   3. No-Intro style filename tag.
// An invalid header short-circuits to NTSC to avoid acting on garbage.
int InfoNES_DetectRegion(uintptr_t addr, uint32_t crc, const char* romName)
{
    auto *p = reinterpret_cast<const uint8_t *>(addr);
    NESHeader *h = (NESHeader *)p;
    printf("Detecting region for ROM CRC %08X\n", crc);
    // Guard against garbage input — without this, random bytes could hit
    // the NES 2.0 path below and yield a bogus region from flags12.
    if (h->magic[0] != 'N' || h->magic[1] != 'E' || h->magic[2] != 'S' || h->magic[3] != 0x1A) {
        printf("Invalid NES header magic: %02X %02X %02X %02X\n", h->magic[0], h->magic[1], h->magic[2], h->magic[3]);
        return 0; // default to NTSC if not a valid NES header
    }
    // 1. NES 2.0 header is authoritative
    if (is_nes2(h)) {
        printf("NES 2.0 header detected.\n");
        int region = (h->flags12 >> 2) & 0x03;
        // print the region for debugging
        switch (region) {
            case REGION_NTSC:  printf("Region: NTSC\n"); break; 
                case REGION_PAL:   printf("Region: PAL\n"); break; 
                case REGION_MULTI: printf("Region: MULTI\n"); break; 
                case REGION_DENDY: printf("Region: DENDY\n"); break;
        }
        if (region == REGION_PAL)
            return 1;
        if (region == REGION_DENDY)
            return 2;
        return 0;
    } else {
        printf("Not an NES 2.0 header (flags7=%02X); falling back to CRC lookup.\n", h->flags7);
    }
    // 2. Database lookup by CRC32
    for (auto c : nes_pal_crcs)
    {
        if (c == crc)
            return 1;
    }
    for (auto c : nes_dendy_crcs)
    {
        if (c == crc)
            return 2;
    }
    printf("CRC not found in PAL/Dendy lists; falling back to filename-based detection.\n");
    // 3. Fallback to filename-based detection
    NESRegion region = region_from_filename(romName);
    if (region == REGION_PAL)
        return 1;
    if (region == REGION_DENDY)
        return 2;

    
    return 0;
}
