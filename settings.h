#ifndef SETTINGS
#define SETTINGS
#include "ff.h"
#define SETTINGSFILE "/settings.dat"	// File to store settings
extern struct settings settings;
enum class ScreenMode
    {
        SCANLINE_8_7,
        NOSCANLINE_8_7,
        SCANLINE_1_1,
        NOSCANLINE_1_1,
        MAX,
    };
#define CBLACK 15
#define CWHITE 48
#define CRED 6
#define CGREEN 0x2A
#define CBLUE 2
#define CLIGHTBLUE 0x11
#define DEFAULT_FGCOLOR CBLACK // 60
#define DEFAULT_BGCOLOR CWHITE
struct settings {
    ScreenMode screenMode;
    int firstVisibleRowINDEX;
    int selectedRow;
    int horzontalScrollIndex;
    int fgcolor;
    int bgcolor;
    //int reserved[3];
    char currentDir[FF_MAX_LFN];
};
void savesettings();
void loadsettings();
void resetsettings();
#endif