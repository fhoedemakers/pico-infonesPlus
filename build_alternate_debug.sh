:
# ====================================================================================
# PICO-INFONESPLUS build script with alternate configuration and debug enabled
# Builds the emulator for use with the
# AdaFruit DVI Breakout Board
#         https://www.adafruit.com/product/4984
#         https://learn.adafruit.com/adafruit-dvi-breakout-board
# and the 
# AdaFruit  MicroSD card breakout board+
#         https://www.adafruit.com/product/254 
# ====================================================================================
cd `dirname $0` || exit 1
./bld.sh -c 2 -d