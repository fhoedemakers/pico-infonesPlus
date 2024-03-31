# CHANGELOG

# General Info

Binaries are at the end of this page.

[See readme section how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# Release notes

## v0.10

Features:

- none

Fixes:

- Fixed menu colors not displaying correctly. Using NES color palette properly now.

## v0.9

Features:

- Added support for [Waveshare RP2040-PiZero Development Board](https://www.waveshare.com/rp2040-pizero.htm)

Fixes:

- Some minor code changes.

## v0.8

Features:

- Added support for [Adafruit feather RP2040 DVI](https://www.adafruit.com/product/5710) and WII-Classic controller by @PaintYourDragon 

Fixes:

- Removed unused mapper 6 to conserve memory #22  @kholia 
- Now works with latest Pico SDK 1.5 #7 @kholia 
- Added framerate toggle #20 @fhoedemakers
- Sound not working properly when using Pico SDK 1.5 #21 by @shuichitakano
- Moved nes rom flashing from menu.cpp to main.cpp in order to prevent locking up the Feather RP2040 DVI when using the WII-Classic controller. @fhoedemakers 
