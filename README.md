# pico-infonesPlus
A NES Emulator  for the Raspberry PI Pico with SD card and menu support, specifically built for the [Pimoroni Pico DV Demo Base](https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291)

The emulator is based on the [Raspberry PI Pico port of InfoNES by Shuichi Takano](https://github.com/shuichitakano/pico-infones)

In stead of flashing a NES rom to the Pico using picotool, you can create a FAT32 formatted SD card and copy your NES roms on to it. It is possible to organise your roms into different folders.

A menu is added to the emulator, which reads the roms from the SD card and shows them on screen for the user to select and flash.

## Warning
Repeatedly flashing your Pico will eventually wear out the flash memory. Use this software at your own risk!

The emulator supports these controllers:

- Sony Dual Shock 4
- Sony Dual Sense
- BUFFALO BGC-FC801

## Wiring the Pimoroni Pico DV Demo Base
TODO

## Menu Usage

- UP/DOWN: Next/previous item in the menu
- LEFT/RIGHT: next/previous page
- A (Circle): Open folder/flash and start game.
- B (X): Back to parent folder
- START: Starts game currently loaded in flash

## Save games
For games which support it, saves will be stored in the /SAVES folder of the SD card. Caution: the save ram will only be saved back to the SD card when quitting the game via (START + SELECT)

## Raspberry Pico W support
The emulator does not work with the Pico W

## Known Issues and limitations
- Game sometimes starts with distorted sound. Workaround is to quit the game (START + SELECT) and restart the game (START button)
- The audio out jack is not supported, audio only functions via the HDMI connector
- Some TV's don't support the hdmi signal (Like my Samsung TV)
- Due to the Pico's memory limitations, not all mappers are supported.
- tar file support is removed.

## Building from source
The emulator and menu system take almost the whole ram. Therefore, the Release build is to big to fit into the Pico's ram, and cannot be used. Also the MinSizeRel (Optimise for smallest binary size) build is not suitable, because it makes the emulator too slow.

Best is to use the included build script [build.sh](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/build.sh). You can then copy the.uf2 to your Pico via the bootsel option.

When using Visual Studio code, choose the RelWithDebInfo or the Debug build variant.

