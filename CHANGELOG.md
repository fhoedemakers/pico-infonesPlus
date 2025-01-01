# CHANGELOG

# General Info

Binaries for each configuration and PCB design are at the end of this page.

- For Raspberry Pi Pico (RP2040) you need to download the .uf2 files starting with pico_.
- For Raspberry Pi Pico w (rp2040) you can download the .uf2 files starting with pico_w_. Although you can also use the pico_ binaries on the Pico w if you don't mind the blinking led.
- For Raspberry Pi Pico 2 (w) (RP2350) you need to download the .uf2 files starting with pico2_ or pico2_riscv_ for Risc-V. 

>[!NOTE]
>There is no specific build for the Pico 2 w because of issues with the display when blinking the led. Use the pico2_ binaries instead. There is no blinking led on the Pico 2 w.


[See setup section in readme how to install and wire up](https://github.com/fhoedemakers/pico-infonesPlus#pico-setup)

3D-printed case design for PCB: [https://www.thingiverse.com/thing:6689537](https://www.thingiverse.com/thing:6689537). 
For the latest two player PCB 2.0, you need:

- Top_v2.0_with_Bootsel_Button.stl. This allows for software upgrades without removing the cover. (*)
- Base_v2.0.stl
- Power_Switch.stl.

(*) in case you don't want to access the bootsel button on the Pico, you can choose Top_v2.0.stl

3D-printed case design for Waveshare RP2040-PiZero: [https://www.thingiverse.com/thing:6758682](https://www.thingiverse.com/thing:6758682)

# v0.25 release notes

## Features
- Enabe fastscrolling in the menu, by holding up/down/left/right for 500 milliseconds, repeat delay is 40 milliseconds.
- bld.sh mow uses the amount of cores available on the system to speed up the build process.

## Fixes
- Temporary Rollback NesPad code for the WaveShare RP2040-PiZero only. Other configurations are not affected.
- Update time functions to return milliseconds and use uint64_t to return microseconds.

All changes are in the pico_shared submodule. When building from source, make sure you do a **git submodule update --init** from within the source folder to get the latest pico_shared module.

# v0.24 release notes

## Features

- Menu now uses the full screen resolution of 320x240 pixels resulting in a 40x30 character display in stead of 32x29.

## Fixes

- Menu now displaying correctly on Pico2 Risc-V builds.

All changes are in the pico_shared submodule. When building from source, make sure you do a **git submodule update --init** from within the source folder to get the latest pico_shared module.

