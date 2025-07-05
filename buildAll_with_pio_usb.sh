:
# -----------------------------------------------------------------------------
# buildAll_with_pio_usb.sh
#
# Build script for pico-infonesPlus with PIO USB enabled.
# This script builds supported hardware configurations with PIO USB enabled.
# RP2350 only because of memory constraints in RP2040.
#
# Usage:
#   ./buildAll_with_pio_usb.sh
#
# Supported configurations:
#   - Breadboard with pico2(w)
#   - Adafruit Metro RP2350
# -----------------------------------------------------------------------------
:
cd `dirname $0` || exit 1
./bld.sh -c2 -2 -u       || exit 1
./bld.sh -c2 -2 -w -u    || exit 1
./bld.sh -c2 -r -u      || exit 1
./bld.sh -c2 -r -w -u   || exit 1
./bld.sh -c 5 -2 -u         || exit 1
for uf2 in releases/*_pio_usb*.uf2
do
    picotool info $uf2
    echo " "
done