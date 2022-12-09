# pico-infonesPlus 
cmake_minimum_required(VERSION 3.13)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add new build type RELEASENODEBUG
# usage
# cmake -DCMAKE_BUILD_TYPE=RELEASENODEBUG ..
message("* Adding build types...")
SET(GCC_RELEASENODEBUG_FLAGS "-O2 -DNDEBUG")
SET(CMAKE_ASM_FLAGS_RELEASENODEBUG
    "${GCC_RELEASENODEBUG_FLAGS}"
    CACHE STRING "Flags used by the ASM compiler during REL_O2 builds."
    FORCE )

SET(CMAKE_CXX_FLAGS_RELEASENODEBUG
    "${GCC_RELEASENODEBUG_FLAGS}"
    CACHE STRING "Flags used by the C++ compiler during REL_O2 builds."
    FORCE )
SET(CMAKE_C_FLAGS_RELEASENODEBUG
    "${GCC_RELEASENODEBUG_FLAGS}"
    CACHE STRING "Flags used by the C compiler during REL_O2 builds."
    FORCE )
SET(CMAKE_EXE_LINKER_FLAGS_RELEASENODEBUG
    ""
    CACHE STRING "Flags used for linking binaries during REL_O2 builds."
    FORCE )
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASENODEBUG
    ""
    CACHE STRING "Flags used by the shared libraries linker during REL_O2 builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_RELEASENODEBUG
    CMAKE_ASM_FLAGS_RELEASENODEBUG
    CMAKE_C_FLAGS_RELEASENODEBUG
    CMAKE_EXE_LINKER_FLAGS_RELEASENODEBUG
    CMAKE_SHARED_LINKER_FLAGS_RELEASENODEBUG )

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug
       CACHE STRING "Choose the type of build : None Debug Release RelWithDebInfo MinSizeRel REL_O2."
       FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
message("* Current build type is : ${CMAKE_BUILD_TYPE}")

add_definitions( -DNDEBUG )
include(pico_sdk_import.cmake)

project(piconesPlus C CXX ASM)
pico_sdk_init()

add_subdirectory(drivers/fatfs)
add_subdirectory(drivers/sdcard)

set(DVICONFIG "dviConfig_PimoroniDemoDVSock" CACHE STRING
	"Select a default pin configuration from common_dvi_pin_configs.h")
set(SD_CS "22" CACHE STRING "Specify the Chip Select GPIO pin for the SD card")
set(SD_SCK "5" CACHE STRING "Specify de Clock GPIO pin for the SD card")
set(SD_MOSI "18" CACHE STRING "Select the Master Out Slave In GPIO pin for the SD card")
set(SD_MISO "19" CACHE STRING "Select the Master In Slave Out GPIO pin for the SD card")

message("HDMI board type : ${DVICONFIG}")
message("SD card CS      : ${SD_CS}")
message("SD card SCK     : ${SD_SCK}")
message("SD card MOSI    : ${SD_MOSI}")
message("SD card MISO    : ${SD_MISO}")


add_executable(piconesPlus
    main.cpp
    hid_app.cpp
    gamepad.cpp
    tar.cpp
    menu.cpp
    RomLister.cpp
    FrensHelpers.cpp
)

pico_set_program_name(piconesPlus "piconesPlus")
pico_set_program_version(piconesPlus "0.3")

pico_enable_stdio_uart(piconesPlus 1)
pico_enable_stdio_usb(piconesPlus 0)

# tinyusb
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_DEBUG_LEVE 0)
include(${PICO_TINYUSB_PATH}/hw/bsp/${FAMILY}/family.cmake)
family_configure_target(piconesPlus)

target_compile_definitions(piconesPlus PRIVATE
    DVICONFIG=${DVICONFIG}
    SDCARD_PIN_SPI0_CS=${SD_CS}
	SDCARD_PIN_SPI0_SCK=${SD_SCK}
	SDCARD_PIN_SPI0_MOSI=${SD_MOSI}
	SDCARD_PIN_SPI0_MISO=${SD_MISO}
	SDCARD_PIO=pio1
	SDCARD_PIO_SM=0
)
target_link_libraries(piconesPlus PRIVATE
    pico_stdlib
    pico_multicore
    hardware_dma
    hardware_pio
    hardware_i2c
    hardware_interp
    hardware_timer
    hardware_clocks
    hardware_pwm
    hardware_flash
    dvi
    util
    infones
    tinyusb_host
    sdcard
    fatfs        
)

    target_include_directories(piconesPlus
    PRIVATE
        pico_lib
        infones
        assets
        include
        .
    )

    add_subdirectory(pico_lib/dvi)
    add_subdirectory(pico_lib/util)
    add_subdirectory(infones)

    pico_add_extra_outputs(piconesPlus)



