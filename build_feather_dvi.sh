:
# ====================================================================================
# PICO-INFONESPLUS build script with alternate configuration
# Builds the emulator for use with the
# Adafruit Feather RP2040 DVI
# and the 
# Adafruit microSD Card FeatherWing
# ====================================================================================
cd `dirname $0` || exit 1
#. ./checksdk.sh
if [ -d build ] ; then
	rm -rf build || exit 1
fi
mkdir build || exit 1
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=RELEASE -DINFONES_PLUS_HW_CONFIG=3 ..
make -j 4
cd ..
#. ./removetmpsdk.sh
