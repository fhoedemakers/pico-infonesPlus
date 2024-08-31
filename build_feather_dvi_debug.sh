:
# ====================================================================================
# PICO-INFONESPLUS build script with alternate configuration and debug enabled
# Builds the emulator for use with the
# Adafruit Feather RP2040 DVI
# and the 
# Adafruit microSD Card FeatherWing
# ====================================================================================
cd `dirname $0` || exit 1
./bld.sh -h 3 -d