# CHANGELOG

# General Info

Binaries for each configuration and PCB design are at the end of this page.

- For Raspberry Pi Pico (RP2040) you need to download the .uf2 files starting with pico_piconesPlus.
- For Raspberry Pi Pico w (rp2040) you can download the .uf2 files starting with pico_w_piconesPlus. Although you can also use the pico_piconesPlus binaries on the Pico w if you don't mind the blinking led.
- For Raspberry Pi Pico 2 (RP2350) you need to download the .uf2 files starting with pico2_piconesPlus for ARM or pico2_riscv_piconesPlus  for Risc-V. 
- For Raspberry Pi Pico 2 w (RP2350) you can download the .uf2 files starting with pico2_w_piconesPlus for ARM or pico2_w_riscv_piconesPlus for Risc-V Although you can also use the pico2_piconesPlus binaries on the Pico w if you don't mind the blinking led.

[Click here for tested configurations](testresults.md).

[See setup section in readme how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

# 3D-printed case designs for custom PCBs

## Raspberry Pi Pico and Pico 2 PCB

[https://www.thingiverse.com/thing:6689537](https://www.thingiverse.com/thing:6689537). 
For the latest two player PCB 2.0, you need:

- Top_v2.0_with_Bootsel_Button.stl. This allows for software upgrades without removing the cover. (*)
- Base_v2.0.stl
- Power_Switch.stl.

(*) in case you don't want to access the bootsel button on the Pico, you can choose Top_v2.0.stl

## Waveshare RP2040-Zero and RP2350-Zero PCB

[https://www.thingiverse.com/thing:7041536](https://www.thingiverse.com/thing:7041536)

## Waveshare RP2040-PiZero

[https://www.thingiverse.com/thing:6758682](https://www.thingiverse.com/thing:6758682)

# v0.27 release notes 

- Added second PCB design for use with Waveshare [RP2040-Zero](https://www.waveshare.com/rp2040-zero.htm) or [RP2350-Zero](https://www.waveshare.com/rp2350-zero.htm) mini development board. The PCB is designed to fit in a 3D-printed case. PCB and Case design by [@DynaMight1124](https://github.com/DynaMight1124).
Based around cheaper but harder to solder components for those that fancy a bigger challenge. It also allows the design to be smaller.
- Added new configuration to BoardConfigs.cmake and bld.sh to support the new configuration for this PCB.

| | |
| ---- | ---- |
| ![NESMiniPCB](https://github.com/user-attachments/assets/64696de1-2896-4a9c-94e9-692f125c55b6) | ![NESMiniCase](https://github.com/user-attachments/assets/a68f31ff-529f-49fb-9ec4-f3512c8e9e38) |


## Fixes
- Pico 2 W executables added to the release.

All changes are in the pico_shared submodule. When building from source, make sure you do a **git submodule update --init** from within the source folder to get the latest pico_shared module.

