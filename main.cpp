#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/divider.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "util/work_meter.h"
#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_pAPU.h"
#include "ff.h"
#include "tusb.h"
#include "gamepad.h"
#include "rom_selector.h"
#include "menu.h"
#include "nespad.h"
#include "wiipad.h"
#include "FrensHelpers.h"
#include "settings.h"
#include "FrensFonts.h"
#include "zapper.h"
bool isFatalError = false;

char *romName;

static bool fps_enabled = false;
static uint32_t start_tick_us = 0;
static uint32_t fps = 0;

constexpr uint32_t CPUFreqKHz = 252000;

namespace
{
    ROMSelector romSelector_;
}   

#define CC(x) (((x >> 1) & 15) | (((x >> 6) & 15) << 4) | (((x >> 11) & 15) << 8))
const WORD __not_in_flash_func(NesPalette)[64] = {
    CC(0x39ce), CC(0x1071), CC(0x0015), CC(0x2013), CC(0x440e), CC(0x5402), CC(0x5000), CC(0x3c20),
    CC(0x20a0), CC(0x0100), CC(0x0140), CC(0x00e2), CC(0x0ceb), CC(0x0000), CC(0x0000), CC(0x0000),
    CC(0x5ef7), CC(0x01dd), CC(0x10fd), CC(0x401e), CC(0x5c17), CC(0x700b), CC(0x6ca0), CC(0x6521),
    CC(0x45c0), CC(0x0240), CC(0x02a0), CC(0x0247), CC(0x0211), CC(0x0000), CC(0x0000), CC(0x0000),
    CC(0x7fff), CC(0x1eff), CC(0x2e5f), CC(0x223f), CC(0x79ff), CC(0x7dd6), CC(0x7dcc), CC(0x7e67),
    CC(0x7ae7), CC(0x4342), CC(0x2769), CC(0x2ff3), CC(0x03bb), CC(0x0000), CC(0x0000), CC(0x0000),
    CC(0x7fff), CC(0x579f), CC(0x635f), CC(0x6b3f), CC(0x7f1f), CC(0x7f1b), CC(0x7ef6), CC(0x7f75),
    CC(0x7f94), CC(0x73f4), CC(0x57d7), CC(0x5bf9), CC(0x4ffe), CC(0x0000), CC(0x0000), CC(0x0000)};


uint32_t getCurrentNVRAMAddr()
{
    if (!romSelector_.getCurrentROM())
    {
        return {};
    }
    int slot = romSelector_.getCurrentNVRAMSlot();
    if (slot < 0)
    {
        return {};
    }
    printf("SRAM slot %d\n", slot);
    return ROM_FILE_ADDR - SRAM_SIZE * (slot + 1);
}



void saveNVRAM()
{
    char pad[FF_MAX_LFN];
    char fileName[FF_MAX_LFN];
    strcpy(fileName, Frens::GetfileNameFromFullPath(romName));
    Frens::stripextensionfromfilename(fileName);
    if (!SRAMwritten)
    {
        printf("SRAM not updated.\n");
        return;
    }
    snprintf(pad, FF_MAX_LFN, "%s/%s.SAV", GAMESAVEDIR, fileName);
    printf("Save SRAM to %s\n", pad);
    FIL fil;
    FRESULT fr;
    fr = f_open(&fil, pad, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK)
    {
        snprintf(ErrorMessage, ERRORMESSAGESIZE, "Cannot open save file: %d", fr);
        printf("%s\n", ErrorMessage);
        return;
    }
    size_t bytesWritten;
    fr = f_write(&fil, SRAM, SRAM_SIZE, &bytesWritten);
    if (bytesWritten < SRAM_SIZE)
    {
        snprintf(ErrorMessage, ERRORMESSAGESIZE, "Error writing save: %d %d/%d written", fr, bytesWritten, SRAM_SIZE);
        printf("%s\n", ErrorMessage);
    }
    f_close(&fil);
    printf("done\n");
    SRAMwritten = false;
}

bool loadNVRAM()
{
    char pad[FF_MAX_LFN];
    FILINFO fno;
    bool ok = false;
    char fileName[FF_MAX_LFN];
    strcpy(fileName, Frens::GetfileNameFromFullPath(romName));
    Frens::stripextensionfromfilename(fileName);

    snprintf(pad, FF_MAX_LFN, "%s/%s.SAV", GAMESAVEDIR, fileName);

    FIL fil;
    FRESULT fr;

    size_t bytesRead;
    if (auto addr = getCurrentNVRAMAddr())
    {
        printf("Load SRAM from %s\n", pad);
        fr = f_stat(pad, &fno);
        if (fr == FR_NO_FILE)
        {
            printf("Save file not found, load SRAM from flash %x\n", addr);
            memcpy(SRAM, reinterpret_cast<void *>(addr), SRAM_SIZE);
            ok = true;
        }
        else
        {
            if (fr == FR_OK)
            {
                printf("Loading save file %s\n", pad);
                fr = f_open(&fil, pad, FA_READ);
                if (fr == FR_OK)
                {
                    fr = f_read(&fil, SRAM, SRAM_SIZE, &bytesRead);
                    if (fr == FR_OK)
                    {
                        printf("Savefile read from disk\n");
                        ok = true;
                    }
                    else
                    {
                        snprintf(ErrorMessage, ERRORMESSAGESIZE, "Cannot read save file: %d %d/%d read", fr, bytesRead, SRAM_SIZE);
                        printf("%s\n", ErrorMessage);
                    }
                }
                else
                {
                    snprintf(ErrorMessage, ERRORMESSAGESIZE, "Cannot open save file: %d", fr);
                    printf("%s\n", ErrorMessage);
                }
                f_close(&fil);
            }
            else
            {
                snprintf(ErrorMessage, ERRORMESSAGESIZE, "f_stat() failed on save file: %d", fr);
                printf("%s\n", ErrorMessage);
            }
        }
    }
    else
    {
        ok = true;
    }
    SRAMwritten = false;
    return ok;
}



static DWORD prevButtons[2]{};
static int rapidFireMask[2]{};
static int rapidFireCounter = 0;
void InfoNES_PadState(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem)
{
    static constexpr int LEFT = 1 << 6;
    static constexpr int RIGHT = 1 << 7;
    static constexpr int UP = 1 << 4;
    static constexpr int DOWN = 1 << 5;
    static constexpr int SELECT = 1 << 2;
    static constexpr int START = 1 << 3;
    static constexpr int A = 1 << 0;
    static constexpr int B = 1 << 1;

    // moved variables outside function body because prevButtons gets initialized to 0 everytime the function is called.
    // This is strange because a static variable inside a function is only initialsed once and retains it's value
    // throughout different function calls.
    // Am i missing something?
    // static DWORD prevButtons[2]{};
    // static int rapidFireMask[2]{};
    // static int rapidFireCounter = 0;

    ++rapidFireCounter;
    bool reset = false;
    bool usbConnected = false;
    for (int i = 0; i < 2; ++i)
    {
        auto &dst = i == 0 ? *pdwPad1 : *pdwPad2;
        auto &gp = io::getCurrentGamePadState(i);
        if ( i == 0 )
        {
            usbConnected = gp.isConnected();
        }
        int v = (gp.buttons & io::GamePadState::Button::LEFT ? LEFT : 0) |
                (gp.buttons & io::GamePadState::Button::RIGHT ? RIGHT : 0) |
                (gp.buttons & io::GamePadState::Button::UP ? UP : 0) |
                (gp.buttons & io::GamePadState::Button::DOWN ? DOWN : 0) |
                (gp.buttons & io::GamePadState::Button::A ? A : 0) |
                (gp.buttons & io::GamePadState::Button::B ? B : 0) |
                (gp.buttons & io::GamePadState::Button::SELECT ? SELECT : 0) |
                (gp.buttons & io::GamePadState::Button::START ? START : 0) |
                0;
#if NES_PIN_CLK != -1
        // When USB controller is connected both NES ports act as controller 2
        if (usbConnected)
        {          
            if (i == 1)
            {
                v = v | nespad_states[1] | nespad_states[0];
            }
        }
        else
        {
            v |= nespad_states[i];
        }
#endif

// When USB controller is connected  wiipad acts as controller 2 
#if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0
        if (usbConnected)
        {
            if (i == 1)
            {
                v |= wiipad_read();
            }
        }
        else // if no USB controller is connected, wiipad acts as controller 1
        {
            if (i == 0)
            {
                v |= wiipad_read();
            }
        }
#endif
        // read zapper input, only controller 2
        if ( i == 1 )
        {
            // Zapper
            v |= readzapper();
        }
        int rv = v;
        if (rapidFireCounter & 2)
        {
            // 15 fire/sec
            rv &= ~rapidFireMask[i];
        }

        dst = rv;

        auto p1 = v;

        auto pushed = v & ~prevButtons[i];

        // Toggle frame rate
        if (p1 & START)
        {
            if (pushed & A)
            {
                fps_enabled = !fps_enabled;
            }
        }
        if (p1 & SELECT)
        {
            // if (pushed & LEFT)
            // {
            //     saveNVRAM();
            //     romSelector_.prev();
            //     reset = true;
            // }
            // if (pushed & RIGHT)
            // {
            //     saveNVRAM();
            //     romSelector_.next();
            //     reset = true;
            // }
            if (pushed & START)
            {
                saveNVRAM();
                reset = true;
            }
            if (pushed & A)
            {
                rapidFireMask[i] ^= io::GamePadState::Button::A;
            }
            if (pushed & B)
            {
                rapidFireMask[i] ^= io::GamePadState::Button::B;
            }
            if (pushed & UP)
            {
                scaleMode8_7_ = Frens::screenMode(-1);
            }
            else if (pushed & DOWN)
            {
                scaleMode8_7_ = Frens::screenMode(+1);
            }
        }

        prevButtons[i] = v;
    }

    *pdwSystem = reset ? PAD_SYS_QUIT : 0;
}

void InfoNES_MessageBox(const char *pszMsg, ...)
{
    printf("[MSG]");
    va_list args;
    va_start(args, pszMsg);
    vprintf(pszMsg, args);
    va_end(args);
    printf("\n");
}

void InfoNES_Error(const char *pszMsg, ...)
{
    printf("[Error]");
    va_list args;
    va_start(args, pszMsg);
    vsnprintf(ErrorMessage, ERRORMESSAGESIZE, pszMsg, args);
    printf("%s", ErrorMessage);
    va_end(args);
    printf("\n");
}
bool parseROM(const uint8_t *nesFile)
{
    memcpy(&NesHeader, nesFile, sizeof(NesHeader));
    if (!checkNESMagic(NesHeader.byID))
    {
        return false;
    }

    nesFile += sizeof(NesHeader);

    memset(SRAM, 0, SRAM_SIZE);

    if (NesHeader.byInfo1 & 4)
    {
        memcpy(&SRAM[0x1000], nesFile, 512);
        nesFile += 512;
    }

    auto romSize = NesHeader.byRomSize * 0x4000;
    ROM = (BYTE *)nesFile;
    nesFile += romSize;

    if (NesHeader.byVRomSize > 0)
    {
        auto vromSize = NesHeader.byVRomSize * 0x2000;
        VROM = (BYTE *)nesFile;
        nesFile += vromSize;
    }

    return true;
}

void InfoNES_ReleaseRom()
{
    ROM = nullptr;
    VROM = nullptr;
}

void InfoNES_SoundInit()
{
}

int InfoNES_SoundOpen(int samples_per_sync, int sample_rate)
{
    return 0;
}

void InfoNES_SoundClose()
{
}

int __not_in_flash_func(InfoNES_GetSoundBufferSize)()
{
    return dvi_->getAudioRingBuffer().getFullWritableSize();
}

void __not_in_flash_func(InfoNES_SoundOutput)(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5)
{
    while (samples)
    {
        auto &ring = dvi_->getAudioRingBuffer();
        auto n = std::min<int>(samples, ring.getWritableSize());
        if (!n)
        {
            return;
        }
        auto p = ring.getWritePointer();

        int ct = n;
        while (ct--)
        {
            int w1 = *wave1++;
            int w2 = *wave2++;
            int w3 = *wave3++;
            int w4 = *wave4++;
            int w5 = *wave5++;
            //            w3 = w2 = w4 = w5 = 0;
            int l = w1 * 6 + w2 * 3 + w3 * 5 + w4 * 3 * 17 + w5 * 2 * 32;
            int r = w1 * 3 + w2 * 6 + w3 * 5 + w4 * 3 * 17 + w5 * 2 * 32;
            *p++ = {static_cast<short>(l), static_cast<short>(r)};

            // pulse_out = 0.00752 * (pulse1 + pulse2)
            // tnd_out = 0.00851 * triangle + 0.00494 * noise + 0.00335 * dmc

            // 0.00851/0.00752 = 1.131648936170213
            // 0.00494/0.00752 = 0.6569148936170213
            // 0.00335/0.00752 = 0.4454787234042554

            // 0.00752/0.00851 = 0.8836662749706228
            // 0.00494/0.00851 = 0.5804935370152762
            // 0.00335/0.00851 = 0.3936545240893067
        }

        ring.advanceWritePointer(n);
        samples -= n;
    }
}

extern WORD PC;


int InfoNES_LoadFrame()
{
#if NES_PIN_CLK != -1
    nespad_read_start();
#endif
    auto count = dvi_->getFrameCounter();
    auto onOff = hw_divider_s32_quotient_inlined(count, 60) & 1;
    Frens::blinkLed(onOff);
#if NES_PIN_CLK != -1
    nespad_read_finish(); // Sets global nespad_state var
#endif
    tuh_task();
    // Frame rate calculation
    if (fps_enabled)
    {
        // calculate fps and round to nearest value (instead of truncating/floor)
        uint32_t tick_us = Frens::time_us() - start_tick_us;
        fps = (1000000 - 1) / tick_us + 1;
        start_tick_us = Frens::time_us();
    }
    return count;
}

namespace
{
    dvi::DVI::LineBuffer *currentLineBuffer_{};
}

void __not_in_flash_func(drawWorkMeterUnit)(int timing,
                                            [[maybe_unused]] int span,
                                            uint32_t tag)
{
    if (timing >= 0 && timing < 640)
    {
        auto p = currentLineBuffer_->data();
        p[timing] = tag; // tag = color
    }
}

void __not_in_flash_func(drawWorkMeter)(int line)
{
    if (!currentLineBuffer_)
    {
        return;
    }

    memset(currentLineBuffer_->data(), 0, 64);
    memset(&currentLineBuffer_->data()[320 - 32], 0, 64);
    (*currentLineBuffer_)[160] = 0;
    if (line == 4)
    {
        for (int i = 1; i < 10; ++i)
        {
            (*currentLineBuffer_)[16 * i] = 31;
        }
    }

    constexpr uint32_t clocksPerLine = 800 * 10;
    constexpr uint32_t meterScale = 160 * 65536 / (clocksPerLine * 2);
    util::WorkMeterEnum(meterScale, 1, drawWorkMeterUnit);
    //    util::WorkMeterEnum(160, clocksPerLine * 2, drawWorkMeterUnit);
}

void __not_in_flash_func(InfoNES_PreDrawLine)(int line)
{
    util::WorkMeterMark(0xaaaa);
    auto b = dvi_->getLineBuffer();
    util::WorkMeterMark(0x5555);
    // b.size --> 640
    // printf("Pre Draw%d\n", b->size());
    // WORD = 2 bytes
    // b->size = 640
    // printf("%d\n", b->size());
    InfoNES_SetLineBuffer(b->data() + 32, b->size());
    //    (*b)[319] = line + dvi_->getFrameCounter();

    currentLineBuffer_ = b;
}

void __not_in_flash_func(InfoNES_PostDrawLine)(int line)
{
#if !defined(NDEBUG)
    util::WorkMeterMark(0xffff);
    drawWorkMeter(line);
#endif
    // Display frame rate
    if (fps_enabled && line >= 8 && line < 16)
    {
        char fpsString[2];
        WORD *fpsBuffer = currentLineBuffer_->data() + 40;
        WORD fgc = NesPalette[48];
        WORD bgc = NesPalette[15];
        fpsString[0] = '0' + (fps / 10);
        fpsString[1] = '0' + (fps % 10);

        int rowInChar = line % 8;
        for (auto i = 0; i < 2; i++)
        {
            char firstFpsDigit = fpsString[i];
            char fontSlice = getcharslicefrom8x8font(firstFpsDigit, rowInChar);
            for (auto bit = 0; bit < 8; bit++)
            {
                if (fontSlice & 1)
                {
                    *fpsBuffer++ = fgc;
                }
                else
                {
                    *fpsBuffer++ = bgc;
                }
                fontSlice >>= 1;
            }
        }
    }
    assert(currentLineBuffer_);
    dvi_->setLineBuffer(line, currentLineBuffer_);
    currentLineBuffer_ = nullptr;
}

bool loadAndReset()
{
    auto rom = romSelector_.getCurrentROM();
    if (!rom)
    {
        printf("ROM does not exists.\n");
        return false;
    }

    if (!parseROM(rom))
    {
        printf("NES file parse error.\n");
        return false;
    }
    if (loadNVRAM() == false)
    {
        return false;
    }

    if (InfoNES_Reset() < 0)
    {
        printf("NES reset error.\n");
        return false;
    }

    return true;
}

int InfoNES_Menu()
{
    // InfoNES_Main() のループで最初に呼ばれる
    return loadAndReset() ? 0 : -1;
    // return 0;
}

int main()
{
    char selectedRom[FF_MAX_LFN];
    romName = selectedRom;
    ErrorMessage[0] = selectedRom[0] = 0;

    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10);
    set_sys_clock_khz(CPUFreqKHz, true);

    stdio_init_all();
    printf("Start program\n");
    printf("CPU freq: %d\n", clock_get_hz(clk_sys));
    printf("Starting Tinyusb subsystem\n");
    tusb_init();
#if NES_MAPPER_5_ENABLED == 1
    printf("Mapper 5 is enabled\n");
#else
    printf("Mapper 5 is disabled\n");
#endif
    isFatalError =  !Frens::initAll(selectedRom, CPUFreqKHz, 4, 4 );
    scaleMode8_7_ = Frens::applyScreenMode(settings.screenMode);
    initzapper();
    bool showSplash = true;
    while (true)
    {
        if (strlen(selectedRom) == 0)
        {
            menu("Pico-InfoNES+", ErrorMessage, isFatalError, showSplash, ".nes"); // never returns, but reboots upon selecting a game
        }
        printf("Now playing: %s\n", selectedRom);
        romSelector_.init(ROM_FILE_ADDR);
        InfoNES_Main();
        selectedRom[0] = 0;
        showSplash = false;
    }

    return 0;
}
