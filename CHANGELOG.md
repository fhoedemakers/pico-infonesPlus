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

# v0.26 release notes (To be released)

## Features

- Improved SD card Support:
    - Updated to the latest version of the pico_fatfs library from https://github.com/elehobica/pico_fatfs.
    - Besides FAT32, SD cards can now also be formatted as exFAT.
- Nes controller PIO code updated by [@ManCloud](https://github.com/ManCloud). This fixes the issue with the controller issues  on Waveshare RP2040 - PiZero board. [#8](https://github.com/fhoedemakers/pico_shared/issues/8)
- Support for Adafruit Metro RP2350 board. You need the following materials:
    - Adafruit Metro RP2350 board. https://www.adafruit.com/product/6267
    - 22-pin 0.5mm pitch FPC flex cable for DSI CSI or HSTX. https://www.adafruit.com/product/6036
    - Adafruit RP2350 22-pin FPC HSTX to DVI Adapter for HDMI Displays. https://www.adafruit.com/product/6055 
    - Usb-c Y-cable. https://a.co/d/9vCzu0h For power and USB-controller. NES controller support is not yet available. You can use the USB controller for now. You need to build the firmware for the Metro RP2350 board yourself. See the instructions below.

````bash
./bld.sh -c5 -2
````

## Fixes


All changes are in the pico_shared submodule. When building from source, make sure you do a **git submodule update --init** from within the source folder to get the latest pico_shared module.

