add_library(infones INTERFACE)

target_sources(infones
INTERFACE
    InfoNES_Mapper.cpp
    InfoNES_pAPU.cpp
    InfoNES.cpp
    InfoNES_Region.cpp
    K6502.cpp
)

# emu2413 (VRC7 OPLL) is only compiled on RP2350 — its lookup tables
# and the OPLL state want PSRAM-backed allocations and the ~30 KB of
# code is dead weight on RP2040 (Mapper 85 itself is gated the same).
if (PICO_PLATFORM STREQUAL "rp2350-arm-s" OR PICO_PLATFORM STREQUAL "rp2350-riscv")
    target_sources(infones INTERFACE emu2413.c)
endif()

# target_include_directories(infones 
# INTERFACE
# )

# target_link_libraries(infones 
# INTERFACE
# )
