# CHANGELOG

# General Info

[Binaries for each configuration and PCB design are at the end of this page](#downloads___).

[Click here for tested configurations](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/testresults.md).

[See setup section in readme how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

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


<a name="downloads___"></a>
## Downloads by configuration

Binaries for each configuration are listed below. Binaries for Pico(2) also work for Pico(2)-w. No blinking led however on the -w boards.
For some configurations risc-v binaries are available. It is recommended however to use the arm binaries. 

>[!NOTE]
> No dedicated binaries are provided for the Pico w or Pico 2w. Instead, use the Pico or Pico 2 binaries. Enabling the LED on these boards causes too many issues. [#136](https://github.com/fhoedemakers/pico-infonesPlus/issues/136) 

### Standalone boards

| Board | Binary | Readme | |
|:--|:--|:--|:--|
| Adafruit Metro RP2350 | [piconesPlus_AdafruitMetroRP2350_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitMetroRP2350_arm.uf2) | [Readme](README.md#adafruit-metro-rp2350) | |
| Adafruit Fruit Jam | [piconesPlus_AdafruitFruitJam_arm_piousb.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitFruitJam_arm_piousb.uf2) | [Readme](README.md#adafruit-fruit-jam)| |
| Waveshare RP2040-PiZero | [piconesPlus_WaveShareRP2040PiZero_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2040PiZero_arm.uf2) | [Readme](README.md#waveshare-rp2040rp2350-pizero-development-board)| [3-D Printed case](README.md#3d-printed-case-for-rp2040rp2350-pizero) |
| Waveshare RP2350-PiZero | [piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2) | [Readme](README.md#waveshare-rp2040rp2350-pizero-development-board)| [3-D Printed case](README.md#3d-printed-case-for-rp2040rp2350-pizero) |

### Breadboard

| Board | Binary | Readme |
|:--|:--|:--|
| Pico| [piconesPlus_AdafruitDVISD_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard) |
| Pico W | [piconesPlus_AdafruitDVISD_pico_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_w_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard) |
| Pico 2 | [piconesPlus_AdafruitDVISD_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard) |
| Pico 2 W | [piconesPlus_AdafruitDVISD_pico2_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_w_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard) |
| Adafruit feather rp2040 DVI | [piconesPlus_AdafruitFeatherDVI_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitFeatherDVI_arm.uf2) | [Readme](README.md#adafruit-feather-rp2040-with-dvi-hdmi-output-port-setup) |
| Pimoroni Pico Plus 2 | [piconesPlus_AdafruitDVISD_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard) |


### PCB Pico/Pico2

| Board | Binary | Readme |
|:--|:--|:--|
| Pico| [piconesPlus_AdafruitDVISD_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_arm.uf2) | [Readme](README.md#pcb-with-raspberry-pi-pico-or-pico-2) |
| Pico W| [piconesPlus_AdafruitDVISD_pico_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico_w_arm.uf2) | [Readme](README.md#pcb-with-raspberry-pi-pico-or-pico-2) |
| Pico 2 | [piconesPlus_AdafruitDVISD_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_arm.uf2) | [Readme](README.md#pcb-with-raspberry-pi-pico-or-pico-2) |
| Pico 2 W | [piconesPlus_AdafruitDVISD_pico2_w_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_AdafruitDVISD_pico2_w_arm.uf2) | [Readme](README.md#pcb-with-raspberry-pi-pico-or-pico-2) |

PCB [pico_nesPCB_v2.1.zip](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/pico_nesPCB_v2.1.zip)

3D-printed case designs for PCB:

[https://www.thingiverse.com/thing:6689537](https://www.thingiverse.com/thing:6689537). 
For the latest two player PCB 2.0, you need:

- Top_v2.0_with_Bootsel_Button.stl. This allows for software upgrades without removing the cover. (*)
- Base_v2.0.stl
- Power_Switch.stl.
(*) in case you don't want to access the bootsel button on the Pico, you can choose Top_v2.0.stl

### PCB WS2XX0-Zero (PCB required)

| Board | Binary | Readme |
|:--|:--|:--|
| Waveshare RP2040-Zero | [piconesPlus_WaveShareRP2040ZeroWithPCB_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2040ZeroWithPCB_arm.uf2) | [Readme](README.md#pcb-with-waveshare-rp2040rp2350-zero) |
| Waveshare RP2350-Zero | [piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2350PiZero_arm_piousb.uf2) | [Readme](README.md#pcb-with-waveshare-rp2040rp2350-zero) |

PCB: [Gerber_PicoNES_Mini_PCB_v1.0.zip](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/Gerber_PicoNES_Mini_PCB_v1.0.zip)

3D-printed case designs for PCB WS2XX0-Zero:
[https://www.thingiverse.com/thing:7041536](https://www.thingiverse.com/thing:7041536)

### PCB Waveshare RP2350-USBA
[Binary](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShare2350USBA_arm_piousb.uf2)
[Readme](README.md#pcb-with-waveshare-rp2350-usb-a)
[Build guide](https://www.instructables.com/PicoNES-RaspberryPi-Pico-Based-NES-Emulator/)

### Pimoroni Pico DV

| Board | Binary | Readme |
|:--|:--| :--|
| Pico/Pico w | [piconesPlus_PimoroniDVI_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_PimoroniDVI_pico_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-for-pimoroni-pico-dv-demo-base) |
| Pico 2/Pico 2 w | [piconesPlus_PimoroniDVI_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_PimoroniDVI_pico2_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-for-pimoroni-pico-dv-demo-base) |
| Pimoroni Pico Plus 2 | [piconesPlus_PimoroniDVI_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_PimoroniDVI_pico2_arm.uf2) | [Readme](README.md#raspberry-pi-pico-or-pico-2-setup-for-pimoroni-pico-dv-demo-base) |

> [!NOTE]
> On Pico W and Pico2 W, the CYW43 driver (used only for blinking the onboard LED) causes a DMA conflict with I2S audio on the Pimoroni Pico DV Demo Base, leading to emulator lock-ups. For now, no Pico W or Pico2 W binaries are provided; please use the Pico or Pico2 binaries instead. (#132)


### Other downloads

- Metadata: [PicoNesMetadata.zip](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/PicoNesMetadata.zip)


Extract the zip file to the root folder of the SD card. Select a game in the menu and press START to show more information and box art. Works for most official released games. Screensaver shows floating random cover art.




















