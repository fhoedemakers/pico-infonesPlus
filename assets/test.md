
# MMC5

*From NESdev Wiki*

## Overview

The **Nintendo MMC5** is a mapper ASIC used in Nintendo's ExROM Game Pak boards.  
All MMC5 boards are assigned to **mapper 5**.

It is the most elaborate mapper ASIC made for the NES and Famicom.

### Example games

- Castlevania 3  
- Just Breed  
- Uncharted Waters  
- Romance of the Three Kingdoms II  
- Laser Invasion  
- Metal Slader Glory  
- Uchuu Keibitai SDF  
- Shin 4 Nin Uchi Mahjong - Yakuman Tengoku  
- Bandit Kings of Ancient China  

The first game to use this chip (*Nobunaga's Ambition II*) was released in February 1990.

A later **MMC5A** revision introduced additional features, but all released games remain compatible with the original MMC5.

---

## Specifications

| Feature | Details |
|--------|--------|
| Company | Nintendo, Koei, others |
| Boards | EKROM, ELROM, ETROM, EWROM |
| PRG ROM capacity | 1024 KB |
| PRG ROM window | 8K, 16K, or 32K |
| PRG RAM capacity | 128 KB |
| CHR capacity | 1024 KB |
| CHR window | 1K, 2K, 4K, or 8K |
| IRQ support | Yes |
| Audio | Yes |
| Mapper ID | 5 |

---

## Features

The MMC5 supports a wide range of advanced features:

- 4 PRG ROM switching modes  
- 4 CHR ROM switching modes  
- Up to 128 KB WRAM  
- Flexible nametable mapping  
- Scanline IRQs  
- Extended RAM  
- Hardware multiplier (8×8 → 16-bit)  
- Additional audio channels  

---

## Memory & Banking

### PRG Modes
- Mode 0
- Mode 1
- Mode 2
- Mode 3

Each mode defines how PRG ROM/RAM is mapped into CPU address space.

### CHR Modes
- Mode 0
- Mode 1
- Mode 2
- Mode 3

These control how CHR data is mapped for graphics.

---

## Registers

### Sound ($5000–$5015)
Controls MMC5 audio hardware.

### Configuration Registers

- `$5100` – PRG mode  
- `$5101` – CHR mode  
- `$5102-$5103` – PRG RAM protection  
- `$5104` – Extended RAM mode  
- `$5105` – Nametable mapping  
- `$5106-$5107` – Fill mode tile/color  

### PRG Banking ($5113–$5117)
Controls PRG ROM/RAM switching.

### CHR Banking ($5120–$5130)
Controls CHR ROM switching.

---

## Advanced Features

### Vertical Split Mode ($5200)
Allows splitting the screen into regions with different graphics.

### Scanline IRQ

- `$5203` – Scanline compare value  
- `$5204` – IRQ status  

Used for mid-frame effects.

### Hardware Multiplier

- `$5205`, `$5206`  
- Performs 8-bit × 8-bit multiplication  
- Result is 16-bit  

---

## Extended RAM

- Address range: `$5C00–$5FFF`  
- Used for:
  - Extra nametable data  
  - Attributes  
  - General-purpose RAM  

---

## MMC5A Variant

Adds:

- Extra registers  
- Hardware timer with IRQ  
- Additional I/O features  

Note: Not required by any released games.

---

## Summary

The MMC5 is the most powerful NES mapper, providing:

- Advanced memory control  
- Enhanced graphics capabilities  
- Extra audio hardware  
- Hardware math support  

It enabled some of the most technically advanced NES games ever released.

---

## Source

Original page: https://www.nesdev.org/wiki/MMC5
