:
# ====================================================================================
# PICO-INFONESPLUS build all script 
# Builds the emulator for the default and alternat configuration
# Binaries are copied to the releases folder
#   - piconesPlusPimoroniDV.uf2     Pimoroni Pico DV Demo Base
#   - piconesPlusAdaFruitDVISD.uf2  AdaFruit HDMI and SD Breakout boards
# ====================================================================================
cd `dirname $0` || exit 1
[ -d releases ] || mkdir releases || exit 1
./build.sh
if [ -f build/piconesPlus.uf2 ] ; then
	cp build/piconesPlus.uf2 releases/piconesPlusPimoroniDV.uf2 || exit 1
fi
cd `dirname $0` || exit 1
./build_alternate.sh
if [ -f build/piconesPlus.uf2 ] ; then
	cp build/piconesPlus.uf2 releases/piconesPlusAdaFruitDVISD.uf2 || exit 1
fi
ls -l releases
