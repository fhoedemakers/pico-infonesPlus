:
# ====================================================================================
# PICO-INFONESPLUS build script with alternate configuration
# Builds the emulator for use with the
# AdaFruit DVI Breakout Board
#         https://www.adafruit.com/product/4984
#         https://learn.adafruit.com/adafruit-dvi-breakout-board
# and the 
# AdaFruit  MicroSD card breakout board+
#         https://www.adafruit.com/product/254 
# ====================================================================================
cd `dirname $0` || exit 1
#. ./checksdk.sh
if [ -d build ] ; then
	rm -rf build || exit 1
fi
mkdir build || exit 1
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=RELEASENODEBUG -DINFONES_PLUS_HW_CONFIG=2 ..
make -j 4
cd ..
#. ./removetmpsdk.sh
