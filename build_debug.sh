:
# ====================================================================================
# PICO-INFONESPLUS build script with default configuration and debug enabled
# Builds the emulator for use with the 
# Pimoroni Pico DV Demo Base
# https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291 
# ====================================================================================
cd `dirname $0` || exit 1
. ./checksdk.sh
if [ -d build ] ; then
	rm -rf build || exit 1
fi
mkdir build || exit 1
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=DEBUG -DINFONES_PLUS_HW_CONFIG=1 ..
make -j 4
cd ..
. ./removetmpsdk.sh
