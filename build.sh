:
# ====================================================================================
# PICO-INFONESPLUS build script with default configuration
# Builds the emulator for use with the 
# Pimoroni Pico DV Demo Base
# https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291 
# ====================================================================================
cd `dirname $0` || exit 1
if [ -d build ] ; then
	rm -rf build || exit 1
	mkdir build || exit 1
fi
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=RELEASENODEBUG ..
make -j 4
