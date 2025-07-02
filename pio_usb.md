# Build with PIO USB

It is possible to build the Pico Infones Plus with PIO USB support. This enables the use of a second USB port for attaching a game controller.

The built-in usb port is used for providing power and for flashing the firmware. The PIO USB port is used for attaching a game controller.

> [!CAUTION]
> Do not provide power to the PIO USB port. It is not designed to provide power and may damage the board.

## Supported configurations

### Breadboard

Add this Adafruit USB Type C breakout board to the breadboard: https://www.adafruit.com/product/4090

![image](https://github.com/user-attachments/assets/417d49cd-94dd-4a6e-8e5f-ff2bfd65684e)


Pinouts:

- **VBUS**: Connect to 5V on the breadboard
- **GND**: Connect to GND on the breadboard
- **D+**: Connect to GPIO 20 on the Pico
- **D-**: Connect to GPIO 21 on the Pico


### Adafruit Metro RP2350

Solder this USB type A jack breakout cable to the Adafruit Metro RP2350 USB host port: https://www.adafruit.com/product/4449

![image](https://github.com/user-attachments/assets/4819f7c1-9759-4fc9-9452-e082d315efb2)

You need some headers like these: https://www.adafruit.com/product/392

![image](https://github.com/user-attachments/assets/62bba136-05e2-457b-b42d-a3990d11778e)


Pinouts:
- **GND**: Black
- **D-**: White
- **D+**: Green
- **5V**: Red

![image](https://github.com/user-attachments/assets/b05a4c47-cd3d-45f9-ab04-327c7a6136b9)

## Building

Requirements:

- Latest Master branch of TinyUSB

```bash
cd $PICO_SDK_PATH/lib/tinyusb
git checkout master
git pull
```

- PIO Pio USB: https://github.com/sekigon-gonnoc/Pico-PIO-USB Point the environment variabele PICO_PIO_USB_PATH to this repository

```
cd ~
git clone https://github.com/sekigon-gonnoc/Pico-PIO-USB.git
export PICO_PIO_USB_PATH=~/Pico-PIO-USB
# You may also put this environment var in your .bashrc
```

To build, use this script: **buildAll_with_pio_usb.sh**


