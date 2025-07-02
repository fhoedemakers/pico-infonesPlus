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
if [ ! -d "${PICO_SDK_PATH}/lib/tinyusb/hw/bsp/rp2040/boards/adafruit_metro_rp2350" ] ; then
    echo "Adafruit Metro RP2350 board not found in $PICO_SDK_PATH/lib/tinyusb/hw/bsp/rp2040/boards/"
    echo "Please install the latest TinyUSB library:"
    echo " cd $PICO_SDK_PATH/lib/tinyusb"
    echo " git checkout master"
    echo " git pull"
    exit 1
fi
# check if environment var PICO_PIO_USB_PATH is set and points to a valid path
piovalid=1
if [ -z "$PICO_PIO_USB_PATH" ] ; then
    echo "PICO_PIO_USB_PATH not set."
    echo "Please set the PICO_PIO_USB_PATH environment variable to the location of the PIO USB library"
    piovalid=0
elif [ ! -r "${PICO_PIO_USB_PATH}/src/pio_usb.h" ] ; then
    echo "No valid PIO USB repo found."
    echo "Please set the PICO_PIO_USB_PATH environment variable to the location of the PIO USB library"
    piovalid=0
fi
if [ $piovalid -eq 0 ] ; then
    echo "To install the PIO USB library:"
    echo " git clone https://github.com/sekigon-gonnoc/Pico-PIO-USB.git"
    echo " and set the PICO_PIO_USB_PATH environment variable to the location of the Pico-PIO-USB repository"
    echo " Example: export PICO_PIO_USB_PATH=~/Pico-PIO-USB"
    echo " or add it to your .bashrc file"
    exit 1
fi
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