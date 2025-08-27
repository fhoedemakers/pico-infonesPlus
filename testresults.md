# Test results for release v0.29

Below the different possible configs and their status are listed. The status is either OK, NOK or Untested. All is built using SDK 2.2.0

Unfortunately, i don't have all the hardware to test all configurations. If you have a configuration that is not listed, please let me know and I will add it to the list. If you have a configuration that is listed as untested, please let me know if it works or not.

| Add-on Board       | Board                  | Processor | Arch |  Status   | Remarks       |
|--------------------|------------------------|-----------|------|----------|---------------|
| -                  | Adafruit Fruit Jam     | RP2350    | ARM  | OK       |               |
| -                  | Adafruit Fruit Jam     | RP2350    | ARM  | Unavailable|               |
| -                  | Adafruit Metro RP2350  | RP2350    | ARM  | OK |      Built with DVI, I2S_AUDIO disabled, I2S_AUDIO only works with HSTX         |
| -                  | Adafruit Metro RP2350  | RP2350    | RISCV| NOK      | mount error 3 |
| Adafruit FeatherWing| Adafruit Feather DVI  | RP2040    | ARM  | Untested |               |
| Breadboard         | Adafruit Feather DVI   | RP2040    | ARM  | Untested |               |
| Breadboard         | Pico                   | RP2040    | ARM  | Untested |               |
| Breadboard         | Pico2                  | RP2350    | ARM  | OK |               |
| Breadboard         | Pico2                  | RP2350    | RISCV| Untested       |               |
| Breadboard         | Pico-w                 | RP2040    | ARM  | Untested | Should work   |
| Breadboard         | Pico2-w                | RP2350    | ARM  | Untested | Should work   |
| Breadboard         | Pico2-w                | RP2350    | RISCV| Untested | Should work   |
| Breadboard         | Pimoroni Pico Plus 2     | RP2350  | ARM  | OK       |               |
| PCB v2.x           | Pico                   | RP2040    | ARM  | Untested       |               |
| PCB v2.x           | Pico2                  | RP2350    | ARM  | OK       |               |
| PCB v2.x           | Pico2                  | RP2350    | RISCV| Untested        |               |
| PCB v2.x           | Pico2-w                | RP2350    | ARM  | Untested | Should work   |
| PCB v2.x           | Pico2-w                | RP2350    | RISCV| Untested | Should work   |
| PCB v2.x           | Pico-w                 | RP2040    | ARM  | Untested | Should work   |
| PCB PicoNes Mini v1.0 | Waveshare RP2040-Zero | RP2040  | ARM  | Untested | Should work              |
| PCB PicoNes Mini v1.0 | Waveshare RP2350-Zero | RP2350  | ARM  | Untested         |             |
| PCB PicoNes Mini v1.0 | Waveshare RP2350-Zero | RP2350  | RISCV | Untested       |  |
| Pimoroni Pico DV   | Pico                   | RP2040    | ARM  | Untested       |               |
| Pimoroni Pico DV   | Pico2                  | RP2350    | ARM  | Untested       |               |
| Pimoroni Pico DV   | Pico2                  | RP2350    | RISCV| Untested       |               |
| Pimoroni Pico DV   | Pico2-w                | RP2350    | ARM  | Untested       |               |
| Pimoroni Pico DV   | Pico2-w                | RP2350    | RISCV| Untested       |               |
| Pimoroni Pico DV   | Pico-w                 | RP2040    | ARM  | Untested       |               |
| Pimoroni Pico DV   | Pimoroni Pico Plus 2     | RP2350  | ARM  | Untested       |               |
| -                  | Waveshare RP2040 PiZero| RP2040    | ARM  | Untested       |               |
| -                  | Waveshare RP2350 PiZero| RP2350    | ARM  | Untested       |               |

# Test results for release v0.28

Below the different possible configs and their status are listed. The status is either OK, NOK or Untested. All is built using SDK 2.1.1

Unfortunately, i don't have all the hardware to test all configurations. If you have a configuration that is not listed, please let me know and I will add it to the list. If you have a configuration that is listed as untested, please let me know if it works or not.

| Add-on Board       | Board                  | Processor | Arch | SD-driver | Status   | Remarks       |
|--------------------|------------------------|-----------|------|-----------|----------|---------------|
| -                  | Adafruit Metro RP2350  | RP2350    | ARM  | New       | OK       |               |
| -                  | Adafruit Metro RP2350  | RP2350    | RISCV| New       | NOK      | mount error 3 |
| Adafruit FeatherWing| Adafruit Feather DVI  | RP2040    | ARM  | New       | Untested |               |
| Breadboard         | Adafruit Feather DVI   | RP2040    | ARM  | New       | OK       |               |
| Breadboard         | Pico                   | RP2040    | ARM  | New       | OK       |               |
| Breadboard         | Pico2                  | RP2350    | ARM  | New       | OK       |               |
| Breadboard         | Pico2                  | RP2350    | RISCV| New       | OK       |               |
| Breadboard         | Pico-w                 | RP2040    | ARM  | New       | Untested | Should work   |
| Breadboard         | Pico2-w                | RP2350    | ARM  | New       | Untested | Should work   |
| Breadboard         | Pico2-w                | RP2350    | RISCV| New       | Untested | Should work   |
| PCB v2.x           | Pico                   | RP2040    | ARM  | New       | OK       |               |
| PCB v2.x           | Pico2                  | RP2350    | ARM  | New       | OK       |               |
| PCB v2.x           | Pico2                  | RP2350    | RISCV| New       | OK        |               |
| PCB v2.x           | Pico2-w                | RP2350    | ARM  | New       | Untested | Should work   |
| PCB v2.x           | Pico2-w                | RP2350    | RISCV| New       | Untested | Should work   |
| PCB v2.x           | Pico-w                 | RP2040    | ARM  | New       | Untested | Should work   |
| PCB PicoNes Mini v1.0 | Waveshare RP2040-Zero | RP2040  | ARM  | New       | Untested | Should work              |
| PCB PicoNes Mini v1.0 | Waveshare RP2350-Zero | RP2350  | ARM  | New       | OK         |  tested by [@DynaMight1124](https://github.com/DynaMight1124)             |
| PCB PicoNes Mini v1.0 | Waveshare RP2350-Zero | RP2350  | RISCV | New      | OK       | Tested by [@DynaMight1124](https://github.com/DynaMight1124)              |
| Pimoroni Pico DV   | Pico                   | RP2040    | ARM  | New       | OK       |               |
| Pimoroni Pico DV   | Pico2                  | RP2350    | ARM  | New       | OK       |               |
| Pimoroni Pico DV   | Pico2                  | RP2350    | RISCV| New       | OK       |               |
| Pimoroni Pico DV   | Pico2-w                | RP2350    | ARM  | New       | OK       |               |
| Pimoroni Pico DV   | Pico2-w                | RP2350    | RISCV| New       | OK       |               |
| Pimoroni Pico DV   | Pico-w                 | RP2040    | ARM  | New       | OK       |               |
| -                  | Waveshare RP2040 PiZero| RP2040    | ARM  | New       | OK       |               |


# Test results for release v0.27

No code changes are done in this release, only configuration added for Waveshare RP2040-Zero and RP2350-Zero.

Below the different possible configs and their status are listed. The status is either OK, NOK or Untested. All is built using SDK 2.1.1

Unfortunately, i don't have all the hardware to test all configurations. If you have a configuration that is not listed, please let me know and I will add it to the list. If you have a configuration that is listed as untested, please let me know if it works or not.

| Add-on Board       | Board                  | Processor | Arch | SD-driver | Status   | Remarks       |
|--------------------|------------------------|-----------|------|-----------|----------|---------------|
| -                  | Adafruit Metro RP2350  | RP2350    | ARM  | New       | OK       |               |
| -                  | Adafruit Metro RP2350  | RP2350    | RISCV| New       | NOK      | mount error 3 |
| Adafruit FeatherWing| Adafruit Feather DVI  | RP2040    | ARM  | New       | Untested |               |
| Breadboard         | Adafruit Feather DVI   | RP2040    | ARM  | New       | OK       | OK            |
| Breadboard         | Pico                   | RP2040    | ARM  | New       | OK       | OK            |
| Breadboard         | Pico2                  | RP2350    | ARM  | New       | OK       | OK            |
| Breadboard         | Pico2                  | RP2350    | RISCV| New       | OK       | OK            |
| Breadboard         | Pico-w                 | RP2040    | ARM  | New       | Untested | Should work   |
| Breadboard         | Pico2-w                | RP2350    | ARM  | New       | Untested | Should work   |
| Breadboard         | Pico2-w                | RP2350    | RISCV| New       | Untested | Should work   |
| PCB v2.x           | Pico                   | RP2040    | ARM  | New       | OK       | OK            |
| PCB v2.x           | Pico2                  | RP2350    | ARM  | New       | OK       | OK            |
| PCB v2.x           | Pico2                  | RP2350    | RISCV| New       | OK       | OK            |
| PCB v2.x           | Pico2-w                | RP2350    | ARM  | New       | Untested | Should work   |
| PCB v2.x           | Pico2-w                | RP2350    | RISCV| New       | Untested | Should work   |
| PCB v2.x           | Pico-w                 | RP2040    | ARM  | New       | Untested | Should work   |
| PCB PicoNes Mini v1.0 | Waveshare RP2040-Zero | RP2040  | ARM  | New       | Untested | Should work              |
| PCB PicoNes Mini v1.0 | Waveshare RP2350-Zero | RP2350  | ARM  | New       | OK       |  tested by [@DynaMight1124](https://github.com/DynaMight1124)             |
| PCB PicoNes Mini v1.0 | Waveshare RP2350-Zero | RP2350  | RISCV | New       | OK       | Tested by [@DynaMight1124](https://github.com/DynaMight1124)              |
| Pimoroni Pico DV   | Pico                   | RP2040    | ARM  | New       | Untested | Should work   |
| Pimoroni Pico DV   | Pico2                  | RP2350    | ARM  | New       | OK       | OK            |
| Pimoroni Pico DV   | Pico2                  | RP2350    | RISCV| New       | OK       | OK            |
| Pimoroni Pico DV   | Pico2-w                | RP2350    | ARM  | New       | OK       | OK            |
| Pimoroni Pico DV   | Pico2-w                | RP2350    | RISCV| New       | OK       | OK            |
| Pimoroni Pico DV   | Pico-w                 | RP2040    | ARM  | New       | OK       | OK            |
| -                  | Waveshare RP2040 PiZero| RP2040    | ARM  | New       | OK       | OK            |


