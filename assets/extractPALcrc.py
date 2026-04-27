#!/usr/bin/env python3
import xml.etree.ElementTree as ET

# Load XML file
tree = ET.parse("NstDatabase.xml")  # change to your filename
root = tree.getroot()

crc_values = []
# Accept both systems
valid_systems = {"Dendy"}   # { "NES-PAL", "Dendy" }
# Iterate through all cartridges
for cartridge in root.findall(".//cartridge"):
    system = cartridge.get("system")
    crc = cartridge.get("crc")

    if system in valid_systems and crc:
        # Convert hex string to integer
        value = int(crc, 16)
        crc_values.append(value)

# Generate C array
print("const uint32_t nes_pal_crcs[] = {")
for v in crc_values:
    print(f"    0x{v:08X},")
print("};")

print(f"\nconst size_t nes_pal_crcs_count = {len(crc_values)};")
