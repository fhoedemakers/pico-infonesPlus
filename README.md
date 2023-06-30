# pico-infonesPlus
A NES (Nintendo Entertainment System) Emulator for the [Raspberry PI Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) and [Adafruit feather RP2040 DVI](https://www.adafruit.com/product/5710) with SD card and menu support. You can use it with:

- Raspberry PI Pico (works with USB and legacy controllers)
  * [Pimoroni Pico DV Demo Base](https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291) hdmi add-on board. (USB controllers only)
  * [Adafruit DVI Breakout For HDMI Source Devices](https://www.adafruit.com/product/4984) and  [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254) together with a breadboard and optional a NES controller
  * A custom printed circuit board designed by [@johnedgarpark](https://twitter.com/johnedgarpark). A NES or SNES controller port can be soldered on this PCB.

- Adafruit Feather RP2040 with DVI (HDMI) Output Port together with a breadboard. (Works only with legacy controllers)
  - SD reader  (choose one below)
    * [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254).
    * [FeatherWing - RTC + SD](https://www.adafruit.com/product/2922). (not tested by me, but should work)
  - Legacy Controler ports (choose one or both):
    * NES Controller
      * [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
      * [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
    * WII-Classic controller
      *  [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
      *  [Adafruit STEMMA QT / Qwiic JST SH 4-pin Cable](https://www.adafruit.com/product/4210)
      *  [WII Classic wired controller](https://www.amazon.com/Classic-Controller-Nintendo-Wii-Remote-Console/dp/B0BYNHWS1V/ref=sr_1_1_sspa?crid=1I66OX5L05507&keywords=Wired+WII+Classic+controller&qid=1688119981&sprefix=wired+wii+classic+controller%2Caps%2C150&sr=8-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)

The emulator used is  [Infones by Jay Kumogata](https://github.com/jay-kumogata/InfoNES) which was ported to the [Raspberry PI Pico by Shuichi Takano](https://github.com/shuichitakano/pico-infones) with changes done by me to accomodate the SD card menu.

In stead of flashing a NES rom to the Pico using picotool, you create a FAT32 formatted SD card and copy your NES roms on to it. It is possible to organise your roms into different folders. Then insert the SD Card into the card slot of the Pimoroni Pico DV Demo base. Needless to say you must own all the roms you put on the card.

A menu is added to the emulator, which reads the roms from the SD card and shows them on screen for the user to select,  flash and play.

## Game Controller support
The emulator supports these controllers:

- Raspberry Pi Pico
  - USB controllers
    - Sony Dual Shock 4
    - Sony Dual Sense
    - BUFFALO BGC-FC801 connected to USB - not tested
  - Legacy Controllers
    - An original NES controller.  Does not apply with Piomoroni Pico DV Demo Base.
    - An original SNES controller. PCB Only
    - WII-classic controller. Breadboard only. Not tested - should work
- Adafruit Feather RP2040 with DVI (HDMI)
  - USB controllers
    - None
  - Legacy Controllers
    - An original NES controller.
    - WII-classic controller.
 
## Video
Click on image below to see a demo video.

[![Video](https://img.youtube.com/vi/OEcpNMNzZCQ/0.jpg)](https://www.youtube.com/watch?v=OEcpNMNzZCQ)

## Other versions
[There is also a version available for the Pimoroni PicoSystem handheld](https://github.com/fhoedemakers/PicoSystem_InfoNes). 

![Image](https://github.com/fhoedemakers/PicoSystem_InfoNes/blob/master/assets/gamescreen.jpeg)

## Warning
Repeatedly flashing your Pico will eventually wear out the flash memory. 

The emulator overclocks the Pico in order to get the emulator working fast enough. Overclocking can reduce the Pico's lifespan.

Use this software at your own risk! I will not be responsible in any way for any damage to your Pico and/or connected peripherals caused by using this software.

I also do not take responsability in any way when damage is caused to the Pico or display due to incorrect wiring or voltages.



##  Setup - Pimoroni Pico DV Demo Base.

### materials needed
- Raspberry Pi Pico with soldered male headers.
- [Pimoroni Pico DV Demo Base](https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291).
- [Micro usb to usb OTG Cable](https://a.co/d/dKW6WGe)
- Dual Shock 4 or Dual Sense Controller.
- HDMI Cable.
- Micro usb power adapter.
- Micro usb to usb cable when using the Duak Shock 4 controller
- usb C to usb cable when using the Sony Dual Sense controller.
- FAT 32 formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.


### flashing the Pico
- Download piconesPlusPimoroniDV.uf2 from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Push and hold the BOOTSEL button on the Pico, then connect to your computer using a micro usb cable. Release BOOTSEL once the drive RPI-RP2 appears on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The Raspberry Pi Pico will reboot and will now run the emulator.

### setting up the hardware
- Disconnect the Pico from your computer.
- Attach the Pico to the DV Demo Base.
- Connect the HDMI cable to the Demo base and your monitor.
- Connect the usb OTG cable to the Pico's usb port.
- Connect the controller to the other end of the usb OTG.
- Insert the SD card into the SD card slot.
- Connect the usb power adapter to the usb port of the Demo base.
- Power on the monitor and the Pico

![Image](assets/PicoInfoNesPlusPimoroni.jpeg)


## setup - using Raspberry PI Pico with Adafruit hardware and breadboard

### materials needed
- Raspberry Pi Pico with soldered male headers.
- [Adafruit DVI Breakout For HDMI Source Devices](https://www.adafruit.com/product/4984)
- [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254)
- [Micro usb to OTG Y-Cable](https://a.co/d/b9t11rl)
- [Breadboard](https://www.amazon.com/s?k=breadboard&crid=1E5ZFUFWE6HNI&sprefix=breadboard%2Caps%2C167&ref=nb_sb_noss_2)
- Optional: [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
- Dual Shock 4 or Dual Sense Controller.
- HDMI Cable.
- Micro usb power adapter.
- Usb C to usb cable when using the Sony Dual Sense controller.
- Micro usb to usb cable when using a Dual Shock 4.
- FAT 32 formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.



### flashing the Pico
- Download piconesPlusAdaFruitDVISD.uf2 from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).
- Push and hold the BOOTSEL button on the Pico, then connect to your computer using a micro usb cable. Release BOOTSEL once the drive RPI-RP2 appears on your computer.
- Drag and drop the UF2 file on to the RPI-RP2 drive. The Raspberry Pi Pico will reboot and will now run the emulator.

### Pinout 

Use the breadboard to connect all together:

- Wire Pico Pin 38 to the breadboard ground column (-)
- Wire the breadboard left ground column (-) with the breadboard right ground column (-)

#### Adafruit Micro-SD breakout board+

|  Breakout     | GPIO   | Pin number     |
| ------------- | ------ | -------------- |
| CS            | GP5    | 7              |
| CLK (SCK)     | GP2    | 4              |
| DI (MOSI)     | GP3    | 5              |
| DO (MISO)     | GP4    | 6              |
| 3V            |        | 36 (3v3 OUT)            |
| GND           |        | Ground on breadboard (-) |

#### Adafruit DVI Breakout For HDMI Source Devices

|  Breakout     | GPIO | Pin number |
| ------------- | ---- | ---------- |
| D0+           | GP12 | 16         |
| D0-           | GP13 | 17         |
| CK+           | GP14 | 19         |
| CK-           | GP15 | 20         |
| D2+           | GP16 | 21         |
| D2-           | GP17 | 22         |
| D1+           | GP18 | 24         |
| D1-           | GP19 | 25         |
| 5 (*)         | VBUS | 40 (5volt) |
| GND (3x)      |      | Ground on breadboard (-)     |

(*) This is the via on the side of the board marked 5. (next to via D and C). 

![Image](assets/DVIBreakout.jpg)

#### Optional NES controller port.
|  Port         | GPIO | Pin number |
| ------------- | ---- | ---------- |
| NES Clock     | GP6  | 9          |
| NES Data      | GP7  | 10         |
| NES LATCH     | GP8  | 11         |
| VCC (Power)   |      | 36 (3v3 OUT)        |
| GND           |      | Ground on breadboard (-) |

![Image](assets/nes-controller-pinout.png)

### setting up the hardware

- Disconnect the Pico from your computer.
- Attach the Pico to the breadboard.
- Insert the SD card into the SD card slot.
- Connect the HDMI cable to the Adafruit HDMI Breakout board and to your monitor.
- Connect the usb OTG Y-cable to the Pico's usb port.
- Connect the controller to the full size female usb port of the OTG Y-Cable
- Connect the Micro usb power adapter to the female Micro usb connecter of the OTG Y-Cable.
- Power on the monitor and the Pico

See image below. 
Note. The Shotky Diode (VSYS - Pin 39 to breadboard + column) and the wire on breadboard left (+) to right (+) are not necessary, but recommended when powering the Pico from a Raspberry Pi. 
[See Chapter 4.6 - Powering the Board of the Raspberry Pi Pico Getting Started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) 

![Image](assets/PicoInfoNesPlusAdafruit.jpeg)

##  Setup -  Adafruit Feather RP2040 with DVI (HDMI) Output Port
NOTE: USB gamecontrollers do not work in this configuration.
### materials needed
- [Adafruit Feather RP2040 with DVI (HDMI) Output Port](https://www.adafruit.com/product/5710)
- SD Reader (Choose one below)
  * [Adafruit Micro-SD breakout board+](https://www.adafruit.com/product/254) together with a breadboard.
  * [FeatherWing - RTC + SD](https://www.adafruit.com/product/2922) togheter with a breadboard. (not tested by me, but should work)
- [Breadboard](https://www.amazon.com/s?k=breadboard&crid=1E5ZFUFWE6HNI&sprefix=breadboard%2Caps%2C167&ref=nb_sb_noss_2)
- Game Controller ports (You can use both, or one of them, depending what controller you have lying around)
  * NES Controller
    * [NES controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz)
    * [An original NES controller](https://www.amazon.com/s?k=NES+controller&crid=1CX7W9NQQDF8H&sprefix=nes+controller%2Caps%2C174&ref=nb_sb_noss_1)
  * WII-Classic controller
    *  [Adafruit Wii Nunchuck Breakout Adapter - Qwiic / STEMMA QT](https://www.adafruit.com/product/4836)
    *  [Adafruit STEMMA QT / Qwiic JST SH 4-pin Cable](https://www.adafruit.com/product/4210)
    *  [WII Classic wired controller](https://www.amazon.com/Classic-Controller-Nintendo-Wii-Remote-Console/dp/B0BYNHWS1V/ref=sr_1_1_sspa?crid=1I66OX5L05507&keywords=Wired+WII+Classic+controller&qid=1688119981&sprefix=wired+wii+classic+controller%2Caps%2C150&sr=8-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)
- HDMI Cable.
- USB-C Power adapter
- FAT 32 formatted Micro SD card with roms you legally own. Roms must have the .nes extension. You can organise your roms into different folders.


## PCB

Create your own little Pico Based NES console and play with an orginal (S)NES controller. Soldering skills are required.
The PCB design files can be found in the [assets/pcb](https://github.com/fhoedemakers/pico-infonesPlus/tree/main/assets/pcb) folder. Several Companies  can make these PCBs for you. 

I personally recommend [PCBWay](https://www.pcbway.com/). The boards i ordered from them are of excellent quality.

[![Image](assets/pcbw.png)](https://www.pcbway.com/)

Simply upload the design files packed as a [zip archive](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest) when ordering. A zip file containing the design files can be found on the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).

Other materials needed:

- Raspberry PI Pico with no headers.
- on/off switch, like [this](https://www.kiwi-electronics.com/en/spdt-slide-switch-410?search=KW-2467) 
- [Adafruit DVI Breakout Board - For HDMI Source Devices](https://www.adafruit.com/product/4984)
- [Adafruit Micro SD SPI or SDIO Card Breakout Board - 3V ONLY!](https://www.adafruit.com/product/4682)
- Optional a [NES Controller port](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz) if you want to play using an original NES controller.
- Optional a [SNES controller port](https://www.zedlabz.com/products/zedlabz-7-pin-90-degree-female-controller-connector-port-for-nintendo-snes-console-2-pack-grey). if you want to play using an original SNES controller. 
- Optional [Micro usb to OTG Y-Cable](https://a.co/d/b9t11rl) for connecting a Dualshock/Dualsense controller.
- Micro USB power supply.

Flash the Pico with piconesPlusAdaFruitDVISD.uf2 from the [releases page](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest).

![Image](assets/picones.jpg)

## Menu Usage
Gamepad buttons:
- UP/DOWN: Next/previous item in the menu.
- LEFT/RIGHT: next/previous page.
- A (Circle): Open folder/flash and start game.
- B (X): Back to parent folder.
- START: Starts game currently loaded in flash.

## Emulator 
Gamepad buttons:
- SELECT + START: Resets back to the SD Card menu. Game saves are saved to the SD card.
- SELECT + UP/SELECT + DOWN: switches screen modes.
- SELECT + A/B: toggle rapid-fire.

## Save games
For games which support it, saves will be stored in the /SAVES folder of the SD card. Caution: the save ram will only be saved back to the SD card when quitting the game via (START + SELECT)

## Raspberry Pico W support
The emulator works with the Pico W, but without the onboard blinking led. In order for the led to work on the Pico W, the cyw43 driver needs to be initialised. This causes the emulator to stop with an out of memory panic. 

## Troubleshooting no image on TV or monitor

- Make sure the board is directly connected to your display. Do not connect through a HDMI splitter.
- When using the Adafruit HDMI board, make sure VBUS (Pin 40) is connected to the 5 volt via  on the board. (Marked 5 on the side) Some displays need 5V in order to work. 

![Image](assets/DVIBreakout.jpg)

## Known Issues and limitations
- Pimoroni Pico DV: Audio through the audio out jack is not supported, audio only works over hdmi.
- Due to the Pico's memory limitations, not all games will work. Games not working will show a "Mapper n is unsupported." (n is a number). For example starting Castlevania III will show the "Mapper 5 is unsupported." message.
- tar file support is removed.
- Pico W: The onboard led does not blink every 60 frames.

## Building from source
IMPORTANT The emulator does not run when built with the latest version (1.5.0) of the Pico SDK. Use 1.4.0 in stead. See issue [#7](https://github.com/fhoedemakers/pico-infonesPlus/issues/7)

The emulator and menu system take almost the whole ram. Therefore, the Release build is too big to fit into the Pico's ram, and cannot be used. Also the MinSizeRel (Optimise for smallest binary size) build is not suitable, because it makes the emulator too slow.

Best is to use the included build script [buildAll.sh](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/buildAll.sh). You can then copy the.uf2 to your Pico via the bootsel option. The script builds two .uf2 files (Pimoroni and Adafruit) and puts them in the assets folder.

When using Visual Studio code, choose the RelWithDebInfo or the Debug build variant.

## Credits
InfoNes is programmed by [Jay Kumogata](https://github.com/jay-kumogata/InfoNES) and ported to the Raspberry PI Pico by [Shuichi Takano](https://github.com/shuichitakano/pico-infones).

I contributed by adding SD card and menu support. For this reasons I made code changes to the emulator for accommodating the menu and SD card.

PCB design by [@johnedgarpark](https://twitter.com/johnedgarpark).

NES gamepad support contributed by [PaintYourDragon](https://github.com/PaintYourDragon) & [Adafruit](https://github.com/adafruit). If using Pimoroni Pico DV Demo Base: NES controller clock, data and latch go to GPIO pins 14, 15 and 16, respectively. If Adafruit DVI Breakout build, it's GPIO pins 6, 7, 8 instead. Gamepad should be powered from 3.3V when connected to Pico GPIO, not 5V as usual...seems to work OK regardless.
Controller ports which can be wired up to the Pimoroni DV or breadboard can be found [here](https://www.zedlabz.com/products/controller-connector-port-for-nintendo-nes-console-7-pin-90-degree-replacement-2-pack-black-zedlabz?_pos=4&_sid=4581182f3&_ss=r)

![Image](assets/nes-controller-pinout.png)
