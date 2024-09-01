:
# ====================================================================================
# PICO-INFONESPLUS build script in DEBUG configuration
# Builds the emulator for use with the
# Waveshare RP2040-PiZero
# https://www.waveshare.com/rp2040-pizero.htm
# ====================================================================================
cd `dirname $0` || exit 1
./bld.sh -c 4 -d

