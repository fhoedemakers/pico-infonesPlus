:
# Force build with 1.4.0 SDk when 1.5.0 or higher is found in PCO_SDK_PATH
function load14sdk() {
	OLDPWD=$PWD
	echo "Installing Pico SDK 1.4.0" 
	export PICO_SDK_PATH=/tmp/$$/pico-sdk-1.4.0
	mkdir -p /tmp/$$
	git clone https://github.com/raspberrypi/pico-sdk.git --branch 1.4.0 $PICO_SDK_PATH
	cd $PICO_SDK_PATH 
	git submodule update --init
	cd $OLDPWD
}
sdkok=1
#echo $PICO_SDK_PATH
if [ "$PICO_SDK_PATH" != "" ] ; then
	if [  -d $PICO_SDK_PATH ] ; then 
		if [ -d "${PICO_SDK_PATH}/lib/btstack" ] ; then
			sdkok=-1
		fi
	else
		sdkok=0
	fi
else
	sdkok=0
fi
if [ $sdkok -eq 0 ] ; then
	echo "PICO_SDK_PATH not set"
	load14sdk
fi
if [ $sdkok -eq -1 ] ; then
	echo "Pico SDK 1.5.0 or higher found"
	echo "Please use SDK version 1.4.0"
	load14sdk
fi
