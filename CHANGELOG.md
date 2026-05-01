# CHANGELOG

Famicom Disk Sytem support (but with limitations), more mapper fixes and PAL/Dendy games now run at the correct speed on RP2350 boards. 

# General Info

[Binaries for each configuration and PCB design are at the end of this page](#downloads___).

[Click here for tested configurations](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/testresults.md).

[See setup section in readme how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# v0.39
- Can play Famicom Disk System (.fds) games, but with some limitations. See [#192](https://github.com/fhoedemakers/pico-infonesPlus/issues/192), [#193](https://github.com/fhoedemakers/pico-infonesPlus/issues/193), [#194](https://github.com/fhoedemakers/pico-infonesPlus/issues/194). You need to supply your own bios in /bios/fds-bios.rom When game asks to load swap disk, press SELECT-START to open the settings menu. Here you can swap the disks. Support is still early, so there still can be bugs. Please report an issue when you encounter a bug.
- Added "reset game" to the in-game settings menu.
- Removed the 360 folder from the metdata since it's not being used.

## Fixes
- PAL/Dendy games now run at the correct speed on RP2350 boards. Previously, they ran at 60Hz instead of 50Hz, causing them to run too fast. Due to the way the RP2040 has no framebuffer and renders directly to the display, it was not possible to implement a 50Hz mode on RP2040 boards. PAL/Dendy games on RP2040 boards will continue to run at 60Hz. Not that only PAL games are tested. Dendy games are not tested yet, but they should also run at the correct speed now. 
- Mapper 85 games now working. Tested with Tiny Toon Adventures 2 (JP), Lagrange Point (JP). Note that the expansion audio for Mapper 85 is yet emulated, so Lagrange Point will be missing music and sound effects. Tiny Toon Adventures 2 (JP) does not use the expansion audio and is playable with music and sound effects.
- Partial fix for a sound glitch in Double Dragon. [#188](https://github.com/fhoedemakers/pico-infonesPlus/issues/188)
- Fix for black screen in Akumajou Special: Boku Dracula-kun - Mapper 23 [#186](https://github.com/fhoedemakers/pico-infonesPlus/issues/186)
- Fix for Gimmick! (JP) showing only the HUD with a black playfield after pressing Start. [#187](https://github.com/fhoedemakers/pico-infonesPlus/issues/187)
- Added Sunsoft 5B expansion audio emulation for Mapper 69 (Gimmick!, Hebereke). 
- Fix for roms with incorrect header info, like Galaxian (JP)
- Fix for Robocop 3 (USA) not starting and only showing a black screen instead. (Mapper 1 fix, see [#185](https://github.com/fhoedemakers/pico-infonesPlus/issues/185)) 
- Fixed a bug where sorting large directory contents could cause a stack overflow; now uses a safer sorting method to prevent this issue. 
- Fix graphical issue in intro screen of Akumajou Densetsu (Castlevania III JP)
- Fix sound effects (e.g. whip) disappearing after a few screens in Castlevania III US (mapper 5) and Castlevania III JP (mapper 24). The APU `$4015` status register now correctly reflects actual channel/DPCM state rather than the last-written value.
- Fix for missing sound effects in Battletoads - Double Dragon [#111](https://github.com/fhoedemakers/pico-infonesPlus/issues/111)
- **Adafruit Fruit Jam:**
  - Headphone detection now works correctly. Plugging in headphones automatically mutes the speaker (Internal or monitor); unplugging them re-enables it.
  - The external audio setting, when enabled, enables the Fruit Jam built-in speaker. Audio is muted from the monitor.
- In DVI video mode, a watchdog function was added on core 1 to recover from occasional signal drops. This issue is not present in HDMI mode.


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






























