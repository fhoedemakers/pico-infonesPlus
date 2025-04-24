
# Test results for release v0.26

Below the different possible configs and their status are listed. The status is either OK, NOK or Untested. All is built using SDK 2.1.1

Unfortunately, i don't have all the hardware to test all configurations. If you have a configuration that is not listed, please let me know and I will add it to the list. If you have a configuration that is listed as untested, please let me know if it works or not.

| Add-on Board          | Board                    | Processor | Arch | SD-driver | Status   | Remarks         |
|-----------------------|--------------------------|-----------|------|-----------|----------|-----------------|
| -                     | Adafruit Metro RP2350    | RP2350    | ARM  | New       | OK       |                 |
| -                     | Adafruit Metro RP2350    | RP2350    | RISCV| New       | NOK      | mount error 3   |
| Adafruit FeatherWing  | Adafruit Feather DVI     | RP2040    | ARM  | New       | Untested |                 |
| Breadboard            | Adafruit Feather DVI     | RP2040    | ARM  | New       | OK       |                 |
| Breadboard            | Pico                     | RP2040    | ARM  | New       | OK       |                 |
| Breadboard            | Pico2                    | RP2350    | ARM  | New       | OK       |                 |
| Breadboard            | Pico2                    | RP2350    | RISCV| New       | OK       |                 |
| Breadboard            | Pico-w                   | RP2040    | ARM  | New       | Untested | Should work     |
| Breadboard            | Pico-2w                  | RP2350    | ARM  | New       | Untested | Should work     |
| Breadboard            | Pico-2w                  | RP2350    | RISCV| New       | Untested | Should work     |
| PCB v2.x              | Pico                     | RP2040    | ARM  | New       | OK       |                 |
| PCB v2.x              | Pico-2                   | RP2350    | ARM  | New       | OK       |                 |
| PCB v2.x              | Pico-2                   | RP2350    | RISCV| New       | OK       |                 |
| PCB v2.x              | Pico-2w                  | RP2350    | ARM  | New       | Untested | Should work     |
| PCB v2.x              | Pico-2w                  | RP2350    | RISCV| New       | Untested | Should work     |
| PCB v2.x              | Pico-w                   | RP2040    | ARM  | New       | Untested | Should work     |
| Pimoroni Pico DV      | Pico                     | RP2040    | ARM  | Old       | Untested | Should work     |
| Pimoroni Pico DV      | Pico-2                   | RP2350    | ARM  | Old       | OK       |                 |
| Pimoroni Pico DV      | Pico-2                   | RP2350    | RISCV| Old       | OK       |                 |
| Pimoroni Pico DV      | Pico-2w                  | RP2350    | ARM  | Old       | OK       |                 |
| Pimoroni Pico DV      | Pico-2w                  | RP2350    | RISCV| Old       | OK       |                 |
| Pimoroni Pico DV      | Pico-w                   | RP2040    | ARM  | Old       | OK       |                 |
| -                     | Waveshare RP2040 PiZero  | RP2040    | ARM  | New       | OK       |                 |


