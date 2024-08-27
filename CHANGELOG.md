# CHANGELOG

# General Info

Binaries for each configuration are at the end of this page.

[See readme section how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# Release notes

## v0.14

### Features

For two player games. When a USB controller is connected, you can connect a NES controller in either Port 1 or Port 2.  
The USB controller is always player 1, the NES controller is player 2. 
In this situation you don't need an extra NES controller port wired for port 2 for playing two player games. 

When no USB controller is connected. You can use two NES controllers for two player games. Port 1 is player 1, Port 2 is Player 2.

### Fixes

- none

## v0.13

### Features

- Two player games can now be played. An extra NES controller port can be added to any configuration. Controller port 1 can be a USB or NES controller, controller 2 must be a NES controller. At the moment, no second USB controller can be connected.

### Fixes

- none

### Technical changes:

- Pimoroni Pico DV Demo Base: uart output fore debug printf messages is disabled, because gpio1 is needed for the second NES controller port.


## v0.12

### Features

- Some settings are now saved to SD card. This includes the selected screen mode, chosen with Select+Up or Select+Down [#42](https://github.com/fhoedemakers/pico-infonesPlus/issues/42) and the last chosen menu selection. [#46](https://github.com/fhoedemakers/pico-infonesPlus/issues/46). Settings are written to /settings.dat on the SD-card. When screen mode is changed, this will be automatically saved. The causes some red flicker due to the delay it causes.

### Fixes

- none

### Technical changes:

- Update BuildAndRelease.yml to use self-hosted runner instead of GitHub hosted runner.
- pico_lib now linked to the latest release. This fixes compiler errors in more recent versions of gcc. [https://github.com/shuichitakano/pico_lib/tree/master](https://github.com/shuichitakano/pico_lib/tree/master)


## v0.11

### Features

- Display program version in lower right corner of the menu


## v0.10

### Features

- none

### Fixes

- Fixed menu colors not displaying correctly. Using NES color palette properly now.

## v0.9

### Features

- Added support for [Waveshare RP2040-PiZero Development Board](https://www.waveshare.com/rp2040-pizero.htm)

### Fixes

- Some minor code changes.

## v0.8

### Features

- Added support for [Adafruit feather RP2040 DVI](https://www.adafruit.com/product/5710) and WII-Classic controller by @PaintYourDragon 

### Fixes

- Removed unused mapper 6 to conserve memory #22  @kholia 
- Now works with latest Pico SDK 1.5 #7 @kholia 
- Added framerate toggle #20 @fhoedemakers
- Sound not working properly when using Pico SDK 1.5 #21 by @shuichitakano
- Moved nes rom flashing from menu.cpp to main.cpp in order to prevent locking up the Feather RP2040 DVI when using the WII-Classic controller. @fhoedemakers 
