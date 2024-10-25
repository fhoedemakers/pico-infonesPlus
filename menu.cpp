#include <stdio.h>
#include <string.h>

#include "FrensHelpers.h"
#include <pico.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "InfoNES_System.h"
#include <util/exclusive_proc.h>
#include <gamepad.h>
#include "hardware/watchdog.h"
#include "InfoNES.h"
#include "RomLister.h"
#include "menu.h"
#include "nespad.h"
#include "wiipad.h"

#include "font_8x8.h"
#include "settings.h"

#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 8
#define FONT_N_CHARS 95
#define FONT_FIRST_ASCII 32
#define SCREEN_COLS 32
#define SCREEN_ROWS 29

#define STARTROW 3
#define ENDROW 24
#define PAGESIZE (ENDROW - STARTROW + 1)

#define VISIBLEPATHSIZE (SCREEN_COLS - 3)   

extern util::ExclusiveProc exclProc_;
void screenMode(int incr);
extern const WORD __not_in_flash_func(NesPalette)[];

static char connectedGamePadName[sizeof(io::GamePadState::GamePadName)];

#define CBLACK 15
#define CWHITE 48
#define CRED 6
#define CGREEN 0x2A
#define CBLUE 2
#define CLIGHTBLUE 0x11
#define DEFAULT_FGCOLOR CBLACK // 60
#define DEFAULT_BGCOLOR CWHITE

static int fgcolor = DEFAULT_FGCOLOR;
static int bgcolor = DEFAULT_BGCOLOR;

struct charCell
{
    uint8_t fgcolor;
    uint8_t bgcolor;
    char charvalue;
};

#define SCREENBUFCELLS SCREEN_ROWS *SCREEN_COLS
static charCell *screenBuffer;

static WORD *WorkLineRom = nullptr;
void RomSelect_SetLineBuffer(WORD *p, WORD size)
{
    WorkLineRom = p;
}

static constexpr int LEFT = 1 << 6;
static constexpr int RIGHT = 1 << 7;
static constexpr int UP = 1 << 4;
static constexpr int DOWN = 1 << 5;
static constexpr int SELECT = 1 << 2;
static constexpr int START = 1 << 3;
static constexpr int A = 1 << 0;
static constexpr int B = 1 << 1;
static constexpr int X = 1 << 8;
static constexpr int Y = 1 << 9;

char getcharslicefrom8x8font(char c, int rowInChar)
{
    return font_8x8[(c - FONT_FIRST_ASCII) + (rowInChar)*FONT_N_CHARS];
}
void RomSelect_PadState(DWORD *pdwPad1, bool ignorepushed = false)
{

    static DWORD prevButtons{};
    auto &gp = io::getCurrentGamePadState(0);
    strcpy(connectedGamePadName, gp.GamePadName);

    int v = (gp.buttons & io::GamePadState::Button::LEFT ? LEFT : 0) |
            (gp.buttons & io::GamePadState::Button::RIGHT ? RIGHT : 0) |
            (gp.buttons & io::GamePadState::Button::UP ? UP : 0) |
            (gp.buttons & io::GamePadState::Button::DOWN ? DOWN : 0) |
            (gp.buttons & io::GamePadState::Button::A ? A : 0) |
            (gp.buttons & io::GamePadState::Button::B ? B : 0) |
            (gp.buttons & io::GamePadState::Button::SELECT ? SELECT : 0) |
            (gp.buttons & io::GamePadState::Button::START ? START : 0) |
            (gp.buttons & io::GamePadState::Button::X ? X : 0) |
            (gp.buttons & io::GamePadState::Button::Y ? Y : 0) |
            0;
#if NES_PIN_CLK != -1
    v |= nespad_states[0];
#endif
#if NES_PIN_CLK_1 != -1
    v |= nespad_states[1];
#endif
#if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0
    v |= wiipad_read();
#endif

    *pdwPad1 = 0;

    unsigned long pushed;

    if (ignorepushed == false)
    {
        pushed = v & ~prevButtons;
    }
    else
    {
        pushed = v;
    }
    // if (p1 & SELECT)
    // {
    //     if (pushed & UP)
    //     {
    //         screenMode(-1);
    //         v = 0;
    //     }
    //     else if (pushed & DOWN)
    //     {
    //         screenMode(+1);
    //         v = 0;
    //     }
    // }
    if (pushed)
    {
        *pdwPad1 = v;
    }
    prevButtons = v;
}
void RomSelect_DrawLine(int line, int selectedRow)
{
    WORD fgcolor, bgcolor;
    memset(WorkLineRom, 0, 640);

    for (auto i = 0; i < SCREEN_COLS; ++i)
    {
        int charIndex = i + line / FONT_CHAR_HEIGHT * SCREEN_COLS;
        int row = charIndex / SCREEN_COLS;
        uint c = screenBuffer[charIndex].charvalue;
        if (row == selectedRow)
        {
            fgcolor = NesPalette[screenBuffer[charIndex].bgcolor];
            bgcolor = NesPalette[screenBuffer[charIndex].fgcolor];
        }
        else
        {
            fgcolor = NesPalette[screenBuffer[charIndex].fgcolor];
            bgcolor = NesPalette[screenBuffer[charIndex].bgcolor];
        }

        int rowInChar = line % FONT_CHAR_HEIGHT;
        char fontSlice = getcharslicefrom8x8font(c, rowInChar); // font_8x8[(c - FONT_FIRST_ASCII) + (rowInChar)*FONT_N_CHARS];
        for (auto bit = 0; bit < 8; bit++)
        {
            if (fontSlice & 1)
            {
                *WorkLineRom = fgcolor;
            }
            else
            {
                *WorkLineRom = bgcolor;
            }
            fontSlice >>= 1;
            WorkLineRom++;
        }
    }
    return;
}

void drawline(int scanline, int selectedRow)
{
    RomSelect_PreDrawLine(scanline);
    RomSelect_DrawLine(scanline - 4, selectedRow);
    InfoNES_PostDrawLine(scanline);
}

static void putText(int x, int y, const char *text, int fgcolor, int bgcolor)
{

    if (text != nullptr)
    {
        auto index = y * SCREEN_COLS + x;
        auto maxLen = strlen(text);
        if (strlen(text) > SCREEN_COLS - x)
        {
            maxLen = SCREEN_COLS - x;
        }
        while (index < SCREENBUFCELLS && *text && maxLen > 0)
        {
            screenBuffer[index].charvalue = *text++;
            screenBuffer[index].fgcolor = fgcolor;
            screenBuffer[index].bgcolor = bgcolor;
            index++;
            maxLen--;
        }
    }
}

void DrawScreen(int selectedRow)
{
    const char *spaces = "                   ";
    char tmpstr[sizeof(connectedGamePadName) + 4];
    if (selectedRow != -1)
    {
        putText(SCREEN_COLS / 2 - strlen(spaces) / 2, SCREEN_ROWS - 1, spaces, bgcolor, bgcolor);
        snprintf(tmpstr,sizeof(tmpstr), "- %s -", connectedGamePadName[0] != 0 ? connectedGamePadName : "No USB GamePad");
        putText(SCREEN_COLS / 2 - strlen(tmpstr) / 2, SCREEN_ROWS - 1, tmpstr, CBLUE, CWHITE);
    }
   
    for (auto line = 4; line < 236; line++)
    {
        drawline(line, selectedRow);
    }
}

void ClearScreen(charCell *screenBuffer, int color)
{
    for (auto i = 0; i < SCREENBUFCELLS; i++)
    {
        screenBuffer[i].bgcolor = color;
        screenBuffer[i].fgcolor = color;
        screenBuffer[i].charvalue = ' ';
    }
}

void displayRoms(Frens::RomLister romlister, int startIndex)
{
    char buffer[ROMLISTER_MAXPATH + 4];
    char s[SCREEN_COLS + 1];
    auto y = STARTROW;
    auto entries = romlister.GetEntries();
    ClearScreen(screenBuffer, bgcolor);
    strcpy(s, "- Pico-InfoNES+ -");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 0, s, fgcolor, bgcolor);
    
    strcpy(s, "Choose a rom to play:");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 1, s, fgcolor, bgcolor);
    // strcpy(s, "---------------------");
    // putText(SCREEN_COLS / 2 - strlen(s) / 2, 1, s, fgcolor, bgcolor);

    for (int i = 1; i < SCREEN_COLS - 1; i++)
    {
        putText(i, STARTROW - 1, "-", fgcolor, bgcolor);
    }
    for (int i = 1; i < SCREEN_COLS - 1; i++)
    {
        putText(i, ENDROW + 1, "-", fgcolor, bgcolor);
    }
    strcpy(s, "A Select, B Back");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, ENDROW + 2, s, fgcolor, bgcolor);
    putText(SCREEN_COLS - strlen(SWVERSION), SCREEN_ROWS - 1, SWVERSION, fgcolor, bgcolor);
    for (auto index = startIndex; index < romlister.Count(); index++)
    {
        if (y <= ENDROW)
        {
            auto info = entries[index];
            if (info.IsDirectory)
            {
                // snprintf(buffer, sizeof(buffer), "D %s", info.Path);
                snprintf(buffer, SCREEN_COLS - 1, "D %s", info.Path);
            }
            else
            {
                // snprintf(buffer, sizeof(buffer), "R %s", info.Path);
                snprintf(buffer, SCREEN_COLS - 1, "R %s", info.Path);
            }

            putText(1, y, buffer, fgcolor, bgcolor);
            y++;
        }
    }
}

void DisplayFatalError(char *error)
{
    ClearScreen(screenBuffer, bgcolor);
    putText(0, 0, "Fatal error:", fgcolor, bgcolor);
    putText(1, 3, error, fgcolor, bgcolor);
    while (true)
    {
        auto frameCount = InfoNES_LoadFrame();
        DrawScreen(-1);
    }
}

void DisplayEmulatorErrorMessage(char *error)
{
    DWORD PAD1_Latch;
    ClearScreen(screenBuffer, bgcolor);
    putText(0, 0, "Error occured:", fgcolor, bgcolor);
    putText(0, 3, error, fgcolor, bgcolor);
    putText(0, ENDROW, "Press a button to continue.", fgcolor, bgcolor);
    while (true)
    {
        auto frameCount = InfoNES_LoadFrame();
        DrawScreen(-1);
        RomSelect_PadState(&PAD1_Latch);
        if (PAD1_Latch > 0)
        {
            return;
        }
    }
}

void showSplashScreen()
{
    DWORD PAD1_Latch;
    char s[SCREEN_COLS + 1];
    ClearScreen(screenBuffer, bgcolor);

    strcpy(s, "Pico-Info");
    putText(SCREEN_COLS / 2 - (strlen(s) + 4) / 2, 2, s, fgcolor, bgcolor);

    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 7, 2, "N", CRED, bgcolor);
    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 8, 2, "E", CGREEN, bgcolor);
    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 9, 2, "S", CBLUE, bgcolor);
    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 10, 2, "+", fgcolor, bgcolor);

    strcpy(s, "NES emulator for RP2040");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 3, s, fgcolor, bgcolor);
    strcpy(s, "Emulator");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 5, s, fgcolor, bgcolor);
    strcpy(s, "@jay_kumogata");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 6, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "Pico Port");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 9, s, fgcolor, bgcolor);
    strcpy(s, "@shuichi_takano");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 10, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "Menu System & SD Card Support");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 13, s, fgcolor, bgcolor);
    strcpy(s, "@frenskefrens");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 14, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "NES/WII controller support");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 17, s, fgcolor, bgcolor);

    strcpy(s, "@PaintYourDragon @adafruit");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 18, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "PCB Design");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 21, s, fgcolor, bgcolor);

    strcpy(s, "@johnedgarpark");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 22, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "https://github.com/");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 25, s, CLIGHTBLUE, bgcolor);
    strcpy(s, "fhoedemakers/pico-infonesPlus");
    putText(1, 26, s, CLIGHTBLUE, bgcolor);
    int startFrame = -1;
    while (true)
    {
        auto frameCount = InfoNES_LoadFrame();
        if (startFrame == -1)
        {
            startFrame = frameCount;
        }
        DrawScreen(-1);
        RomSelect_PadState(&PAD1_Latch);
        if (PAD1_Latch > 0 || (frameCount - startFrame) > 1000)
        {
            return;
        }
        if ((frameCount % 30) == 0)
        {
            for (auto i = 0; i < SCREEN_COLS; i++)
            {
                auto col = rand() % 63;
                putText(i, 0, " ", col, col);
                col = rand() % 63;
                putText(i, SCREEN_ROWS - 1, " ", col, col);
            }
            for (auto i = 1; i < SCREEN_ROWS - 1; i++)
            {
                auto col = rand() % 63;
                putText(0, i, " ", col, col);
                col = rand() % 63;
                putText(SCREEN_COLS - 1, i, " ", col, col);
            }
        }
    }
}

void screenSaver()
{
    DWORD PAD1_Latch;
    WORD frameCount;
    while (true)
    {
        frameCount = InfoNES_LoadFrame();
        DrawScreen(-1);
        RomSelect_PadState(&PAD1_Latch);
        if (PAD1_Latch > 0)
        {
            return;
        }
        if ((frameCount % 3) == 0)
        {
            auto color = rand() % 63;
            auto row = rand() % SCREEN_ROWS;
            auto column = rand() % SCREEN_COLS;
            putText(column, row, " ", color, color);
        }
    }
}
// Global instances of local vars in romselect() some used in Lambda expression later on
static char *selectedRomOrFolder;
static uintptr_t FLASH_ADDRESS;
static bool errorInSavingRom = false;
static char *globalErrorMessage;

static bool showSplash = true;

void menu(uintptr_t NES_FILE_ADDR, char *errorMessage, bool isFatal)
{
    FLASH_ADDRESS = NES_FILE_ADDR;
    // int firstVisibleRowINDEX = 0;
    // int selectedRow = STARTROW;
    // char currentDir[FF_MAX_LFN];
    // int horzontalScrollIndex = 0;
    int totalFrames = -1;
    if (settings.selectedRow <= 0)
    {
        settings.selectedRow = STARTROW;
    }
    globalErrorMessage = errorMessage;
    FRESULT fr;
    DWORD PAD1_Latch;

    printf("Starting Menu\n");
    // allocate buffers
    size_t ramsize = 0x2000;
    screenBuffer = (charCell *)malloc(0x2000); // (charCell *)InfoNes_GetRAM(&ramsize);
    size_t chr_size = 32768;
    void *buffer = (void *)malloc(chr_size); // InfoNes_GetChrBuf(&chr_size);
    Frens::RomLister romlister(buffer, chr_size);

    if (strlen(errorMessage) > 0)
    {
        if (isFatal) // SD card not working, show error
        {
            DisplayFatalError(errorMessage);
        }
        else
        {
            DisplayEmulatorErrorMessage(errorMessage); // Emulator cannot start, show error
        }
        showSplash = false;
    }
    if (showSplash)
    {
        showSplash = false;
        showSplashScreen();
    }
    romlister.list(settings.currentDir);
    displayRoms(romlister, settings.firstVisibleRowINDEX);
    while (1)
    {

        auto frameCount = InfoNES_LoadFrame();
        auto index = settings.selectedRow - STARTROW + settings.firstVisibleRowINDEX;
        auto entries = romlister.GetEntries();
        selectedRomOrFolder = (romlister.Count() > 0) ? entries[index].Path : nullptr;
        errorInSavingRom = false;
        DrawScreen(settings.selectedRow);
        RomSelect_PadState(&PAD1_Latch);
        if (PAD1_Latch > 0)
        {
            totalFrames = frameCount; // Reset screenSaver
            // reset horizontal scroll of highlighted row
            settings.horzontalScrollIndex = 0;
            putText(3, settings.selectedRow, selectedRomOrFolder, fgcolor, bgcolor);
            putText(SCREEN_COLS - 1, settings.selectedRow, " ", bgcolor, bgcolor);
            // if ((PAD1_Latch & Y) == Y)
            // {
            //     fgcolor++;
            //     if (fgcolor > 63)
            //     {
            //         fgcolor = 0;
            //     }
            //     printf("fgColor++ : %02d (%04x)\n", fgcolor, NesPalette[fgcolor]);
            //     displayRoms(romlister, firstVisibleRowINDEX);
            // }
            // else if ((PAD1_Latch & X) == X)
            // {
            //     bgcolor++;
            //     if (bgcolor > 63)
            //     {
            //         bgcolor = 0;
            //     }
            //     printf("bgColor++ : %02d (%04x)\n", bgcolor, NesPalette[bgcolor]);
            //     displayRoms(romlister, firstVisibleRowINDEX);
            // }
            // else
            if ((PAD1_Latch & UP) == UP && selectedRomOrFolder)
            {
                if (settings.selectedRow > STARTROW)
                {
                    settings.selectedRow--;
                }
                else
                {
                    if (settings.firstVisibleRowINDEX > 0)
                    {
                        settings.firstVisibleRowINDEX--;
                        displayRoms(romlister, settings.firstVisibleRowINDEX);
                    }
                }
            }
            else if ((PAD1_Latch & DOWN) == DOWN && selectedRomOrFolder)
            {
                if (settings.selectedRow < ENDROW && (index) < romlister.Count() - 1)
                {
                    settings.selectedRow++;
                }
                else
                {
                    if (index < romlister.Count() - 1)
                    {
                        settings.firstVisibleRowINDEX++;
                        displayRoms(romlister, settings.firstVisibleRowINDEX);
                    }
                }
            }
            else if ((PAD1_Latch & LEFT) == LEFT && selectedRomOrFolder)
            {
                settings.firstVisibleRowINDEX -= PAGESIZE;
                if (settings.firstVisibleRowINDEX < 0)
                {
                    settings.firstVisibleRowINDEX = 0;
                }
                settings.selectedRow = STARTROW;
                displayRoms(romlister, settings.firstVisibleRowINDEX);
            }
            else if ((PAD1_Latch & RIGHT) == RIGHT && selectedRomOrFolder)
            {
                if (settings.firstVisibleRowINDEX + PAGESIZE < romlister.Count())
                {
                    settings.firstVisibleRowINDEX += PAGESIZE;
                }
                settings.selectedRow = STARTROW;
                displayRoms(romlister, settings.firstVisibleRowINDEX);
            }
            else if ((PAD1_Latch & B) == B)
            {
                fr = f_getcwd(settings.currentDir, FF_MAX_LFN);
                if (fr == FR_OK)
                {

                    if (strcmp(settings.currentDir, "/") != 0)
                    {
                        romlister.list("..");
                        settings.firstVisibleRowINDEX = 0;
                        settings.selectedRow = STARTROW;
                        displayRoms(romlister, settings.firstVisibleRowINDEX);
                        fr = f_getcwd(settings.currentDir, FF_MAX_LFN);
                        if (fr == FR_OK)
                        {
                            printf("Current dir: %s\n", settings.currentDir);
                        }
                        else
                        {
                            printf("Cannot get current dir: %d\n", fr);
                        }
                    }
                }
                else
                {
                    printf("Cannot get current dir: %d\n", fr);
                }
            }
            else if ((PAD1_Latch & START) == START && (PAD1_Latch & SELECT) != SELECT)
            {
                // reboot and start emulator with currently loaded game
                // Create a file /START indicating not to reflash the already flashed game
                // The emulator will delete this file after loading the game
                FRESULT fr;
                FIL fil;
                printf("Creating /START\n");
                fr = f_open(&fil, "/START", FA_CREATE_ALWAYS | FA_WRITE);
                if (fr == FR_OK)
                {
                    auto bytes = f_puts("START", &fil);
                    printf("Wrote %d bytes\n", bytes);
                    fr = f_close(&fil);
                    if (fr != FR_OK)
                    {
                        printf("Cannot close file /START:%d\n", fr);
                    }
                }
                else
                {
                    printf("Cannot create file /START:%d\n", fr);
                }
                break; // reboot
            }
            else if ((PAD1_Latch & A) == A && selectedRomOrFolder)
            {

                if (entries[index].IsDirectory)
                {

                    romlister.list(selectedRomOrFolder);
                    settings.firstVisibleRowINDEX = 0;
                    settings.selectedRow = STARTROW;
                    displayRoms(romlister, settings.firstVisibleRowINDEX);
                    // get full path name of folder
                    fr = f_getcwd(settings.currentDir, FF_MAX_LFN);
                    if (fr != FR_OK)
                    {
                        printf("Cannot get current dir: %d\n", fr);
                    }
                    printf("Current dir: %s\n", settings.currentDir);
                }
                else
                {
                    FRESULT fr;
                    FIL fil;
                    char curdir[FF_MAX_LFN];

                    fr = f_getcwd(curdir, sizeof(curdir));
                    printf("Current dir: %s\n", curdir);
                    // Create file containing full path name currently loaded rom
                    // The contents of this file will be used by the emulator to flash and start the correct rom in main.cpp
                    printf("Creating %s\n", ROMINFOFILE);
                    fr = f_open(&fil, ROMINFOFILE, FA_CREATE_ALWAYS | FA_WRITE);
                    if (fr == FR_OK)
                    {
                        for (auto i = 0; i < strlen(curdir); i++)
                        {

                            int x = f_putc(curdir[i], &fil);
                            printf("%c", curdir[i]);
                            if (x < 0)
                            {
                                snprintf(globalErrorMessage, 40, "Error writing file %d", fr);
                                printf("%s\n", globalErrorMessage);
                                errorInSavingRom = true;
                                break;
                            }
                        }
                        f_putc('/', &fil);
                        printf("%c", '/');
                        for (auto i = 0; i < strlen(selectedRomOrFolder); i++)
                        {

                            int x = f_putc(selectedRomOrFolder[i], &fil);
                            printf("%c", selectedRomOrFolder[i]);
                            if (x < 0)
                            {
                                snprintf(globalErrorMessage, 40, "Error writing file %d", fr);
                                printf("%s\n", globalErrorMessage);
                                errorInSavingRom = true;
                                break;
                            }
                        }
                        printf("\n");
                    }
                    else
                    {
                        printf("Cannot create %s:%d\n", ROMINFOFILE, fr);
                        snprintf(globalErrorMessage, 40, "Cannot create %s:%d", ROMINFOFILE, fr);
                        errorInSavingRom = true;
                    }
                    f_close(&fil);
                    // break out of loop and reboot
                    // rom will be flashed and started by main.cpp
                    // Cannot flash here because of lockups (when using wii controller) and sound issues
                    break;
                }
            }
        }
        // scroll selected row horizontally if textsize exceeds rowlength
        if (selectedRomOrFolder)
        {
            if ((frameCount % 30) == 0)
            {
                if (strlen(selectedRomOrFolder + settings.horzontalScrollIndex) >= VISIBLEPATHSIZE)
                {
                    settings.horzontalScrollIndex++;
                }
                else
                {
                    settings.horzontalScrollIndex = 0;
                }
                putText(3, settings.selectedRow, selectedRomOrFolder + settings.horzontalScrollIndex, fgcolor, bgcolor);
                putText(SCREEN_COLS - 1, settings.selectedRow, " ", bgcolor, bgcolor);
            }
        }
        if (totalFrames == -1)
        {
            totalFrames = frameCount;
        }
        if ((frameCount - totalFrames) > 800)
        {
            printf("Starting screensaver\n");
            totalFrames = -1;
            screenSaver();
            displayRoms(romlister, settings.firstVisibleRowINDEX);
        }
    } // while 1
    // Wait until user has released all buttons
    while (1)
    {
        InfoNES_LoadFrame();
        DrawScreen(-1);
        RomSelect_PadState(&PAD1_Latch, true);
        if (PAD1_Latch == 0)
        {
            break;
        }
    }
    free(screenBuffer);
    free(buffer);
#if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0
    wiipad_end();
#endif

    savesettings();

    // Don't return from this function call, but reboot in order to get avoid several problems with sound and lockups (WII-pad)
    // After reboot the emulator will and flash start the selected game.
    printf("Rebooting...\n");
    watchdog_enable(100, 1);
    while (1)
        ;
    // Never return
}
