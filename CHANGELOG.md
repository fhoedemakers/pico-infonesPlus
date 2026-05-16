# CHANGELOG

Famicom Disk System games no longer require PSRAM. 8:7 pixel aspect ratio display mode and configurable scanline types added for HSTX based boards. Two player USB support.


# General Info

[Binaries for each configuration and PCB design are at the end of this page](#downloads___).

[Click here for tested configurations](https://github.com/fhoedemakers/pico-infonesPlus/blob/main/testresults.md).

[See setup section in readme how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# v0.42

## Features

**Famicom Disk System**

- PSRAM is no longer required to run Famicom Disk System games. The only requirement now is an RP2350-based board.
- BIOS screen now displays correctly.
- Added "FDS Auto Insert Disk 1 on Start" setting. When set to Off, the BIOS animation keeps playing until the user presses Button2 (A) to insert the disk.

**NSF Player**

- Fix audio clipping issues.
- Fix NSF pause/resume: preserve elapsed time and prevent false auto-advance on track change.
- Fix NSF audio delay on PicoDVI boards: send silence audio packets when ring buffer is empty to keep HDMI audio clock stream unbroken, and delay playback start on initial boot to let the monitor lock onto the HDMI signal before audio begins.

**HDMI**

- Added "Scanline Type" setting (HSTX boards only). Choose between Simple (darken odd lines) and LCD (darken every other output column for a visible pixel grid effect).
- Added 8:7 pixel aspect ratio support for HSTX boards, matching the NES original display proportions.
- Screen mode and scanline settings are now unified across DVI and HSTX paths.

**USB Controllers**

- Added two-player USB controller support. On the Adafruit Fruit Jam, two controllers can be connected directly to the board’s USB ports for multiplayer games. 

This feature is confirmed working on the Adafruit Fruit Jam, Raspberry Pi Pico, and Pico 2. For the Raspberry Pi Pico and Pico 2, a USB Y-cable and USB hub are required.
Currently, this does not yet work on the Waveshare RP2350-PiZero.

Other configurations may also work when using a USB hub, but these have not yet been tested.

**Other**

- Added support for mapper 210 [#200](https://github.com/fhoedemakers/pico-infonesPlus/issues/200)
- Binaries Built using the latest TinyUSB master commit as of today (May 16 2026): 7f146c9ff


## Fixes

- Better audio mixing for VRC6 games like Akumajou Densetsu (Castlevania III JP) [#199](https://github.com/fhoedemakers/pico-infonesPlus/issues/199)
- Fix background jitter in Akumajou Densetsu (Castlevania III JP) during vertical scroll sections. The playfield no longer shifts up and down by a pixel between frames.
- Fix HUD scroll glitches in Rush'n Attack, Galaxian (JP) and Robocop 3.
- Fix crash in settings menu because of use after free of the text screenbuffer.
- Fix for di_ring_buffer allocated twice in pico_shared/drivers/hdmi/hstx_data_island_queue.c
- Fix for mapper 19 not working correctly [#200](https://github.com/fhoedemakers/pico-infonesPlus/issues/200)
- Improved display sync and fixed audio clipping on first launch by feeding blank frames.
- Fix settings menu always showing unsaved changes due to struct padding mismatch after adding scanline type field.
- Updated CMakeLists.txt to be compatible with the latest TinyUsb branch.  [#202](https://github.com/fhoedemakers/pico-infonesPlus/issues/202) [#203](https://github.com/fhoedemakers/pico-infonesPlus/issues/203)

# v0.41 

## Features

**Famicom Disk System**

Note that FDS support requires an RP2350 board with PSRAM and a BIOS file at `/bios/fds-bios.rom`.

- Implement save games for games that support write save data back to disk, like Metroid and Zelda. Saves are stored as `/SAVES/gametitle_fds.sav` [#193](https://github.com/fhoedemakers/pico-infonesPlus/issues/193)
- Added an option to the settings menu to automatically swap disk sides. This setting is disabled by default. When it’s off, you can manually swap disks in-game using SELECT + START.

Audio is not perfect but acceptable.

**NSF sound playback**

- Added NSF playback. Emulator can load and play `.nsf` (Nintendo Sound Format) roms.
- Controls:
	- LEFT/RIGHT change track
	- Button2 Stop
	- Button1 Resume

**Settings menu**

- Better use of screen real estate:
	- SAVE / DEFAULT / CANCEL are on the same row.
	- FG/BG color codes now placed to the left to the color grid.

## Fixes

**Famicom Disk System**

- Fix disk error 24 in Metroid and possible in other games too. [#192](https://github.com/fhoedemakers/pico-infonesPlus/issues/192)
- Fix for game lock-up in Zelda when moving to the next screen during gameplay.

## Use of AI

FDS, NSF, additional mappers developed with the help of [Anthropic Claude Opus 4.6](https://www.anthropic.com/claude/opus)

# v0.40 (This is a re-release of v0.39 with some fixes and improvements)

- Fix incorrect parsing of region in NES 2.0 header. [#197](https://github.com/fhoedemakers/pico-infonesPlus/issues/197) Thanks to [@Lome-one](https://github.com/Lome-one) for reporting.
- The emulatortype is now correctly set to "NES" for Famicom Disk System games.

# v0.39

**Features:**
- Famicom Disk System (.fds) support on RP2350 boards with PSRAM (with limitations—see [#192](https://github.com/fhoedemakers/pico-infonesPlus/issues/192), [#193](https://github.com/fhoedemakers/pico-infonesPlus/issues/193), [#194](https://github.com/fhoedemakers/pico-infonesPlus/issues/194), [#195](https://github.com/fhoedemakers/pico-infonesPlus/issues/195)). Requires a BIOS file at `/bios/fds-bios.rom`. Disk swapping is done via the settings menu (SELECT+START).
- Added "reset game" option to the in-game settings menu.
- Removed unused 360 folder from metadata.

## Fixes

**Regional Support:**
- PAL/Dendy games now run at the correct frame rate on RP2350 boards (50Hz instead of 60Hz). RP2040 boards still run PAL/Dendy at 60Hz due to hardware constraints.

**Mapper & Game-Specific Fixes:**
- Mapper 85 now supported (tested with Tiny Toon Adventures 2 JP, Lagrange Point JP). Note: expansion audio for Mapper 85 is not yet emulated.
- Akumajou Special: Boku Dracula-kun (Mapper 23) - fixed black screen issue.
- Gimmick! (JP) - fixed black playfield after pressing Start.
- Robocop 3 (USA) - fixed black screen on startup (Mapper 1 fix).
- Castlevania III US (Mapper 5) and Castlevania III JP (Mapper 24) - fixed sound effects cutting out mid-level.
- Akumajou Densetsu (Castlevania III JP) - fixed graphical glitches in intro screen.
- Galaxian (JP) - fixed handling of incorrect ROM header info.
- Battletoads - Double Dragon - fixed missing sound effects.
- Double Dragon - partial fix for sound glitch.
- Added Sunsoft 5B expansion audio emulation for Mapper 69 (Gimmick!, Hebereke).

**Performance & Stability:**
- Fixed stack overflow when sorting large directory contents.
- Removed 40K fixed buffer used for Mapper 235 from heap memory.
- DVI mode: added watchdog function on core 1 to recover from occasional signal drops 

**Adafruit Fruit Jam:**
- Headphone detection now works correctly; plugging in headphones automatically mutes the speaker.
- External audio setting now enables the Fruit Jam's built-in speaker.


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






























