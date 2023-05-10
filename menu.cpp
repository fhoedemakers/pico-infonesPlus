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
#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 8
#define FONT_N_CHARS 95
#define FONT_FIRST_ASCII 32
#define SCREEN_COLS 32
#define SCREEN_ROWS 29

#define STARTROW 2
#define ENDROW 25
#define PAGESIZE (ENDROW - STARTROW + 1)

#define VISIBLEPATHSIZE (SCREEN_COLS - 3)

extern util::ExclusiveProc exclProc_;
void screenMode(int incr);
extern const WORD __not_in_flash_func(NesPalette)[];

#define CBLACK 15
#define CWHITE 48
#define CRED 6
#define CGREEN 10
#define CBLUE 2
#define CLIGHTBLUE 0x2C
#define DEFAULT_FGCOLOR CBLACK // 60
#define DEFAULT_BGCOLOR CWHITE

static int fgcolor = DEFAULT_FGCOLOR;
static int bgcolor = DEFAULT_BGCOLOR;

struct charCell
{
    uint8_t fgcolor : 4;
    uint8_t bgcolor : 4;
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

char getcharslicefrom8x8font(char c, int rowInChar) {
    return font_8x8[(c - FONT_FIRST_ASCII) + (rowInChar)*FONT_N_CHARS];
}
void RomSelect_PadState(DWORD *pdwPad1, bool ignorepushed = false)
{

    static DWORD prevButtons{};
    auto &gp = io::getCurrentGamePadState(0);

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
    v |= nespad_state;
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
        char fontSlice = getcharslicefrom8x8font(c, rowInChar); //font_8x8[(c - FONT_FIRST_ASCII) + (rowInChar)*FONT_N_CHARS];
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
    auto y = STARTROW;
    auto entries = romlister.GetEntries();
    ClearScreen(screenBuffer, bgcolor);
    putText(1, 0, "Choose a rom to play:", fgcolor, bgcolor);
    putText(1, SCREEN_ROWS - 1, "A: Select, B: Back", fgcolor, bgcolor);
    for (auto index = startIndex; index < romlister.Count(); index++)
    {
        if (y <= ENDROW)
        {
            auto info = entries[index];
            if (info.IsDirectory)
            {
                snprintf(buffer, sizeof(buffer), "D %s", info.Path);
            }
            else
            {
                snprintf(buffer, sizeof(buffer), "R %s", info.Path);
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
    char s[SCREEN_COLS];
    ClearScreen(screenBuffer, bgcolor);

    strcpy(s, "Pico-Info");
    putText(SCREEN_COLS / 2 - (strlen(s) + 4) / 2, 2, s, fgcolor, bgcolor);

    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 7, 2, "N", CRED, bgcolor);
    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 8, 2, "E", CGREEN, bgcolor);
    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 9, 2, "S", CBLUE, bgcolor);
    putText((SCREEN_COLS / 2 - (strlen(s)) / 2) + 10, 2, "+", fgcolor, bgcolor);
    strcpy(s, "Emulator");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 5, s, fgcolor, bgcolor);
     strcpy(s, "@jay_kumogata");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 6, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "Pico Port");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 9, s, fgcolor, bgcolor);
     strcpy(s, "@shuichi_takano");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 10, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "Menu System & SD Card Support" );
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 13, s, fgcolor, bgcolor);
    strcpy(s, "@frenskefrens");
    putText(SCREEN_COLS / 2 - strlen(s) / 2, 14, s, CLIGHTBLUE, bgcolor);

    strcpy(s, "(S)NES controller support");
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
    int firstVisibleRowINDEX = 0;
    int selectedRow = STARTROW;
    char currentDir[FF_MAX_LFN];
    int totalFrames = -1;

    globalErrorMessage = errorMessage;
    FRESULT fr;
    DWORD PAD1_Latch;

    int horzontalScrollIndex = 0;
    printf("Starting Menu\n");
    size_t ramsize;
    // Borrow Emulator RAM buffer for screen.
    screenBuffer = (charCell *)InfoNes_GetRAM(&ramsize);
    size_t chr_size;
    // Borrow ChrBuffer to store directory contents
    void *buffer = InfoNes_GetChrBuf(&chr_size);
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
    romlister.list("/");
    displayRoms(romlister, firstVisibleRowINDEX);
    while (1)
    {

        auto frameCount = InfoNES_LoadFrame();
        auto index = selectedRow - STARTROW + firstVisibleRowINDEX;
        auto entries = romlister.GetEntries();
        selectedRomOrFolder = (romlister.Count() > 0) ? entries[index].Path : nullptr;
        errorInSavingRom = false;
        DrawScreen(selectedRow);
        RomSelect_PadState(&PAD1_Latch);
        if (PAD1_Latch > 0)
        {
            totalFrames = frameCount; // Reset screenSaver
            // reset horizontal scroll of highlighted row
            horzontalScrollIndex = 0;
            putText(3, selectedRow, selectedRomOrFolder, fgcolor, bgcolor);

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
                if (selectedRow > STARTROW)
                {
                    selectedRow--;
                }
                else
                {
                    if (firstVisibleRowINDEX > 0)
                    {
                        firstVisibleRowINDEX--;
                        displayRoms(romlister, firstVisibleRowINDEX);
                    }
                }
            }
            else if ((PAD1_Latch & DOWN) == DOWN && selectedRomOrFolder)
            {
                if (selectedRow < ENDROW && (index) < romlister.Count() - 1)
                {
                    selectedRow++;
                }
                else
                {
                    if (index < romlister.Count() - 1)
                    {
                        firstVisibleRowINDEX++;
                        displayRoms(romlister, firstVisibleRowINDEX);
                    }
                }
            }
            else if ((PAD1_Latch & LEFT) == LEFT && selectedRomOrFolder)
            {
                firstVisibleRowINDEX -= PAGESIZE;
                if (firstVisibleRowINDEX < 0)
                {
                    firstVisibleRowINDEX = 0;
                }
                selectedRow = STARTROW;
                displayRoms(romlister, firstVisibleRowINDEX);
            }
            else if ((PAD1_Latch & RIGHT) == RIGHT && selectedRomOrFolder)
            {
                if (firstVisibleRowINDEX + PAGESIZE < romlister.Count())
                {
                    firstVisibleRowINDEX += PAGESIZE;
                }
                selectedRow = STARTROW;
                displayRoms(romlister, firstVisibleRowINDEX);
            }
            else if ((PAD1_Latch & B) == B)
            {
                fr = f_getcwd(currentDir, 40);
                if (fr != FR_OK)
                {
                    printf("Cannot get current dir: %d\n", fr);
                }
                if (strcmp(currentDir, "/") != 0)
                {
                    romlister.list("..");
                    firstVisibleRowINDEX = 0;
                    selectedRow = STARTROW;
                    displayRoms(romlister, firstVisibleRowINDEX);
                }
            }
            else if ((PAD1_Latch & START) == START && (PAD1_Latch & SELECT) != SELECT)
            {
                // start emulator with currently loaded game
                break;
            }
            else if ((PAD1_Latch & A) == A && selectedRomOrFolder)
            {

                if (entries[index].IsDirectory)
                {
                    romlister.list(selectedRomOrFolder);
                    firstVisibleRowINDEX = 0;
                    selectedRow = STARTROW;
                    displayRoms(romlister, firstVisibleRowINDEX);
                }
                else
                {
                    // https://kevinboone.me/picoflash.html?i=1
                    // https://www.makermatrix.com/blog/read-and-write-data-with-the-pi-pico-onboard-flash/
                    printf("Start saving rom to flash memory\n");
                    exclProc_.setProcAndWait(
                        []
                        {
                            FIL fil;
                            FRESULT fr;
                            // borrow buffer from where to flash from emulator
                            size_t bufsize;
                            BYTE *buffer = (BYTE *)InfoNes_GetPPURAM(&bufsize);

                            auto ofs = FLASH_ADDRESS - XIP_BASE;
                            printf("write %s rom to flash %x\n", selectedRomOrFolder, ofs);
                            fr = f_open(&fil, selectedRomOrFolder, FA_READ);

                            UINT bytesRead;
                            if (fr == FR_OK)
                            {
                                for (;;)
                                {
                                    fr = f_read(&fil, buffer, bufsize, &bytesRead);
                                    if (fr == FR_OK)
                                    {
                                        if (bytesRead == 0)
                                        {
                                            break;
                                        }
                                        printf("Flashing %d bytes to flash address %x\n", bytesRead, ofs);
                                        printf("  -> Erasing...");

                                        // Disable interupts, erase, flash and enable interrupts
                                        uint32_t ints = save_and_disable_interrupts();
                                        flash_range_erase(ofs, bufsize);
                                        printf("\n  -> Flashing...");
                                        flash_range_program(ofs, buffer, bufsize);
                                        restore_interrupts(ints);
                                        //
                                        
                                        printf("\n");
                                        ofs += bufsize;
                                    }
                                    else
                                    {
                                        snprintf(globalErrorMessage, 40, "Error reading rom: %d", fr);
                                        printf("Error reading rom: %d\n", fr);
                                        errorInSavingRom = true;
                                        break;
                                    }
                                }
                                f_close(&fil);
                            }
                            else
                            {
                                snprintf(globalErrorMessage, 40, "Cannot open rom %d", fr);
                                printf("%s\n", globalErrorMessage);
                                errorInSavingRom = true;
                            }
                            if (!errorInSavingRom)
                            {
                                // Create file containing name currently loaded rom
                                printf("Creating %s\n", ROMINFOFILE);
                                fr = f_open(&fil, ROMINFOFILE, FA_CREATE_ALWAYS | FA_WRITE);
                                if (fr == FR_OK)
                                {
                                    for (auto i = 0; i < strlen(selectedRomOrFolder) - 4; i++)
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
                            }
                        });
                    printf("done\n");
                    if (!errorInSavingRom)
                    {
                        selectedRomOrFolder[strlen(selectedRomOrFolder) - 4] = 0;
                        break; // starts emulator
                    }
                    else
                    {
                        DisplayEmulatorErrorMessage(globalErrorMessage);
                        globalErrorMessage[0] = 0;
                    }
                }
            }
        }
        // scroll selected row horizontally if textsize exceeds rowlength
        if (selectedRomOrFolder)
        {
            if ((frameCount % 30) == 0)
            {
                if (strlen(selectedRomOrFolder + horzontalScrollIndex) > VISIBLEPATHSIZE)
                {
                    horzontalScrollIndex++;
                }
                else
                {
                    horzontalScrollIndex = 0;
                }
                putText(3, selectedRow, selectedRomOrFolder + horzontalScrollIndex, fgcolor, bgcolor);
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
            displayRoms(romlister, firstVisibleRowINDEX);
        }
    }
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
#if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0
    wiipad_end();
#endif

    // Don't return from this function call, but reboot in order to get the sound properly working
    // Starting emulator after return from menu often disables or corrupts sound
    // After reboot, the emulator starts the selected game.
    printf("Rebooting...\n");
    watchdog_enable(100, 1);
    while(1);
    // Never return
}
