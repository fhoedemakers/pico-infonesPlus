# pico-infonesPlus.

## Introduction.

A NES (Nintendo Entertainment System) emulator with SD card and menu support for the Raspberry Pi Pico, Raspberry Pi Pico 2 and other RP2040/RP2350 based microcontrollers. Uses HDMI for display. 

Supports two controllers for two player games. [See "about two player games" below for specifics and limitations](#about-two-player-games) 

The emulator used is  [Infones by Jay Kumogata](https://github.com/jay-kumogata/InfoNES) which was ported to the [Raspberry Pi Pico by Shuichi Takano](https://github.com/shuichitakano/pico-infones) with changes done by me to accomodate the SD card menu.

Create a FAT32 or exFAT formatted SD card and copy your NES roms and optional [metadata](#using-metadata) on to it. It is possible to organize your roms into different folders. Then insert the SD Card into the card slot. Needless to say you must own all the roms you put on the card.

A menu is added to the emulator, which reads the roms from the SD card and shows them on screen for the user to select,  flash and play.

See below for [possible configurations](#possible-configurations), [supported game controllers](#gamecontroller-support) and how to [setup](#setup).  There is even a custom [PCB (printed circuit board)](#pcb-with-raspberry-pi-pico-or-pico-2) available and a [3D-printable case design](https://github.com/fhoedemakers/pico-infonesPlus#3d-printed-case) which fits the PCB.

[See also the Adafruit guide](https://learn.adafruit.com/nes-emulator-for-rp2040-dvi-boards).

There is also an emulator port for the Sega Master System/Sega Game Gear and DMG Game Boy. You can find them here:

- Sega Master System/Game Gear: [https://github.com/fhoedemakers/pico-smsplus](https://github.com/fhoedemakers/pico-smsplus)
- DMG Game Boy: [https://github.com/fhoedemakers/pico-peanutGB](https://github.com/fhoedemakers/pico-peanutGB)

***

## Video
Click on image below to see a demo video.

[![Video](https://img.youtube.com/vi/OEcpNMNzZCQ/0.jpg)](https://www.youtube.com/watch?v=OEcpNMNzZCQ)

***

## Possible configurations

You can use it with these RP2040/RP2350 boards and configurations:

- Raspberry Pi Pico, Pico 2, [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107). Requires one of these addons:
  - [Pimoroni Pico DV Demo Base](https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291) hdmi add-on board. For use with a USB gamecontroller or up to two a legacy NES controllers. (NES controller ports require soldering)
  - Breadboard and
    - [Adafruit DVI Breakout For HDMI Source Devices](https://www.adafruit.com/product/4984)
    - [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254).
      
    For use with a USB gamecontroller or up to two legacy NES controllers. (No soldering requirerd)
    
  - A custom printed circuit board (PCB) designed by [@johnedgarpark](https://twitter.com/johnedgarpark). (requires soldering) Up to two NES controller ports can be added to this PCB. Can also be used with a USB gamecontroller. You can 3d print your own NES-like case for the PCB. The Pimoroni Pico Plus 2 is not suitable for this PCB because of the SP/CE connector on back of the board
    
  - An additional PCB design for Waveshare RP2040 & RP2350 Zero including case design by DynaMight1124 based around cheaper but harder to solder components for those that fancy a bigger challenge. It also allows the design to be smaller.
 
- [Adafruit Feather RP2040 with DVI](https://www.adafruit.com/product/5710) (HDMI) Output Port. For use with a USB gamecontroller, up to two legacy NES controllers, or even a WII classic controller. Requires these addons:
  - Breadboard
  - SD reader  (choose one below)
    - [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254).
    - [FeatherWing - RTC + SD](https://www.adafruit.com/product/2922). (not tested by me, but should work)
   
- [Waveshare RP2040-PiZero Development Board](https://www.waveshare.com/rp2040-pizero.htm)

  For use with a USB gamecontroller, up to two legacy NES controllers, or a WII classic controller. (No soldering requirerd)

  You can 3d print your own NES-like case for for this board. This does require some soldering.

- [Waveshare RP2350-PiZero Development Board](https://www.waveshare.com/rp2350-pizero.htm) Supports the optional PSRAM chip. When installed, the emulator loads ROMs from PSRAM instead of flash memory for significantly faster performance. Fully functional even without PSRAM.

- [Adafruit Metro RP2350](https://www.adafruit.com/product/6003) Supports the optional PSRAM chip. When installed, the emulator loads ROMs from PSRAM instead of flash memory for significantly faster performance. Fully functional even without PSRAM.

- [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107)

  Use the breadboard config as mentioned above. Works also on the Pimoroni Pico DV Demo base. This board does not fit the PCB because of the SP/CE connector on back of the board.
  The PSRAM on the board is used in stead of flash to load the roms from SD.

- [Adafruit Fruit Jam](https://www.adafruit.com/product/6200)
No additional hardware is required apart from a USB gamepad. Audio is output through the included speaker, with the option to connect external speakers or a Wii Classic controller via Stemma QT. The PSRAM on the board is used in stead of flash to load the roms from SD.

[See below to see how to setup your specific configuration.](#Setup)

> [!NOTE]
> It seems that sellers on AliExpress have copied the PCB design and are selling pre-populated PCB's. I do not condone this in any way. 
> For questions about those boards, please contact the seller on AliExpress.

***

## Gamecontroller support
Depending on the hardware configuration, the emulator supports these gamecontrollers. An USB-Y cable is needed to both connect power and a gamecontroller to the usb-port.

### USB  game Controllers
- Sony Dual Shock 4
- Sony Dual Sense
- BUFFALO BGC-FC801 connected to USB - not tested
- Genesis Mini 1 and 2
- PlayStation Classic
- Keyboard
- XInput type of controllers like Xbox 360 and Xbox One controllers and other XInput compatible controllers like 8bitDo.
- Mantapad, cheap [NES](https://nl.aliexpress.com/w/wholesale-nes-controller-usb.html?spm=a2g0o.home.search.0) and [SNES](https://nl.aliexpress.com/w/wholesale-snes-controller-usb.html?spm=a2g0o.productlist.search.0) USB controllers from AliExpress. Although cheap and working i do not recommended them.

See also [troubleshooting USB controllers below](#troubleshooting-usb-controllers)

>[!NOTE]
> There is some input lag when using USB controllers.

#### Optional Second USB-Port for game controller use.
In some configurations, a second USB port is available for connecting a USB game controller. The built-in USB port remains dedicated to power and firmware flashing, so there is no need for a USB-Y cable.

This feature is enabled by default on the Adafruit Fruit Jam and Waveshare RP2350-PiZero. For other boards, you’ll need to [build the firmware from source](#building-from-source) to enable it, as the pre-built binaries do not include this option.

For more info, see [pio_usb.md](pio_usb.md).

### Legacy controllers
- One or optional two original NES controllers for two player games.  In some configurations, soldering is required.
- WII-classic controller: Adafruit Feather RP2040 and WaveShare RP2040 Pi-Zero boards only
      
Parts list for legacy controllers
  * NES Controller. A second controller port and controller is optional and only needed if you want to play two player games using NES controllers. Two player games can also be played with a USB controller and a NES controller.
    * [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
    * [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)

  * WII-Classic controller 
    *  [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
    *  Adafruit Feather RP2040: [Adafruit STEMMA QT / Qwiic JST SH 4-pin Cable](https://www.adafruit.com/product/4210)
    *  Waveshare RP2040-PiZero Development Board: [STEMMA QT / Qwiic JST SH 4-pin Cable with Premium Female Sockets](https://www.adafruit.com/product/4397)
    *  [WII Classic wired controller](https://www.amazon.com/s?k=wii-classic+controller)

***

## About two player games

The emulator supports two player games using two NES controllers or an USB gamecontroller and a NES controller.

> [!NOTE]
> You cannot use two USB controllers for two player games.
> At the moment only one USB controller is recognized by the driver. In this case the USB controller is always player 1. Player 2 must be a NES controller.


| | Player 1 | Player 2 |
| --- | -------- | -------- |
| USB controller connected | USB | NES port 1 or NES port 2 |
| No usb controller connected | NES port 1| NES port 2 |

***
## PSRAM
Some boards support additional memory called PSRAM with a capacity of 8MB On certain boards this comes pre-installed, while on others it is optional and must be soldered manually. When PSRAM is detected, the emulator will automatically make use of it.

Without PSRAM, selecting a game ROM triggers a reboot: the ROM is written to flash memory during startup to prevent the system from locking up. This process is relatively slow, taking several seconds before the game starts.

With PSRAM, this step is no longer needed. Games are loaded directly from the SD card into PSRAM and executed immediately, resulting in much faster startup times.

| Board | PSRAM Included |
|:--|:--|
| [Waveshare RP2350-PiZero](https://www.waveshare.com/rp2350-pizero.htm) | No – optional, must be soldered ([PSRAM module](https://www.adafruit.com/product/4677)) |
| [Adafruit Metro RP2350 with PSRAM](https://www.adafruit.com/product/6267) | Yes – pre-installed |
| [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2) | Yes – pre-installed |


***

## Warning
Repeatedly flashing your Pico will eventually wear out the flash memory. 

The emulator overclocks the Pico in order to get the emulator working fast enough. Overclocking can reduce the Pico's lifespan.

Use this software at your own risk! I will not be responsible in any way for any damage to your Pico and/or connected peripherals caused by using this software.

I also do not take responsability in any way when damage is caused to the Pico or display due to incorrect wiring or voltages.

***

# Setup

Click on the link below for your specific board configuration:

- [Raspberry Pi Pico or Pico 2, setup for Pimoroni Pico DV Demo Base](#raspberry-pi-pico-or-pico-2-setup-for-pimoroni-pico-dv-demo-base)
- [Raspberry Pi Pico or Pico 2, setup with Adafruit hardware and breadboard](#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard)
- [Pimoroni Pico Plus 2](#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard)
- [Adafruit Feather RP2040 with DVI (HDMI) Output Port setup](#adafruit-feather-rp2040-with-dvi-hdmi-output-port-setup)
- [Adafruit Metro RP2350](#adafruit-metro-rp2350)
- [Adafruit Fruit Jam](#adafruit-fruit-jam)
- [Waveshare RP2040-PiZero Development Board](#waveshare-rp2040rp2350-pizero-development-board)
  * [3D printed case for this board](#3d-printed-case-for-rp2040rp2350-pizero)
- [Waveshare RP2350-PiZero Development Board](#waveshare-rp2040rp2350-pizero-development-board)
  * [3D printed case for this board](#3d-printed-case-for-rp2040rp2350-pizero)
- [Printed Circuit Board (PCB) for Raspberry Pi Pico or Pico 2](#pcb-with-raspberry-pi-pico-or-pico-2)
  * [3D printed case for this PCB](#3d-printed-case-for-pcb)
- [PCB with WaveShare RP2040/RP2350 Zero](#pcb-with-waveshare-rp2040rp2350-zero)
  * [3D printed case for this PCB](#3d-printed-case)

***

##  Raspberry Pi Pico or Pico 2, setup for Pimoroni Pico DV Demo Base.

### materials needed
- Raspberry Pi Pico or Pico 2 with soldered male headers.
- [Pimoroni Pico DV Demo Base](https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291).
- [Micro usb to usb OTG Cable](https://a.co/d/dKW6WGe)
- Controllers (Depending on what you have)
  - Dual Shock 4 or Dual Sense Controller.
  - one or two NES Controllers.
    - [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz). Requires soldering.
    - [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
    - Optional: A sconde NES controller port and controller if you want to play two player games.
    - [Dupont wires](https://a.co/d/cJVmnQO)
    - [Mail or female headers to be soldered on the board](https://a.co/d/dSNPuyo)
- HDMI Cable.
- Micro usb power adapter.
- Micro usb to usb cable when using the Duak Shock 4 controller
- USB C to USB data cable when using the Sony Dual Sense controller.
- FAT32 or exFAT formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.

> [!NOTE]
> An external speaker can be connected to the audio jack of the Pimoroni Pico DV Demo Base. You can toggle audio output to this jack with SELECT + LEFT. 

### flashing the Pico
- When using a Pico / Pico W, download **[piconesPlus_PimoroniDVI_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_PimoroniDVI_pico_arm.uf2)**  from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- When using a Pico 2 or Pico 2 W or Pimoroni Pico Plus 2, download **[piconesPlus_PimoroniDVI_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_PimoroniDVI_pico2_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Push and hold the BOOTSEL button on the Pico, then connect to your computer using a micro usb cable. Release BOOTSEL once the drive RPI-RP2 appears on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The Raspberry Pi Pico will reboot and will now run the emulator.

### Pinout

#### NES controller port(s) (if you want to use legacy NES controllers).


|          | Port 1 | Port 2 (optional) |Note  |
| ------------- | ------------- | ------------- | ----------- |
| GND           |               |               | GND (- on board) |
| VCC (Power)   |               |               | Connect to 3V3  |
| NES Clock     | GPIO14        |    GPIO1      |             | 
| NES LATCH     | GPIO16        |    GPIO20     |             |
| NES Data      | GPIO15         |   GPIO21     |             |

> [!NOTE]
> Soldering is required.
> There is only one 3v3 pin header on the board, the other 3V3 must be soldered directly onto pin 36 (3V3 OUT) of the Pico.
> For GND there are two pin headers available on the board.
> Clock Data and Latch for NES controller port 1 must be soldered directly onto the Pico.
> The Clock, Data and Latch for NES controller port 2 can be soldered on the available pin headers on the board, no need to solder them directly onto the Pico.

![PinHeadersPimoroniDV](https://github.com/user-attachments/assets/4e2ee8e1-13dd-44d6-a5a0-908771872c11)


![Image](assets/nes-controller-pinout.png)

### setting up the hardware
- Disconnect the Pico from your computer.
- Attach the Pico to the DV Demo Base.
- Connect the HDMI cable to the Demo base and your monitor.
- Connect the usb OTG cable to the Pico's usb port.
- Depending which controller you want to use:
  - Connect the controller to the other end of the usb OTG.
  - Connect legacy NES controller(s) to NES controller port(s).
- Insert the SD card into the SD card slot.
- Connect the usb power adapter to the usb port of the Demo base. (USB POWER)
- Power on the monitor and the Pico

### Image: Usb controller only

![Image](assets/PicoInfoNesPlusPimoroni.jpeg)

### Image: one or two player setup with usb controller and NES controller port 2

In this image the NES controller port is wired to port 2.

For single player games: use USB controller. 

For two player games: Connect a USB controller for player 1 and a NES controller for player 2.

![Image](assets/2plpimoronidv.png)

### Image: Two player setup using two NES controllers or a USB controller and a NES controller

Controller port 1 pins must be soldered directly onto the Pico.

Controller port 2 pins can be soldered to the available headers of the Pimoroni DV. 

For two player games: 

- Connect two NES controllers or
- Connect a USB controller for player 1 and a NES controller for player 2. You can use either NES controller ports.

NOIMAGE - TODO

***

## Raspberry Pi Pico or Pico 2, setup with Adafruit hardware and breadboard

> [!NOTE]
> Instead of a Raspberry Pi Pico, you can also use a [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107)

### materials needed
- Raspberry Pi Pico or Pico 2 with soldered male headers.
- [Adafruit DVI Breakout For HDMI Source Devices](https://www.adafruit.com/product/4984)
- [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254)
- [Micro usb to OTG Y-Cable](https://a.co/d/b9t11rl)
- [Breadboard](https://www.amazon.com/s?k=breadboard&crid=1E5ZFUFWE6HNI&sprefix=breadboard%2Caps%2C167&ref=nb_sb_noss_2)
- [Breadboard jumper wires](https://a.co/d/2NoWOgK)
- Controllers (Depending on what you have)
  - one or two NES controllers.
    - [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
    - [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
    - [Dupont male to female wires](https://a.co/d/cJVmnQO)
  - Dual Shock 4 or Dual Sense Controller.
- HDMI Cable.
- Micro usb power adapter.
- Usb C to usb cable when using the Sony Dual Sense controller.
- Micro usb to usb cable when using a Dual Shock 4.
- FAT32 or exFAT formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organize your roms into different folders.



### flashing the Pico
- When using a Pico / Pico W, download **[piconesPlus_AdafruitDVISD_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_arm.uf2)** / **[piconesPlus_AdafruitDVISD_pico_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_w_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- When using a Pico 2 or Pico 2 W, download **[piconesPlus_AdafruitDVISD_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_arm.uf2)** / **[piconesPlus_AdafruitDVISD_pico2_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_w_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Push and hold the BOOTSEL button on the Pico, then connect to your computer using a micro usb cable. Release BOOTSEL once the drive RPI-RP2 appears on your computer. Or when already powered-on. Press and hold BOOTSEL, then press RUN on the board.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The Raspberry Pi Pico will reboot and will now run the emulator.

### Pinout 

See https://www.raspberrypi.com/documentation/microcontrollers/images/pico-pinout.svg for the pinout schema of the Raspberry Pi Pico.

Use the breadboard to connect all together:

- Wire Pico Pin 38 to the breadboard ground column (-)
- Wire the breadboard left ground column (-) with the breadboard right ground column (-)

#### Adafruit Micro-SD breakout board+

|  Breakout     | GPIO   | Note     |
| ------------- | ------ | -------------- |
| CS            | GPIO5    |               |
| CLK (SCK)     | GPIO2    |               |
| DI (MOSI)     | GPIO3    |               |
| DO (MISO)     | GPIO4    |               |
| 3V            |        | Pin 36 (3v3 OUT)            |
| GND           |        | Ground on breadboard (-) |

#### Adafruit DVI Breakout For HDMI Source Devices

|  Breakout     | GPIO | Note|
| ------------- | ---- | ---------- |
| D0+           | GPIO12 |         |
| D0-           | GPIO13 |         |
| CK+           | GPIO14 |         |
| CK-           | GPIO15 |         |
| D2+           | GPIO16 |         |
| D2-           | GPIO17 |         |
| D1+           | GPIO18 |         |
| D1-           | GPIO19 |         |
| 5 (*)         | VBUS | Pin 40 (5volt) |
| GND (3x)      |      | Ground on breadboard (-)     |

(*) This is the via on the side of the board marked 5. (next to via D and C). 

![Image](assets/DVIBreakout.jpg)

#### NES controller port(s). (if you want to use legacy NES controllers).
|           | Port1 | Port 2 (optional) | Note |
| ------------- | ---- | -------- |---------- |
| GND           |      | | |Ground on breadboard (-) |
| VCC (Power)   |      |   |(3v3 OUT)        |
| NES Clock     | GPIO6  | GPIO9 |          |
| NES LATCH     | GPIO8  | GPIO11 |          |
| NES Data      | GPIO7  | GPIO10 |          |

![Image](assets/nes-controller-pinout.png)

### setting up the hardware

- Disconnect the Pico from your computer.
- Attach the Pico to the breadboard.
- Insert the SD card into the SD card slot.
- Connect the HDMI cable to the Adafruit HDMI Breakout board and to your monitor.
- Connect the usb OTG Y-cable to the Pico's usb port.
- Connect the Micro usb power adapter to the female Micro usb connecter of the OTG Y-Cable.
- Controllers (Depending on what you have)
  - Connect the USB-controller to the full size female usb port of the OTG Y-Cable.
  - Connect your NES controller(s) to the NES controller port(s).
- Power on the monitor and the Pico

See image below. 

> [!NOTE]
> The Shotky Diode (VSYS - Pin 39 to breadboard + column) and the wire on breadboard left (+) to right (+) are not necessary, but recommended when powering the Pico from a Raspberry Pi.
> [See Chapter 4.6 - Powering the Board of the Raspberry Pi Pico Getting Started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) 

### Image: one or two player setup with usb controller and NES controller port

In this image the NES controller port is wired to port 1.

For single player games, connect either an USB controller **or** a NES controller. Not both!

For two player games: Connect a USB controller for player 1 and a NES controller for player 2.

![Image](assets/PicoBreadBoard.jpg)

### Image: Two player setup using two NES controllers or a USB controller and a NES controller

Choose either of the following:

- Connect two NES controllers 
- Connect a USB controller for player 1 and a NES controller for player 2. You can use either NES controller ports.

> [!NOTE]
> The device on the left is a Pico Debug probe used for debugging. This is optional

![Image](assets/2plpicobreadb.png)

***

##  Adafruit Feather RP2040 with DVI (HDMI) Output Port setup

### materials needed

- [Adafruit Feather RP2040 with DVI (HDMI) Output Port](https://www.adafruit.com/product/5710)
- SD Reader (Choose one below)
  * [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254) together with a breadboard.
  * [FeatherWing - RTC + SD](https://www.adafruit.com/product/2922) - not tested by me, but should work.
- [Breadboard](https://www.amazon.com/s?k=breadboard&crid=1E5ZFUFWE6HNI&sprefix=breadboard%2Caps%2C167&ref=nb_sb_noss_2)
- [Breadboard jumper wires](https://a.co/d/2NoWOgK)
- USB-C to USB data cable.
- HDMI Cable.
- FAT32 or exFAT formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.
- Optional: a push button like [this](https://www.kiwi-electronics.com/nl/drukknop-12mm-10-stuks-403?country=NL&utm_term=403&gclid=Cj0KCQjwho-lBhC_ARIsAMpgMoeZIyZD1ZW5GKC0r7iTBCxEP84dIZLqFfoup1D0XNOnpevkp06oyDoaAojJEALw_wcB).

When using a USB gamecontroller this is needed:
- [USB C male to micro USB female cable](https://www.amazon.com/Adapter-Connector-Charging-Compatible-Z3-Black/dp/B07Z9FLJG3/ref=sr_1_5?keywords=usb+c+male+to+micro+usb+female&qid=1688473279&sprefix=usb+c+male+to+micro+%2Caps%2C159&sr=8-5)
- [Micro usb to OTG Y-Cable](https://a.co/d/b9t11rl)
- Micro usb power adapter
- Usb C to usb cable when using the Sony Dual Sense controller.
- Micro usb to usb cable when using a Dual Shock 4.

When using legacy controllers, this is needed:
  * USB-C Power supply   
  * Depending on what you have:
    * one or two NES Controllers.
      * [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
      * [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
      * [Dupont male to female wires](https://a.co/d/cJVmnQO)
    * WII-Classic controller
      *  [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
      *  [Adafruit STEMMA QT / Qwiic JST SH 4-pin Cable](https://www.adafruit.com/product/4210)
      *  [WII Classic wired controller](https://www.amazon.com/Classic-Controller-Nintendo-Wii-Remote-Console/dp/B0BYNHWS1V/ref=sr_1_1_sspa?crid=1I66OX5L05507&keywords=Wired+WII+Classic+controller&qid=1688119981&sprefix=wired+wii+classic+controller%2Caps%2C150&sr=8-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)
  
### Pinout 
See: https://learn.adafruit.com/assets/119662 for the Feather pin scheme.

Use the breadboard to connect all together:

- Wire the 3.3V Pin to the breadboard + column.
- Wire the GND Pin to the breadboard - column
- Wire the breadboard left ground column (-) with the breadboard right ground column (-)
- Optional: Attach a push button to the breadboard and connect a wire from this button to the Feather RST pin and breadboard ground column(-). This adds an extra easy to access Reset button.

#### Adafruit Micro-SD breakout board+

|  Breakout     | GPIO   |      |
| ------------- | ------ | -------------- |
| CS            | GPIO10    |              |
| CLK (SCK)     | GPIO14    |               |
| DI (MOSI)     | GPIO15   |               |
| DO (MISO)     | GPIO8   |               |
| 5V            | USB     | pin labelled USB on feather       |
| 3V            |        | See Note below
| GND           |        | - column on breadboard connected to feather ground pin|

> [!NOTE]
> The Adafruit Micro-SD breakout board+ has also a 3V input pin which can be connected to + column on breadboard connected to feather 3.3V pin. However, this gave me frequently errors trying to mount the SD card. So use 5V in stead.

#### WII  nunchuck breakout adapter.

Connect the nunchuck breakout adapter to the Feather DVI using the STEMMA QT cable.

#### NES controller port(s). (if you want to use legacy NES controllers).

|               | Port 1 | Port 2 (optional) | Note |
| ------------- | ---- | ------ | ---------- |
| GND           |      | | - column on breadboard connected to feather ground pin |
| VCC (Power)   |      | | + column on breadboard connected to feather 3.3V pin         |
| NES Clock     | GPIO5 | GPIO26 |          |
| NES LATCH     | GPIO9 | GPIO27 |        |
| NES Data      | GPIO6 | GPIO28 |        |

![Image](assets/nes-controller-pinout.png)

### flashing the Feather RP2040
- Download **[piconesPlus_FeatherDVI_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_FeatherDVI_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Connect the feather to a USB port on your computer using the USB-C data cable.
- On the feather, push and hold the BOOTSEL button, then press RESET. Release the buttons, the drive RPI-RP2 should appear on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The Raspberry Pi Pico will reboot and will now run the emulator.

> [!NOTE]
>  When the emulator won't start after flashing or powering on, and the screen shows 'No signal,' press the reset button once again. The emulator should now boot.

### setting up the hardware

- Disconnect the Pico from your computer.
- Attach the Adafruit Feather RP2040 DVI to the breadboard.
- Insert the SD card into the SD card slot.
- Connect the HDMI cable to the HDMI port of the Adafruit Feather and to your monitor.
- Connect controllers depending on your setup:
  - Legacy controllers.
    - NES Controller to the NES controller port.
    - WII-Classic controller to the Nunchuck Breakout Adapter.
    - Connect USB-C power supply to USB-C connector.
  - USB game Controllers
    * Connect the USB C connector of the "male USB C to female micro usb cable" to the USB C port of the feather.
    * Connect the female micro USB port of the "male USB C to female micro usb cable" to the male micro USB port of the USB OTG Y cable.
    * Connect the Dual Sense or Dual Shock controller with the appropriate cable to the full size female usb port of the OTG Y-Cable.
    * Connect the Micro usb power adapter to the female Micro usb connecter of the OTG Y-Cable.
- Power on the monitor and the Pico

### Image: one or two player setup with usb controller and NES/WII_classic controller port

In this image the NES controller port is wired to port 1.

For single player games, connect either an USB controller **or** a NES/WII-classic controller. Not both!

For two player games: Connect a USB controller for player 1 and a NES or WII-Classic controller for player 2.

![Image](assets/featherDVI.jpg)

### Image: Two player setup using two NES controllers or a USB controller and a NES/WII-classic controller

Choose either of the following:

- Connect two NES controllers
- Connect a WII-Classic Controller for player 1 and a NES-Controller on port 2 for player 2
- Connect a USB controller for player 1 and a NES controller for player 2. You can use either NES controller ports. You can also use the WII-classic controller for player 2.


![Image](assets/2plfeatherdv.png)

***

## Adafruit Metro RP2350

This configuration supports USB-Controller and legacy controllers. (NES / WII-classic). 

### Materials needed

- [Adafruit Metro RP2350](https://www.adafruit.com/product/6003) or [Adafruit Metro RP2350 with PSRAM](https://www.adafruit.com/product/6267)
- [22-pin 0.5mm pitch FPC flex cable for DSI CSI or HSTX.](https://www.adafruit.com/product/6036)
- [Adafruit RP2350 22-pin FPC HSTX to DVI Adapter for HDMI Displays.](https://www.adafruit.com/product/6055) 
- USB-C power supply. 
- [USB-C to USB-C - USB-A Y cable.](https://a.co/d/9vCzu0h) when using a USB game controller. The Y-cable is needed to connect the game controller and power the board at the same time.
- [USB-C to USB-A cable](https://a.co/d/2i7rJid) for flashing the uf2 onto the board 
- FAT32 or exFAT formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.

> [!NOTE]
> Use an USB-c power supply to power the board instead of the barrel jack. Powering the board using the barrel jack can cause usb game controllers to not work properly.

> [!NOTE]
> You can use the [USB host pins](https://learn.adafruit.com/adafruit-metro-rp2350/pinouts#usb-host-pins-3193156) on the board to connect a usb game-controller instead. Soldering is required for this. You also need to build the binary from source, since it is currently not included in the latest release. For more info see [pio_usb.md](pio_usb.md)


#### Legacy Controllers

 * Depending on what you have:
    * one or two NES Controllers.
      * [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
      * [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
      * [Dupont male to female wires](https://a.co/d/cJVmnQO)
    * WII-Classic controller
      *  [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
      *  [Adafruit STEMMA QT / Qwiic JST SH 4-pin Cable](https://www.adafruit.com/product/4210)
      *  [WII Classic wired controller](https://www.amazon.com/Classic-Controller-Nintendo-Wii-Remote-Console/dp/B0BYNHWS1V/ref=sr_1_1_sspa?crid=1I66OX5L05507&keywords=Wired+WII+Classic+controller&qid=1688119981&sprefix=wired+wii+classic+controller%2Caps%2C150&sr=8-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)

#### WII  nunchuck breakout adapter.

Connect the nunchuck breakout adapter to the Metro using the STEMMA QT cable.

#### NES controller port(s). (if you want to use legacy NES controllers).

|               | Port 1 | Port 2 (optional) | Note |
| ------------- | ---- | ------ | ---------- |
| GND           |      | | Any GND pin |
| VCC (Power)   |      | | Any 3V3 pin        |
| NES Clock     | GPIO2 | GPIO5 |          |
| NES LATCH     | GPIO4 | GPIO7 |        |
| NES Data      | GPIO3 | GPIO6 |        |

![Image](assets/nes-controller-pinout.png)

### flashing the Adafruit Metro RP2350

- Download **[piconesPlus_AdafruitMetroRP2350_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitMetroRP2350_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Connect the USB-C port to a USB port on your computer using the USB-C to USB-A data cable.
- On the board, push and hold the BOOT button, then press RESET. Release the buttons, the drive RPI-RP2 should appear on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The board will reboot and will now run the emulator.
  
***

## Adafruit Fruit Jam

The new [Adafruit Fruit Jam](https://www.adafruit.com/product/6200) is supported as well.

### materials needed

- [Adafruit Fruit Jam](https://www.adafruit.com/product/6200) 
- USB gamepad
- Power to USB-C
- Optional
  * If you want to use a WII-Classic controller:
    * [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
    * [Wii Classic controller](https://www.amazon.com/s?k=Wii+Classic+controller&crid=1I66OX5L05507&sprefix=wii+classic+controller%2Caps%2C150&ref=nb_sb_noss_1)
  * External speakers


### Setup

- Connect your USB gamepad to the USB 1 port of the Fruit Jam.
- If you want to use a WII-Classic controller, connect the nunchuck breakout adapter to the Fruit Jam using the STEMMA QT cable and the Wii Classic controller to the breakout adapter.
- Connect external speakers to the audio output of the Fruit Jam.

Audio will be played through the external speakers and mini speaker simultaneously. Press Button 1 on the Fruit Jam to mute the mini speaker

Flash the firmware onto the Fruit Jam. (Connect Fruit Jam via his USB-C connector to computer, then Hold Reset and Button 1). Copy [piconesPlus_AdafruitFruitJam_arm_piousb.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitFruitJam_arm_piousb.uf2) to the RPI-RP2 drive.

Please keep the following in mind:

- There is no audio over HDMI since HSTX does not support it. Use external speakers or the mini speaker for audio output.
- Not all USB controllers from the [supported controllers](#usb--game-controllers) list are guaranteed to work.
- Two player mode is only possible with one USB gamepad on USB1 and one WII-Classic controller. USB2 is not supported for two player mode yet, will be looking into it. 
- When an USB controller is connected, the WII-classic controller is player 2. To use the WII-Classic as player 1, unplug the USB controller.

<img width="800" height="1204" alt="image" src="https://github.com/user-attachments/assets/b72ee649-e166-47c2-a29d-ad0090b9f262" />


***

## Waveshare RP2040/RP2350-PiZero Development Board

### materials needed

- One of these Waveshare boards:
  - [Waveshare RP2040-PiZero Development Board](https://www.waveshare.com/rp2040-pizero.htm).
  - [Waveshare RP2350-PiZero Development Board](https://www.waveshare.com/rp2350-pizero.htm).
    - Optional: [PSRAM chip](https://www.adafruit.com/product/4677) When installed, the emulator loads ROMs from PSRAM instead of flash memory for significantly faster performance. Fully functional even without PSRAM
- [USB-C to USB-A cable](https://a.co/d/2i7rJid) for flashing the uf2 onto the board.
- USB-C Power supply. Connect to the port labelled USB, not PIO-USB. See note below.
- [Mini HDMI to HDMI Cable](https://a.co/d/5BZg3Z6).
- FAT32 or exFAT formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.

Additional for the RP2040-Pizero only:

- [USB-C to USB-C - USB-A Y cable](https://a.co/d/eteMZLt). when using an USB controller. Not needed for the Waveshare RP2350-PiZero where the controller **must** be connected to the PIO-USB port.

> [!NOTE]
> Unlike the WaveShare RP2350-PiZero, where the controller must be connected to the PIO-USB port, the WaveShare RP2040-PiZero Development Board cannot use the PIO-USB port for the controller due to memory limitations.  Instead, connect both the controller and the power adapter to the Y-cable, and then plug the Y-cable into the board’s port labeled “USB.” While the PIO-USB port can still be used to power the RP2040 board, I do not recommend this, as it has occasionally caused unstable behavior.

#### NES controller port.

When using a original NES controller you need:

- [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
- [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
- [Dupont female to female wires](https://a.co/d/cJVmnQO)

For two player games with two NES controllers you need an extra NES controller port, controller and wire


|           | Port 1 | Port 2 (Optional) | Note |
| ------------- | ---- | ----- | ---------- |
| GND           |      | | Any ground pin |
| VCC (Power)   |      | | 5Volt pin         |
| NES Clock     | GPIO5 | GPIO10 |          |
| NES LATCH     | GPIO9 | GPIO11|        |
| NES Data      | GPIO6 | GPIO12|        |

![Image](assets/nes-controller-pinout.png)

> [!NOTE]
> Contrary to other configurations where VCC is connected to 3Volt, VCC should be connected to a 5Volt pin. Otherwise the NES controller could possibly not work.

#### WII-Classic controller.

When using a WII-Classic controller you need:

-  [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
-  [STEMMA QT / Qwiic JST SH 4-pin Cable with Premium Female Sockets](https://www.adafruit.com/product/4397) 
-  [WII Classic wired controller](https://www.amazon.com/s?k=wii-classic+controller)

Connections are as follows:

| Nunchuck Breakout Adapter | RP2040-PiZero |
| ---------------------- | ------------ |
| 3.3V                   | 3V3          |
| GND                    | GND          |
| SDA                    | GPIO2        |
| SCL                    | GPIO3        |

### flashing the Waveshare RP2040-PiZero Development Board
- Download **[piconesPlus_WaveShareRP2040PiZero_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2040PiZero_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Connect the USB-C port marked USB (not PIO-USB) to a USB port on your computer using the USB-C to USB-A data cable.
- On the board, push and hold the BOOT button, then press RUN. Release the buttons, the drive RPI-RP2 should appear on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The board will reboot and will now run the emulator.


### flashing the Waveshare RP2350-PiZero Development Board
- Download **[piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2) | [Readme](README.md#waveshare-rp2040rp2350-pizero-development-board)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Connect the USB-C port marked USB (not PIO-USB) to a USB port on your computer using the USB-C to USB-A data cable.
- On the board, push and hold the BOOT button, then press RUN. Release the buttons, the drive RPI-RP2 should appear on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The board will reboot and will now run the emulator.

> [!NOTE]
>  When the emulator won't start after flashing or powering on, and the screen shows 'No signal,' press the run button once again. The emulator should now boot.

### Image: one or two player setup with usb controller and NES controller port

In this image the NES controller port is wired to port 1.

For single player games, connect either an USB controller **or** a NES controller. Not both!

For two player games: Connect a USB controller for player 1 and a NES controller for player 2.

![Image](assets/WaveShareRP2040_1.jpg)

![Image](assets/WaveShareRP2040_2.jpg)

### Image: Two player setup using two NES controllers or a USB controller and a NES controller

Choose either of the following:

- Connect two NES controllers 
- Connect a USB controller for player 1 and a NES controller for player 2. You can use either NES controller ports.

![Image](assets/2plwsrp2040.png)

### Image: Using a wii-classic controller

![WS-Wiiclassic](https://github.com/user-attachments/assets/d5a89389-6b19-42df-9071-f315b4bb1ee5)

![WS-Wiiclassic2](https://github.com/user-attachments/assets/4b4ba997-6e5a-4004-83e9-dfc71da89d03)



### 3D printed case for RP2040/RP2350-PiZero

Gavin Knight ([DynaMight1124](https://github.com/DynaMight1124)) designed a NES-like case you can 3d-print as an enclosure for this board.  This enclosure is designed for 2 NES controller ports so you can play 1 or 2-player games. [Click here for the design](https://www.thingiverse.com/thing:6758682). Please contact the creator on his Thingiverse page if you have any questions about this case.

![WS3D1](https://github.com/user-attachments/assets/12e48dfa-4338-4f10-922c-66a016605210)

![WS3D2](https://github.com/user-attachments/assets/2c9dde77-59f1-45e7-8d06-d580d97174d7)


***

## PCB with Raspberry Pi Pico or Pico 2

Create your own Pico-based NES console. It features two NES controller ports for 1 or 2-player games.

Designed by [@johnedgarpark](https://twitter.com/johnedgarpark)

![IMG_6011](https://github.com/user-attachments/assets/a55dd2ad-75a8-4115-a38f-fe0ecd6f51c7)

Several Companies  can make these PCBs for you. 

I personally recommend [PCBWay](https://www.pcbway.com/). The boards i ordered from them are of excellent quality. They have also a very short lead time. Boards i ordered on Monday arrived from China to my home in the Netherlands on Friday of the same week.

[![Image](assets/pcbw.png)](https://www.pcbway.com/)

When ordering, simply upload the zip file containing the gerber design.  This file (pico_nesPCB_v2.1.zip) is available in the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest) and can also be found in the [PCB](PCB/) folder. 

> [!NOTE]
>  Soldering skills are required. Make sure you solder all the connections from the Pico onto the PCB. Also the connections on the short right-side of the Pico. (For ground)

> [!NOTE]
> If you are looking for the previous design (v0.2). You can find it [here](PCB/v0.2)

> [!NOTE]
> It seems that sellers on AliExpress have copied the PCB design and are selling pre-populated PCB's. I do not condone this in any way. For questions about those boards, please contact the seller on AliExpress.

Other materials needed:

- Raspberry Pi Pico or Pico 2 **with no headers**.
- [Adafruit DVI Breakout Board - For HDMI Source Devices](https://www.adafruit.com/product/4984)
- [Adafruit Micro SD SPI or SDIO Card Breakout Board - 3V ONLY!](https://www.adafruit.com/product/4682)
- For the NES Controllers:
  * [1 or 2 NES controller port(s)](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
  * [1 or 2 NES controller(s)](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
- [Micro usb to OTG Y-Cable](https://a.co/d/b9t11rl) if you want to use a Dualshock/Dualsense controller.
- Micro USB power supply.
- Optional: on/off switch, like [this](https://www.kiwi-electronics.com/en/spdt-slide-switch-410?search=KW-2467) 

When using a Pico / Pico W, Flash **[piconesPlus_AdafruitDVISD_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_arm.uf2)** / **[piconesPlus_AdafruitDVISD_pico_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_w_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest). 
When using a Pico 2 or Pico 2 W, flash **[piconesPlus_AdafruitDVISD_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_arm.uf2)** / **[piconesPlus_AdafruitDVISD_pico2_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_w_arm.uf2)** instead.

> [!IMPORTANT]
> You cannot use a [Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107) because of the SP/CE connector on the back of the board.

### Image: Two player setup using two NES controllers or a USB controller and a NES controller

Choose either of the following:

- Connect two NES controllers 
- Connect a USB controller for player 1 and a NES controller for player 2. You can use either NES controller ports. Use the OTG Y-Cable to connect an USB power supply and the USB controller.

![image0](https://github.com/user-attachments/assets/d40ed98f-4632-4161-986a-732d35290fac)

### 3D printed case for PCB

Gavin Knight ([DynaMight1124](https://github.com/DynaMight1124)) designed a NES-like case you can 3d-print as an enclosure for this pcb.  You can find it here: [https://www.thingiverse.com/thing:6689537](https://www.thingiverse.com/thing:6689537). Here you can find two designs: the latest design for PCB v2.0  and the previous design for [PCB v0.2](PCB/v0.2). In the latest v2.0 design, you can choose between two top covers, one with a button connecting to the bootsel button for easy firmware upgrades, the other without the button. In this case you have to remove the top cover to access the bootsel button. See images below. Make sure to print the correct files for the PCB version you own. You can find more information on Gavin's Thingiverse page.

#### Top Cover v2.0 without button (Top_v2.0.stl)
![Top cover without button](https://github.com/user-attachments/assets/c6205db3-580e-41e9-83e4-66c9534c6519)

#### Top Cover v2.0 with bootsel button (Top_v2.0_with_Bootsel_Button.stl)
![Top Cover with button to access bootsel](https://github.com/user-attachments/assets/3c8f8990-51b9-4873-9054-64bb2cd6c300)

#### Base v2.0 (Base_v2.0.stl) 
![3d2playerBottom](https://github.com/user-attachments/assets/256bbd1b-b6db-485d-a59c-fd22fd017887)

#### on/off button (Power_Switch.stl)

![powerswitch](https://github.com/user-attachments/assets/edba3bdd-7061-4370-880d-d4cfd7def0e2)

***

## PCB with WaveShare RP2040/RP2350 Zero

Create your own Pico-based NES console. It features two NES controller ports for 1 or 2-player games. This version is smaller than the above and uses cheaper, but ultimately harder to solder components. This is a more advanced project than the above PCB design, if you are unsure of your soldering capabilities I wouldnt recommend this PCB.

Several companies can make these PCBs for you. PCBWay or JLCPCB are two good options.

I personally recommend [PCBWay](https://www.pcbway.com/). The boards I ordered from them are of excellent quality. They have also a very short lead time. Boards I ordered on Monday arrived from China to my home in the Netherlands on Friday of the same week.

[![Image](assets/pcbw.png)](https://www.pcbway.com/)

When ordering, simply upload the zip file containing the gerber design.  This file (Gerber_PicoNES_Mini_PCB_v1.0.zip) is available in the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest) and can also be found in the [PCB](PCB/) folder. 

> [!NOTE]
>  Soldering skills are required. Make sure you solder all the connections from the Pico onto the PCB. This version requires good soldering skills especially for the HDMI portion, a good amount of flux and a fine tip will be required, additional solder can be wicked away with solder wick. I recommend starting with the resistor arrays first, then the HDMI port, after that either Pico or MicroSD adaptor, lastly the NES Ports, which can be hard to push into the PCB.

Other materials needed:

- RP2040 Zero: https://www.aliexpress.com/item/1005007650325892.html
- or
- RP2350 Zero: https://www.aliexpress.com/item/1005008123701162.html
- MicroSD Adaptor: https://www.aliexpress.com/item/1005005591145849.html
- NES Ports: https://www.aliexpress.com/item/1005003699734963.html
- 270ohm Resistor Arrays: https://www.aliexpress.com/item/1005005580079760.html
- HDMI Ports: https://www.aliexpress.com/item/1005001412266648.html
- 12mm M3 Countersunk Screws: https://www.aliexpress.com/item/1005006951618861.html
- NES Controllers: https://www.aliexpress.com/item/1005006635972534.html (original controllers preferred but I have found these aftermarket ones to be suitable for the price)
- USB-C power supply.


When using a RP2040 Zero, Flash **[piconesPlus_WaveShareRP2040ZeroWithPCB_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2040ZeroWithPCB_arm.uf2)** from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest). 
When using a RP2350 Zero, flash **[piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2)** instead.


### 3D printed case for PCB

Gavin Knight ([DynaMight1124](https://github.com/DynaMight1124)) designed a NES-like case you can 3D print as an enclosure for this PCB.  You can find it here: https://www.thingiverse.com/thing:7041536. If you dont own a 3D printer, you can either find a local company that can offer 3D print services or use professional services such as PCBWay or JCLPCB, the professional services can offer extremely high quaility finishes.

#### 3D printed case
![PXL_20250508_183050163](https://github.com/user-attachments/assets/732384bd-062d-43ca-97cb-a16a39607c41)

#### 3D printed case from professional services (JLCPCB in this example)
![PXL_20250527_092756092](https://github.com/user-attachments/assets/773417b4-30dd-4a22-ae40-ca46291167de)
![PXL_20250527_092840066](https://github.com/user-attachments/assets/9cb9f503-7734-43ae-902d-9ccc6a3201d7)

#### Soldered PCB
![PXL_20250508_182416020](https://github.com/user-attachments/assets/13933b1d-af00-402e-a0a0-8456de4a82da)


***

# Using metadata.

Download the metadata pack from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/PicoNesMetadata.zip) and extract its contents to the root of the SD card. It contains box art and game info for many games. The metadata is used in the menu to show box art and game info when a rom is selected. Press START to view the information. When the screensaver is started, random box art is shown.

<img width="1920" height="1080" alt="Screenshot 2025-08-25 15-43-24" src="https://github.com/user-attachments/assets/7aa98825-e3b1-4c7a-ba13-80e04929a27d" />


# Gamepad and keyboard usage

## Menu Usage
Gamepad buttons:
- UP/DOWN: Next/previous item in the menu.
- LEFT/RIGHT: next/previous page.
- A (Circle): Open folder/flash and start game.
- B (X): Back to parent folder.
- START: Show [metadata](#using-metadata) and box art (when available)

The colors in the menu can be changed and saved:
  - Select + Up/Down changes the foreground color.
  - Select + Left/Right changes the background color.
  - Select + A saves the colors. Screen will flicker when saved.
  - Select + B resets the colors to default. (Black on white)

When using an USB-Keyboard:
- Cursor keys: Up, Down, left, right
- Z: Back to parent folder
- X: Open Folder/flash and start a game
- S: Show [metadata](#using-metadata) and box art (when available).
- A: acts as the select button.

## Emulator (in game)
Gamepad buttons:
- SELECT + START, Xbox button: Resets back to the SD Card menu. Game saves are saved to the SD card.
- SELECT + UP/SELECT + DOWN: switches screen modes.
- SELECT + A/B: toggle rapid-fire.
- START + A : Toggle framerate display
- **Pimoroni Pico DV Demo Base only**: SELECT + LEFT: Switch audio output to the connected speakers on the line-out jack of the Pimoroni Pico DV Demo Base. The speaker setting will be remembered when the emulator is restarted.
- **Fruit Jam Only** 
  - Button 1 (on board): Mute audio of built-in speaker. Audio is still outputted to the audio jack.
  - SELECT + UP: Toggle scanlines. 
  - Button 2 (on board) or SELECT + RIGHT: Toggles the VU meter on or off. (NeoPixel LEDs light up in sync with the music rhythm)
- **Genesis Mini Controller**: When using a Genesis Mini controller, press C for SELECT.
- **USB-keyboard**: When using an USB-Keyboard
  - Cursor keys: up, down, left, right
  - A: Select
  - S: Start
  - Z: B
  - X: A

>[!NOTE]
> The standard NES style layout is used for all controller types. So when using XInput controllers, B and A are swapped.

***

# Save games
For games which support it, saves will be stored in the /SAVES folder of the SD card. Caution: the save ram will only be saved back to the SD card when quitting the game via (START + SELECT)

***

# Raspberry Pico W and Pico2 W support
The emulator works with the Pico W (RP2040). Use the pico_w_ or pico2_w_ versions of the uf2 files in the latest release. The Pico W has a built-in wifi module. The wifi module is not used by the emulator. It is only used for enabling the led to blink every 60 frames on the Pico W.  If you don't mind the led blinking, you can use the pico_ versions of the uf2 files on the Pico W.

***

# USB game Controllers latency
Using a USB gamecontroller introduces some latency. The legacy controllers ((S)NES, WII-classic) have less latency.

***

# Troubleshooting usb controllers

## AliExpress Controllers (Mantapad)

When starting a game, and the controller is unresponsive, you have to unplug and replug the controller to get it working. Not all controllers behave this way. I have a SNES controller that has no problems. The NES controller however must alwas be replugged to make it work. It is kind of a hit and miss.

> [!NOTE]
> When using a SNES style usb controller, press Y to properly setup the controller. Otherwise the B button will not work. You have to do this every time you start a game or boot into the menu.


## XInput style controllers.

Might not work with all controllers.

Tested devices:
- Xbox 360 : Works
- Xbox Series X controller : Works
- Xbox One controller : Works
- Xbox elite controller : Works
- 8bitdo SN30 Pro+ firmware V6.01: Works. With the controller switched off, hold X + Start to switch to XInput mode. (LED 1 and 2 will blink). Then connect to USB.
- 8bitdo Pro 2 firmware V3.04: Works. With the controller switched off, hold X + Start to switch to XInput mode. (LED 1 and 2 will blink). Then connect to USB.
- 8bitdo SN30 PRO Wired : Not working, recognized but no report
- 8bitdo SF30 PRO firmware v2.05 : Works. With the controller switched off, hold X + Start to switch to XInput mode. (LED 1 and 2 will blink). Then connect to USB.
- 8bitdo SN30 PRO firmware v2.05 : Not tested, should probably work

### Troubleshooting:

After flashing some bigger games, the controller might become unresponsive:
- Xbox Controller. Playing with batteries removed is recommended. When controller becomes unresponsive:
  - unplug and replug the controller.
  - If controller is still unresponsive, unplug the pico from power, wait a few seconds then plug it back in and press start to start the last flashed game.

- 8bitdo controllers, when controller becomes unresponsive:
  - Disconnect the controller.
  - Hold start to switch the controller off (if it has built-in battery).
  - reconnect the controller.

***

# Troubleshooting no image on TV or monitor

Some displays need 5V connected to the HDMI breakout in order to work:
- When using the breadboard with HDMI and SD breakout, make sure VBUS (Pin 40) is connected to the 5 volt via  on the HDMI breakout board. (Marked 5 on the side) 

![Image](assets/DVIBreakout.jpg)

***

# Known Issues and limitations

- Due to the Pico's memory limitations, not all games will work. Games not working will show a "Mapper n is unsupported." (n is a number). For example starting Castlevania III will show the "Mapper 5 is unsupported." message.
- tar file support is removed.

***

# Building from source

Best is to use the included build script [buildAll.sh](buildAll.sh). You can then copy the correct .uf2 to your Pico via the bootsel option. The script builds all the .uf2 files and puts them in the releases folder.

```bash
git clone https://github.com/fhoedemakers/pico-infonesPlus.git
cd pico-infonesPlus
git submodule update --init
chmod +x build*.sh
./buildAll.sh
```

Alternatively, you can use the [bld.sh](bld.sh) shell script:

```
Build script for the piconesPlus project

Usage: ./pico_shared/bld.sh [-d] [-2 | -r] [-w] [-u] [-m] [-t path to toolchain] [ -p nprocessors] [-c <hwconfig>]
Options:
  -d: build in DEBUG configuration
  -2: build for Pico 2 board (RP2350)
  -r: build for Pico 2 board (RP2350) with riscv core
  -u: enable PIO USB support (default is disabled, RP2350 only)
  -s <ps-ram-cs>: specify the GPIO pin for PSRAM chip select (default is 47 for RP2350 boards)
  -w: build for Pico_w or Pico2_w
  -t <path to riscv toolchain>: only needed for riscv, specify the path to the riscv toolchain bin folder
     Default is $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2/bin
  -p <nprocessors>: specify the number of processors to use for the build
  -c <hwconfig>: specify the hardware configuration
     1: Pimoroni Pico DV Demo Base (Default)
     2: Breadboard with Adafruit AdaFruit DVI Breakout Board and AdaFruit MicroSD card breakout board
        Custom pcb
     3: Adafruit Feather RP2040 DVI
     4: Waveshare RP2040-PiZero
     5: Adafruit Metro RP2350 (latest branch of TinyUSB is required for this board)
     6: Waveshare RP2040-Zero/RP2350-Zero with custom PCB
  -m: Run cmake only, do not build the project
  -h: display this help

To install the RISC-V toolchain:
 - Raspberry Pi: https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.0.0-5/riscv-toolchain-14-aarch64-lin.tar.gz
 - X86/64 Linux: https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.0.0-5/riscv-toolchain-14-x86_64-lin.tar.gz
and extract it to $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2

Example riscv toolchain install for Raspberry Pi OS:

        cd
        sudo apt-get install wget
        wget https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.0.0-5/riscv-toolchain-14-aarch64-lin.tar.gz
        mkdir -p $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2
        tar -xzvf riscv-toolchain-14-aarch64-lin.tar.gz -C $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2

To build for riscv:

        ./bld.sh -c <hwconfig> -r -t $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2/bin

```

When using Visual Studio code, choose the Release or the RelWithDebuginfo build variant.

## Building with support for an additional USB port using PIO-USB

In some configurations, a second USB port can be added. This port can be used to connect a gamepad. The built-in usb port will be used for power and flashing the firmware.
With this there is no need to use a USB-Y cable anymore.

For more info on how to setup and build the firmware, see [pio_usb.md](pio_usb.md).

> [!NOTE]
> You have to build the firmware from source to enable this feature. The pre-built binaries do not support this.

***

# Credits
InfoNes is programmed by [Jay Kumogata](https://github.com/jay-kumogata/InfoNES) and ported to the Raspberry Pi Pico by [Shuichi Takano](https://github.com/shuichitakano/pico-infones).

I contributed by programming functionality for SD card, menu, 2-player games, support for various USB gamepads and keyboard, metdata rendering etc...

PCB design by [John Edgar Park](https://twitter.com/johnedgarpark).

Additional PCB design and 3D-printable case for both PCB's and WaveShare RP2040/RP2350-PiZero by [Gavin Knight](https://github.com/DynaMight1124)

Metadata files provided by [Gavin Knight](https://github.com/DynaMight1124), based on [Ducalex's retro-go-covers](https://github.com/ducalex/retro-go-covers)

NES gamepad support contributed by [PaintYourDragon](https://github.com/PaintYourDragon) & [Adafruit](https://github.com/adafruit). 

WII-Classic controller support by [PaintYourDragon](https://github.com/PaintYourDragon) & [Adafruit](https://github.com/adafruit).

Adafruit Feather DVI - RP2040 support by [PaintYourDragon](https://github.com/PaintYourDragon) & [Adafruit](https://github.com/adafruit).

XInput driver: https://github.com/Ryzee119/tusb_XInput by [Ryzee119](https://github.com/Ryzee119)

FatFS driver: https://github.com/elehobica/pico_fatfs by [elehobica](https://github.com/elehobica)

PSRAM: https://github.com/AndrewCapon/PicoPlusPsram

lwmem: https://github.com/MaJerle/lwmem

***

# Other versions
[There is also a version available for the Pimoroni PicoSystem handheld](https://github.com/fhoedemakers/PicoSystem_InfoNes). 

![Image](https://github.com/fhoedemakers/PicoSystem_InfoNes/blob/master/assets/gamescreen.jpeg)
