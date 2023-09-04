#!/bin/bash
set -ue
PICO_SDK_VERSION="1.5.0"

declare -A HW_NAMES=( [1]="PimoroniDV" [2]="AdaFruitDVISD" [3]="FeatherDVI" )

help(){
    echo "Standard build by using your system packages can be called like below:"
    echo
    echo "./build.sh (build|build_by_docker) board_hw_id (0-${#HW_NAMES[@]}) release_type (DEBUG|RELEASE)"
    echo
    echo "./build.sh build     - build for all supported boards without debug enabled."
    echo "./build.sh 0 RELEASE - build for all supported boards without debug enabled."
    for (( hw=1; hw<=${#HW_NAMES[@]}; hw++ )); do
        echo "./build.sh ${hw} DEBUG   - build for ${HW_NAMES[${hw}]} board with debug enabled."
    done
    echo
    echo "Additionaly if you have a docker in your system you can use it as a sandbox in build process:"
    echo
    echo "./build.sh build_by_docker           - build for all supported boards without debug enabled."
    echo "./build.sh build_by_docker 0 RELEASE - build for all supported boards without debug enabled."
    for (( hw=1; hw<=${#HW_NAMES[@]}; hw++ )); do
        echo "./build.sh build_by_docker ${hw} DEBUG   - build for ${HW_NAMES[${hw}]} board with debug enabled."
    done
}

intro(){
    HW_CONFIG=${1}; BUILD_TYPE=${2};
    DEBUG_MSG=""
    [ ${BUILD_TYPE} == "DEBUG" ] && DEBUG_MSG="and debug enabled";
    echo "# ======================================================================================"
    echo "# PICO-INFONESPLUS build script with ${HW_NAMES[${HW_CONFIG}]} configuration ${DEBUG_MSG}"
    echo "# Builds the emulator for use with the:"
    echo "#"
    if [ ${HW_CONFIG} -eq 1 ]; then
    echo "# Pimoroni Pico DV Demo Base"
    echo "#   https://shop.pimoroni.com/products/pimoroni-pico-dv-demo-base?variant=39494203998291"
    elif [ ${HW_CONFIG} -eq 2 ]; then
    echo "# AdaFruit DVI Breakout Board"
    echo "#   https://www.adafruit.com/product/4984"
    echo "#   https://learn.adafruit.com/adafruit-dvi-breakout-board"
    echo "#"
    echo "# AdaFruit MicroSD card breakout board+"
    echo "#   https://www.adafruit.com/product/254"
    elif [ ${HW_CONFIG} -eq 3 ]; then
    echo "# Adafruit Feather RP2040 DVI"
    echo "# and the"
    echo "# Adafruit microSD Card FeatherWing"
    fi
    echo "# ======================================================================================"
    echo
}

checksdk(){
    if [ -z "${PICO_SDK_PATH:-}" ];then
        export PICO_SDK_PATH=`pwd`/pico-sdk-${PICO_SDK_VERSION}
        if [ ! -d "${PICO_SDK_PATH}" ];then
            echo "Missing pico SDK - download one..."
            git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/raspberrypi/pico-sdk.git --branch ${PICO_SDK_VERSION} ${PICO_SDK_PATH}
        fi
    fi
}

build(){
    HW_CONFIG=${1:-"0"}; BUILD_TYPE=${2:-"RELEASE"};
    # If HW_CONFIG set to 0 build in recurrence for all boards:
    if [ "${HW_CONFIG}" -eq 0 ]; then
        for (( hw=1; hw<=${#HW_NAMES[@]}; hw++ )); do
            build ${hw} ${BUILD_TYPE}
        done
        return
    fi
    intro ${HW_CONFIG} ${BUILD_TYPE}
    rm -rf build
    mkdir -p build releases
    checksdk
    cd build \
    && cmake -D CMAKE_BUILD_TYPE=${BUILD_TYPE} -D INFONES_PLUS_HW_CONFIG=${HW_CONFIG} ../src \
    && make -j $(getconf _NPROCESSORS_ONLN) \
    && cd .. \
    && arm-none-eabi-size build/piconesPlus.elf \
    && cp -rp build/piconesPlus.uf2 releases/piconesPlus${HW_NAMES[${HW_CONFIG}]}.uf2 \
    && ls -alh build/piconesPlus.uf2 releases/piconesPlus${HW_NAMES[${HW_CONFIG}]}.uf2
}

build_by_docker(){
    echo "*" > .dockerignore
    cat <<EOF | docker build . -t picones -f -
    FROM ubuntu:22.04
    SHELL ["/bin/bash", "-c"]

    WORKDIR /srv/picones

    RUN apt-get update \
    && apt-get install -y cmake git gcc-arm-none-eabi build-essential python3 vim

    ENV PICO_SDK_PATH /srv/pico-sdk-${PICO_SDK_VERSION}

    RUN git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/raspberrypi/pico-sdk.git --branch ${PICO_SDK_VERSION} \${PICO_SDK_PATH}
EOF
    rm -f .dockerignore
    docker run -it --rm -v `pwd`:/srv/picones -it --user $(id -u):$(id -g) picones:latest /srv/picones/build.sh build ${@}
    echo "All done."
}

# By default display help message:
${1:-help} ${@: 2}
