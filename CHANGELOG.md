# CHANGELOG

# General Info

Binaries for each configuration and PCB design are at the end of this page.

- For Raspberry Pi Pico (RP2040): Download the .uf2 files that start with pico_piconesPlus.
- For Raspberry Pi Pico W (RP2040 with WiFi): Download the .uf2 files that start with pico_w_piconesPlus. You can also use the regular pico_piconesPlus files if you don’t mind the WiFi LED blinking.
- For Raspberry Pi Pico 2 (RP2350): Download the .uf2 files that start with pico2_piconesPlus (for ARM) or pico2_riscv_piconesPlus (for RISC-V).
- For Raspberry Pi Pico 2 W (RP2350 with WiFi): Download the .uf2 files that start with pico2_w_piconesPlus (for ARM) or pico2_w_riscv_piconesPlus (for RISC-V). You can also use the non-W files if you don’t mind the WiFi LED blinking.

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

# v0.28 release notes 

- Enable I2S audio on the Pimoroni Pico DV Demo Base. This allows audio output through external speakers connected to the line-out jack of the Pimoroni Pico DV Demo Base. You can toggle audio output to this jack with SELECT + A.

## Fixes
none

All changes are in the pico_shared submodule. When building from source, make sure you do a **git submodule update --init** from within the source folder to get the latest pico_shared module.

