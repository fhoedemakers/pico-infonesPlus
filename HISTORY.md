# History of changes

# v0.32 release notes

- Added support for Waveshare RP2350-USBA with PCB. More info and build guide at: https://www.instructables.com/PicoNES-RaspberryPi-Pico-Based-NES-Emulator/
- Added support for [Spotpear HDMI](https://spotpear.com/index/product/detail/id/1207.html) board.

## Known issues

- Pimoroni Pico DV: [#132](https://github.com/fhoedemakers/pico-infonesPlus/issues/132). Conflict with LED and I2S audio on Pico W and Pico2 W. No binaries provided for these boards. Use Pico or Pico2 binaries instead.
- PCB and breadboard [#136](https://github.com/fhoedemakers/pico-infonesPlus/issues/136). If you experience a red flashing screen during gameplay on Pico W/Pico 2W, use the Pico/Pico2 binaries instead.

# v0.31 release notes

- Adafruit Fruit Jam:
  - NeoPixel leds act as a VU meter. Can be toggled on or of via Button2 on the Fruit Jam, or SELECT + RIGHT on the controller.

- Screensaver
  - Block screensaver, which is shown when no metadata is available, is replaced by static floating image.

## Fixes

Better error handling in screensaver function and other minor fixes.

# v0.30 release notes

- Added support for [Adafruit Fruit Jam](https://www.adafruit.com/product/6200):  
  - Uses HSTX for video output.  
  - Audio is not supported over HSTX — connect speakers via the **audio jack** or the **4–8 Ω speaker connector**.  
  - Audio is simultaneousy played through speaker and jack. Speaker audio can be muted with **Button 1**.  
  - Controller options:  
    - **USB gamepad** on USB 1.  
    - **Wii Classic controller** via [Adafruit Wii Nunchuck Adapter](https://www.adafruit.com/product/4836) on the STEMMA QT port.  
  - Two-player mode:  
    - Player 1: USB gamepad (USB 1).  
    - Player 2: Wii Classic controller.  
    - Dual USB (USB 1 + USB 2) multiplayer is **not yet supported**.  
  - Scanlines can be toggled with **SELECT + UP**.  

- Added support for [Waveshare RP2350-PiZero](https://www.waveshare.com/rp2350-pizero.htm):  
  - Gamepad must be connected via the **PIO USB port**.  
  - The built-in USB port is now dedicated to **power and firmware flashing**, removing the need for a USB-Y cable.  
  - Optional: when you solder the optional PSRAM chip on the board, the emulator will make use of it. Roms will be loaded much faster using PSRAM.

- **RP2350 Only** Framebuffer implemented in SRAM. This eliminates the red flicker during slow operations, such as SD card I/O.

- **Cover art and metadata support**:  
  - Download pack [here](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/PicoNesMetadata.zip).  
  - Extract the zip contents to the **root of the SD card**.  
  - In the menu:  
    - Highlight a game and press **START** → show cover art and metadata.  
    - Press **SELECT** → show full game description.  
    - Press **B** → return to menu.  
    - Press **START** or **A** → start the game.

Huge thanks to [Gavin Knight](https://github.com/DynaMight1124) for providing the metadata and images as well as testing the different builds!

>[!NOTE]
> Cover art and metadata is available for most official released games.

- **Screensaver update**: when cover art is installed, the screensaver displays **floating random cover art** from the SD card.  
- Updated to **Pico SDK 2.2.0**  
- Updated to **lwmem V2.2.3**

## fixes

- Fixed a compiler error in pico_lib using SDK 2.2.2  [#129](https://github.com/fhoedemakers/pico-infonesPlus/issues/129)
- Moved the NES controller port 1 PIO from PIO0 to PIO1. This resolves an issue where polling the NES controller would hang in case HDMI (also driven by PIO0) uses GPIO pin numbers 32 and higher, resulting in no image.
- **RP2350 Only** Red screen flicker issue fixed. This was caused by slow operations such as SDcard I/O, which prevented the screen getting updated in time.


# v0.29 release notes 

- PSRAM will be used if detected. (RP2350 only, default pin 47). ROMs load from the SD card into PSRAM instead of flash. This speeds up loading because the board no longer has to reboot to copy the ROM from the SD card to flash. Based on https://github.com/AndrewCapon/PicoPlusPsram Boards with PSRAM are the [Adafruit Metro RP2350 with PSRAM](https://www.adafruit.com/product/6267) and [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107).
- Added -s option to bld.sh to allow an alternative GPIO pin for PSRAM chip select.
- Added support for [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107). (Uses hardware configuration 2, which is also used for breadboard and PCB). No extra binary needed.
- In some configurations, a second USB port can be added. This port can be used to connect a gamepad. The built-in usb port will be used for power and flashing the firmware. With this there is no need to use a USB-Y cable anymore. For more info, see [pio_usb.md](pio_usb.md). You have to build the firmware from source to enable this feature. The pre-built binaries do not support this.

> [!NOTE]
> Some low USB speed devices like keyboards do not work properly when connected to the second USB port. See https://github.com/sekigon-gonnoc/Pico-PIO-USB/issues/18

## Fixes
- Make PIO USB only available for RP2350, because of memory limitations on RP2040.
- Move PIO USB to Pio2, this fixes the NES controller not working on controller port 2.
- Fix save games not working when using PSRAM.

# v0.28 release notes 

- Enable I2S audio on the Pimoroni Pico DV Demo Base. This allows audio output through external speakers connected to the line-out jack of the Pimoroni Pico DV Demo Base. You can toggle audio output to this jack with SELECT + LEFT. Thanks to [Layer812](https://github.com/Layer812) for testing and providing feedback.

## Fixes
- improved error handling in build scripts.
- Github action can be started manual.

All changes are in the pico_shared submodule. When building from source, make sure you do a **git submodule update --init** from within the source folder to get the latest pico_shared module.