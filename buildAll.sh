:
# ====================================================================================
# PICO-INFONESPLUS build all script 
# Builds the emulator for tall the supported hardware configurations in RELEASE configuration
# Binaries are copied to the releases folder
# ====================================================================================
cd `dirname $0` || exit 1
[ -d releases ] && rm -rf releases
mkdir releases || exit 1
# check picotool exists in path
if ! command -v picotool &> /dev/null
then
	echo "picotool could not be found"
	echo "Please install picotool from https://github.com/raspberrypi/picotool.git" 
	exit
fi
# build for Pico
HWCONFIGS="1 2 3 4"
for HWCONFIG in $HWCONFIGS
do	
	./bld.sh -c $HWCONFIG
done
# build for Pico 2 -arm-s
HWCONFIGS="1 2"
for HWCONFIG in $HWCONFIGS
do
	./bld.sh -c $HWCONFIG -2
done
# build for Pico 2 -riscv
HWCONFIGS="1 2"
if [ ! -d $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2/bin ] ; then
	echo "RISC-V toolchain not found in $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2/bin"	
	echo "To install the RISC-V toolchain, execute \"bld.sh -h\" for instructions"
else 
	for HWCONFIG in $HWCONFIGS
	do
		./bld.sh -c $HWCONFIG -r -t $PICO_SDK_PATH/toolchain/RISCV_RPI_2_0_0_2/bin
	done	
fi
if [ -z "$(ls -A releases)" ]; then
	echo "No UF2 files found in releases folder"
	exit
fi
for UF2 in releases/*.uf2
do
	ls -l $UF2
	picotool info $UF2
	echo " "
done
