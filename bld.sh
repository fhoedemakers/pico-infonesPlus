:
# Generic build script for the pico-infoNES-plus project
#
function usage() {
	echo "Build script for the pico-infoNES-plus project"
	echo  ""
	echo "Usage: $0 [-d] [-2] [-c <hwconfig>]"
	echo "Options:"
	echo "  -d: build in DEBUG configuration"
	echo "  -2: build for Pico 2 board"
	echo "  -c <hwconfig>: specify the hardware configuration"
	echo "     1: Pimoroni Pico DV Demo Base (Default)"
	echo "     2: Breadboard with Adafruit AdaFruit DVI Breakout Board and AdaFruit MicroSD card breakout board"
	echo "        Custom pcb"
	echo "     3: Adafruit Feather RP2040 DVI"
	echo "     4: Waveshare RP2040-PiZero"
	echo "     hwconfig 2 and 3 are RP2040-based boards, so they cannot be built for Pico 2"
	echo "  -h: display this help"
} 
PICO_BOARD=pico
BUILD=RELEASE
HWCONFIG=1
UF2="piconesPlusPimoroniDV.uf2"
# check if var PICO_SDK is set and points to the SDK
if [ -z "$PICO_SDK_PATH" ] ; then
	echo "PICO_SDK not set. Please set the PICO_SDK environment variable to the location of the Pico SDK"
	exit 1
fi
# check if the SDK is present
if [ ! -d "$PICO_SDK_PATH" ] ; then
	echo "PICO_SDK not found. Please set the PICO_SDK environment variable to the location of the Pico SDK"
	exit 1
fi
SDKVERSION=`cat ~/pico/pico-sdk/pico_sdk_version.cmake | grep "set(PICO_SDK_VERSION_MAJOR" | cut -f2  -d" " | cut -f1 -d\)`
while getopts "hd2c:" opt; do
  case $opt in
    d)
      BUILD=DEBUG
      ;;
    c)
      HWCONFIG=$OPTARG
      ;;
	2) 
	  PICO_BOARD=pico2
	  ;;
	h)
	  usage
	  exit 0
	  ;;
    \?)
      #echo "Invalid option: -$OPTARG" >&2
	  usage
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
	  usage
      exit 1
      ;;
	*)
	  usage
	  exit 1
	  ;;
  esac
done

case $HWCONFIG in
	1)
		UF2="piconesPlusPimoroniDV.uf2"
		;;
	2)
		UF2="piconesPlusAdaFruitDVISD.uf2"
		;;
	3) 
		UF2="piconesPlusFeatherDVI.uf2"
		;;
	4)
		UF2="piconesPlusWsRP2040PiZero.uf2"
		;;
	*)
		echo "Invalid value: $HWCONFIG specified for option -h"
		exit 1
		;;
esac
if [ "$PICO_BOARD" = "pico2" ] ; then
	UF2="pico2_$UF2"
fi	
echo "Using Pico SDK version: $SDKVERSION"
echo "Building for $PICO_BOARD with $BUILD configuration and HWCONFIG=$HWCONFIG"
echo "UF2 file: $UF2"

if [ $SDKVERSION -lt 2 -a $PICO_BOARD = "pico2" ] ; then
		echo "Pico SDK version $SDKVERSION does not support Pico 2. Please update the SDK to version 2 or higher"
		echo ""
		exit 1
fi
# pico2 board not compatible with HWCONFIG > 2
if [ $HWCONFIG -gt 2 -a $PICO_BOARD = "pico2" ] ; then
	echo "HW configuration $HWCONFIG is a RP2040 based board, not compatible with Pico 2"
	exit 1
fi
cd `dirname $0` || exit 1
[ -d releases ] || mkdir releases || exit 1
if [ -d build ] ; then
	rm -rf build || exit 1
fi
mkdir build || exit 1
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=$BUILD -DINFONES_PLUS_HW_CONFIG=$HWCONFIG -DPICO_BOARD=$PICO_BOARD ..
make -j 4
cd ..
echo ""
if [ -f build/piconesPlus.uf2 ] ; then
	cp build/piconesPlus.uf2 releases/${UF2} || exit 1
fi

