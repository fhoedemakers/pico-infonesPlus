:
# ====================================================================================
# PICO-INFONESPLUS build script with alternate configuration 2 and debug enabled
# Builds the emulator for use with the
# AdaFruit Feather RP2040 DVI
#         https://www.adafruit.com/product/TBD
# and the 
# AdaFruit Adalogger FeatherWing
#         https://www.adafruit.com/product/2922
# (or connect SD breakout using same pins as FeatherWing)
# ====================================================================================
cd `dirname $0` || exit 1
. ./checksdk.sh
if [ -d build ] ; then
	rm -rf build || exit 1
	mkdir build || exit 1
fi
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=DEBUG -DINFONES_PLUS_USE_ALTERNATE_HW_CONFIG=2 ..
make -j 4
cd ..
. ./removetmpsdk.sh
