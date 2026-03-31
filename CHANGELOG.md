# CHANGELOG

Mapper 5 (Castlevania III US) is now working. (RP2350 - Only)

# General Info

[Binaries for each configuration and PCB design are at the end of this page](#downloads___).

[Click here for tested configurations](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/testresults.md).

[See setup section in readme how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# v0.38

- **RP2350 only:** Added support for Mapper 5 (MMC5 – *Castlevania III* US). Minor graphical glitches may still occur.  
- **All platforms:** Added support for Mapper 24 (VRC6A – *Castlevania III/Akumajou Densetsu* JP).  
- **Adafruit Fruit Jam:** Headphone detection now works correctly. Plugging in headphones automatically mutes the internal speaker; unplugging them re-enables it.  
- Added a new **"Display Mode"** option on HSTX boards, allowing selection between HDMI and DVI. When DVI is selected, external audio (when available) is enabled by default. DVI does not have audio over HDMI.
- Enabling **External audio** mo longer forces DVI mode.

# v0.37

- Added support for DVI (Video only) mode on HSTX boards (GPIO 12–19) as an alternative to HDMI (Video + Audio). This allows the emulator to work on displays that do not support HDMI but do support DVI. [#171](https://github.com/fhoedemakers/pico-infonesPlus/issues/171). 
- SELECT + Button1: Force DVI mode (HSTX only). Useful if a DVI monitor shows no picture. This will restore the image.
- Enabling **External audio** also forces DVI mode. 
- DVI mode is the default on the Murmulator M2. Other HSTX boards default to HDMI mode.

## Fixes
- Some minor improvements to the menu and settings display.

# v0.36

For RP2350 boards using HSTX instead of PicoDVI, HDMI audio is now supported via the new HSTX video driver — this was not possible before. Huge thanks to [@fliperama86](https://github.com/fliperama86) for the awesome [pico_hdmi](https://github.com/fliperama86/pico_hdmi) driver and support.

HSTX boards with HDMI audio:
- Adafruit Fruit Jam
- Murmulator M2

Other RP2350 configurations that now use HSTX (GPIO 12–19) instead of PicoDVI:

- [Breadboard](https://github.com/fhoedemakers/pico-infonesPlus?tab=readme-ov-file#raspberry-pi-pico-or-pico-2-setup-with-adafruit-hardware-and-breadboard)
- [PCB](https://github.com/fhoedemakers/pico-infonesPlus?tab=readme-ov-file#pcb-with-raspberry-pi-pico-or-pico-2)
- [Adafruit Metro RP2350](https://github.com/fhoedemakers/pico-infonesPlus?tab=readme-ov-file#adafruit-metro-rp2350)
  
All other boards continue to use PicoDVI.

To use HDMI audio, disable External Audio in the Settings menu.
  
- Add [build-time ROM embedding](https://github.com/fhoedemakers/pico-infonesPlus?tab=readme-ov-file#building-with-an-embedded-rom): pass `-DEMBED_NES_ROM=/path/to/rom.nes` to CMake to embed a ROM into the firmware. Boots straight into the game without an SD card or menu. [@fliperama86](https://github.com/fliperama86)
- In-game BOOTSEL shortcut: SELECT + START + UP + A. [@fliperama86](https://github.com/fliperama86)
- Added option in Settings menu to enter BOOTSEL for flashing firmware.

## Fixes

- Various fixes and improvements

# v0.35 release notes

## Fixes

- The -s option (PSRAM cs pin) is removed from pico_shared/bld.sh script as it caused issues with the PSRAM settings in pico_shared/BoardConfigs.cmake. The PSRAM CS pin must be set correctly in pico_shared/BoardConfigs.cmake. (default is 47)
- Murmulator M1 and M2 fixes [#165](https://github.com/fhoedemakers/pico-infonesPlus/issues/165):
  - Second NES controller now works.
  - PS_RAM setting fixed for M2 board.
  - PS_RAM setting added for M1 board.


# previous changes

See [HISTORY.md](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/HISTORY.md)


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
| Waveshare RP2350-Zero | [piconesPlus_WaveShareRP2350ZeroWithPCB_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShareRP2350ZeroWithPCB_arm.uf2) | [Readme](README.md#pcb-with-waveshare-rp2040rp2350-zero) |


PCB: [Gerber_PicoNES_Mini_PCB_v2.0.zip](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/Gerber_PicoNES_Mini_PCB_v2.0.zip)

3D-printed case designs for PCB WS2XX0-Zero:
[https://www.thingiverse.com/thing:7041536](https://www.thingiverse.com/thing:7041536)

### PCB Waveshare RP2350-USBA with PCB
[Binary](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_WaveShare2350USBA_arm_piousb.uf2)

PCB: [Gerber_PicoNES_Micro_v1.2.zip](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/Gerber_PicoNES_Micro_v1.2.zip)

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

### SpotPear HDMI

For more info about the SpotPear HDMI see this page : https://spotpear.com/index/product/detail/id/1207.html and https://spotpear.com/index/study/detail/id/971.html

The easiest way to set this up is using an expander board like this: https://shop.pimoroni.com/products/pico-omnibus?variant=32369533321299 

See also https://github.com/fhoedemakers/pico-infonesPlus/discussions/127 

| Board | Binary |
|:--|:--|
| Pico/Pico w | [piconesPlus_SpotpearHDMI_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_SpotpearHDMI_pico_arm.uf2) |
| Pico 2/Pico 2 w | [piconesPlus_SpotpearHDMI_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_SpotpearHDMI_pico2_arm.uf2) |

### Murmulator M1

For more info about the Murmulator see this website: https://murmulator.ru/ and [#150](https://github.com/fhoedemakers/pico-infonesPlus/issues/150)

| Board | Binary |
|:--|:--|
| Pico/Pico w | [piconesPlus_MurmulatorM1_pico_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_MurmulatorM1_pico_arm.uf2) |
| Pico 2/Pico 2 w | [piconesPlus_MurmulatorM1_pico2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_MurmulatorM1_pico2_arm.uf2) |

### Murmulator M2

For more info about the Murmulator see this website: https://murmulator.ru/ and [#150](https://github.com/fhoedemakers/pico-infonesPlus/issues/150)

| Board | Binary |
|:--|:--|
| Pico/Pico w | [piconesPlus_MurmulatorM2_arm.uf2](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/piconesPlus_MurmulatorM2_arm.uf2) |

### Other downloads

- Metadata: [PicoNesMetadata.zip](https://github.com/fhoedemakers/pico-infonesPlus/releases/latest/download/PicoNesMetadata.zip)


Extract the zip file to the root folder of the SD card. Select a game in the menu and press START to show more information and box art. Works for most official released games. Screensaver shows floating random cover art.






























