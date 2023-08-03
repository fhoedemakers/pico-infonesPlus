# CHANGELOG

# General Info

Binaries are at the end of this page.

[See readme section how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# Release notes

## v0.8

Features:

- Added support for [Adafruit feather RP2040 DVI](https://www.adafruit.com/product/5710) and WII-Classic controller by @PaintYourDragon 

Fixes:

- Removed unused mapper 6 to conserve memory #22  @kholia 
- Now works with latest Pico SDK 1.5 #7 @kholia 
- Added framerate toggle #20 @fhoedemakers
- Sound not working properly when using Pico SDK 1.5 #21 by @shuichitakano
- Moved nes rom flashing from menu.cpp to main.cpp in order to prevent locking up the Feather RP2040 DVI when using the WII-Classic controller. @fhoedemakers 
