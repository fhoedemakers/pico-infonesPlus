#!/usr/bin/env python3
import sys

if len(sys.argv) != 3:
    print("Usage: extract_crc_system.py input.txt output.cpp")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

entries = []

with open(input_file) as f:
    for line in f:
        line = line.strip()

        # Skip comments and empty lines
        if not line or line.startswith("#"):
            continue

        parts = line.split(",")

        # Extract CRC and System
        crc = int(parts[0], 16)
        system = parts[1]

        entries.append((crc, system))

# Write C++ output
with open(output_file, "w") as out:
    out.write("#include <cstdint>\n\n")

    out.write("struct GameEntry {\n")
    out.write("    uint32_t crc;\n")
    out.write("    const char* system;\n")
    out.write("};\n\n")

    out.write("static const GameEntry gameDb[] = {\n")

    for crc, system in entries:
        out.write(f'    {{0x{crc:08X}, "{system}"}},\n')

    out.write("};\n")

print(f"Generated {output_file} with {len(entries)} entries.")
