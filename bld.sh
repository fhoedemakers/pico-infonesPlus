:
# Generic build script for the project
#
APP=piconesPlus
PROJECT="pico-InfoNESPlus"
function usage() {
	echo "Build script for the ${PROJECT} project"
	echo  ""
	echo "Usage: $0 [-d] [-2 | -r] [-t path to toolchain] [-c <hwconfig>]"
	echo "Options:"
	echo "  -d: build in DEBUG configuration"
	echo "  -2: build for Pico 2 board (RP2350)"
	echo "  -r: build for Pico 2 board (RP2350) with riscv core"
	echo "  -t <path to toolchain>: only needed for riscv, specify the path to the riscv toolchain bin folder"
	echo "  -c <hwconfig>: specify the hardware configuration"
	echo "     1: Pimoroni Pico DV Demo Base (Default)"
	echo "     2: Breadboard with Adafruit AdaFruit DVI Breakout Board and AdaFruit MicroSD card breakout board"
	echo "        Custom pcb"
	echo "     3: Adafruit Feather RP2040 DVI"
	echo "     4: Waveshare RP2040-PiZero"
	echo "     hwconfig 3 and 4 are RP2040-based boards, so they cannot be built for Pico 2"
	echo "  -h: display this help"
	echo ""
	echo "To install the RISC-V toolchain:"
	echo " - Raspberry Pi: https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.0.0-5/riscv-toolchain-14-aarch64-lin.tar.gz"
	echo " - X86/64 Linux: https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.0.0-5/riscv-toolchain-14-x86_64-lin.tar.gz"
	echo "and extract it to \$PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2"	
	echo ""
	echo "Example riscv toolchain install for Raspberry Pi OS:"
	echo ""
	echo -e "\tcd"
	echo -e "\tsudo apt-get install wget"
	echo -e "\twget https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.0.0-5/riscv-toolchain-14-aarch64-lin.tar.gz"
	echo -e "\tmkdir -p \$PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2"
	echo -e "\ttar -xzvf riscv-toolchain-14-aarch64-lin.tar.gz -C \$PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2"
	echo ""
	echo "To build for riscv:"
	echo ""
	echo -e "\t./bld.sh -c <hwconfig> -r -t \$PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2/bin"
	echo ""
} 

PICO_PLATFORM=rp2040
BUILD=RELEASE
HWCONFIG=1
UF2="${APP}PimoroniDV.uf2"
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
SDKVERSION=`cat $PICO_SDK_PATH/pico_sdk_version.cmake | grep "set(PICO_SDK_VERSION_MAJOR" | cut -f2  -d" " | cut -f1 -d\)`
TOOLCHAIN_PATH=
picoarmIsSet=0
picoRiscIsSet=0
while getopts "hd2rc:t:" opt; do
  case $opt in
    d)
      BUILD=DEBUG
      ;;
    c)
      HWCONFIG=$OPTARG
      ;;
	2) 
	  PICO_PLATFORM=rp2350-arm-s
	  picoarmIsSet=1
	  ;;
	r)
	  picoriscIsSet=1
	  PICO_PLATFORM=rp2350-riscv
	  ;;	
	t) TOOLCHAIN_PATH=$OPTARG
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
# -2 and -r are mutually exclusive
if [[ $picoarmIsSet -eq 1 && $picoriscIsSet -eq 1 ]] ; then
	echo "Options -2 and -r are mutually exclusive"
	usage
	exit 1
fi	
# TOOLCHAIN_PATH is set, check if it is a valid path
if [ ! -z "$TOOLCHAIN_PATH" ] ; then
	if [ ! -d "$TOOLCHAIN_PATH" ] ; then
		echo "Toolchain path $TOOLCHAIN_PATH not found"
		exit 1
	fi
fi
case $HWCONFIG in
	1)
		UF2="${APP}PimoroniDV.uf2"
		;;
	2)
		UF2="${APP}AdaFruitDVISD.uf2"
		;;
	3) 
		UF2="${APP}FeatherDVI.uf2"
		;;
	4)
		UF2="${APP}WsRP2040PiZero.uf2"
		;;
	*)
		echo "Invalid value: $HWCONFIG specified for option -c"
		usage
		exit 1
		;;
esac

if [ "$PICO_PLATFORM" = "rp2350-arm-s" ] ; then
	UF2="pico2_$UF2"
fi	
if [ "$PICO_PLATFORM" = "rp2350-riscv" ] ; then
	UF2="pico2_riscv_$UF2"
fi	
echo "Building $PROJECT"
echo "Using Pico SDK version: $SDKVERSION"
echo "Building for $PICO_BOARD with $BUILD configuration and HWCONFIG=$HWCONFIG"
echo "Toolchain path: $TOOLCHAIN_PATH"
echo "UF2 file: $UF2"

# if PICO_PLATFORM starts with rp2350, check if the SDK version is 2 or higher
if [[ $SDKVERSION -lt 2 && $PICO_PLATFORM == rp2350* ]] ; then
		echo "Pico SDK version $SDKVERSION does not support RP2350 (pico2). Please update the SDK to version 2 or higher"
		echo ""
		exit 1
fi
# pico2 board not compatible with HWCONFIG > 2
if [[ $HWCONFIG -gt 2 && $PICO_PLATFORM == rp2350* ]] ; then
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
if [ -z "$TOOLCHAIN_PATH" ] ; then
	cmake -DCMAKE_BUILD_TYPE=$BUILD -DHW_CONFIG=$HWCONFIG -DPICO_PLATFORM=$PICO_PLATFORM ..
else
	cmake -DCMAKE_BUILD_TYPE=$BUILD -DHW_CONFIG=$HWCONFIG -DPICO_PLATFORM=$PICO_PLATFORM -DPICO_TOOLCHAIN_PATH=$TOOLCHAIN_PATH ..
fi
make -j 4
cd ..
echo ""
if [ -f build/${APP}.uf2 ] ; then
	cp build/${APP}.uf2 releases/${UF2} || exit 1
fi

