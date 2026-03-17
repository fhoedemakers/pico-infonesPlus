# MMC5

*From NESdev Wiki*

---

## MMC5

**Mapper:** 5  
**Board type:** ExROM  
**Company:** Nintendo, Koei, others  

### Specifications

- **PRG ROM capacity:** 1024K  
- **PRG ROM window:** 8K, 16K, or 32K  
- **PRG RAM capacity:** 128K  
- **CHR capacity:** 1024K  
- **CHR window:** 1K, 2K, 4K, or 8K  
- **IRQ:** Yes  
- **Audio:** Yes  

---

## Description

The **Nintendo MMC5** is a mapper ASIC used in ExROM Game Pak boards.  
All MMC5 boards are assigned to mapper 5.

Example games:

- Castlevania 3  
- Just Breed  
- Uncharted Waters  
- Romance of the Three Kingdoms II  
- Laser Invasion  
- Metal Slader Glory  
- Uchuu Keibitai SDF  
- Shin 4 Nin Uchi Mahjong - Yakuman Tengoku  
- Bandit Kings of Ancient China  

The first game using this chip (*Nobunaga's Ambition II*) was released in February 1990.

A later **MMC5A** revision added features, but all games remain compatible.

---

## Contents

1. Overview  
2. Banks  
   - PRG modes (0–3)  
   - CHR modes (0–3)  
3. Registers  
4. MMC5A  
5. Scanline detection  
6. Hardware  
7. References  

---

## Overview

The MMC5 is the most elaborate mapper ASIC Nintendo made for the NES.

### Features

- 4 PRG ROM switching modes  
- 4 CHR ROM switching modes  
- Up to 128KB WRAM  
- Hardware multiplier (8×8 → 16-bit)  
- Scanline IRQ system  
- Frame detection  
- Separate CHR for sprites/background  
- 1KB internal RAM usable for:
  - Extra nametable  
  - Attribute expansion  
  - Vertical split screen  
  - General RAM  

### Audio

- 2 pulse channels  
- 1 PCM channel  

### Other

- Fill-mode nametable  
- System reset detection  

---

## Banks

### PRG Mode 0

- $6000–$7FFF: 8KB PRG RAM  
- $8000–$FFFF: 32KB PRG ROM  

### PRG Mode 1

- $6000–$7FFF: 8KB PRG RAM  
- $8000–$BFFF: 16KB PRG (ROM/RAM)  
- $C000–$FFFF: 16KB PRG ROM  

### PRG Mode 2

- $6000–$7FFF: 8KB PRG RAM  
- $8000–$9FFF: 8KB PRG (ROM/RAM)  
- $A000–$BFFF: 8KB PRG (ROM/RAM)  
- $C000–$FFFF: 16KB PRG ROM  

### PRG Mode 3

- Fully switchable 8KB banks  

---

### CHR Modes

#### Mode 0
- One 8KB bank  

#### Mode 1
- Two 4KB banks  

#### Mode 2
- Four 2KB banks  

#### Mode 3
- Eight 1KB banks  

---

## Registers

### Sound ($5000–$5015)

Controls MMC5 audio channels.

---

### NES Internal State Monitoring

- $2000 → 8x16 sprite mode detect  
- $2001 → rendering flags  
- $2002 → scanline reset  
- $4014 → DMA trigger reset  

---

### Configuration Registers

| Address | Function |
|--------|---------|
| $5100 | PRG mode |
| $5101 | CHR mode |
| $5102–$5103 | PRG RAM protect |
| $5104 | Extended RAM mode |
| $5105 | Nametable mapping |
| $5106 | Fill tile |
| $5107 | Fill color |

---

### PRG Banking ($5113–$5117)

Controls PRG switching.

#### Notes

- Separate PRG-ROM / PRG-RAM buses  
- Flexible RAM configurations  

---

### CHR Banking ($5120–$5130)

- $5120–$512B → CHR banks  
- $5130 → upper bits  

---

### Other Registers

#### Vertical Split ($5200)

Allows split-screen rendering.

#### Scroll ($5201)

Controls split scroll.

#### Bank ($5202)

Selects CHR bank for split.

---

### IRQ

| Register | Purpose |
|----------|--------|
| $5203 | Scanline compare |
| $5204 | IRQ status |

---

### Multiplier

- $5205, $5206  
- 8-bit × 8-bit → 16-bit result  

---

### Extended RAM ($5C00–$5FFF)

Used for:

- Nametables  
- Attributes  
- General RAM  

---

## MMC5A

Additional features:

- Extra registers  
- Hardware timer with IRQ  
- Additional I/O  

Not used by commercial games.

---

## Scanline Detection

MMC5 includes a scanline counter used for:

- IRQ triggering  
- Mid-frame effects  

---

## Hardware Notes

- Reset detection via M2 signal  
- Partial state reset on power cycle  
- PRG RAM chip enable reflects reset state  

---

## References

- NESdev Wiki  

---

## External Links

- https://www.nesdev.org/wiki/MMC5
