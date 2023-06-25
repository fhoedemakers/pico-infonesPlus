:
# ====================================================================================
# PICO-INFONESPLUS build all script 
# Builds the emulator for the default and alternate configuration
# Binaries are copied to the releases folder
#   - piconesPlusPimoroniDV.uf2     Pimoroni Pico DV Demo Base
#   - piconesPlusAdaFruitDVISD.uf2  AdaFruit HDMI and SD Breakout boards
#   - piconesPlusFeatherDVI.uf2     Adafruit Feather RP2040 DVI + SD Wing
# ====================================================================================
#export RETAINSDK=1
cd `dirname $0` || exit 1
[ -d releases ] || mkdir releases || exit 1
. ./checksdk.sh
./build.sh
if [ -f build/piconesPlus.uf2 ] ; then
	cp build/piconesPlus.uf2 releases/piconesPlusPimoroniDV.uf2 || exit 1
fi
cd `dirname $0` || exit 1
./build_alternate.sh
if [ -f build/piconesPlus.uf2 ] ; then
	cp build/piconesPlus.uf2 releases/piconesPlusAdaFruitDVISD.uf2 || exit 1
fi
cd `dirname $0` || exit 1
./build_feather_dvi.sh
if [ -f build/piconesPlus.uf2 ] ; then
	cp build/piconesPlus.uf2 releases/piconesPlusFeatherDVI.uf2 || exit 1
fi
ls -l releases
#unset RETAINSDK
#. ./removetmpsdk.sh
