add_library(infones INTERFACE)

target_sources(infones
INTERFACE
    InfoNES_Mapper.cpp
    InfoNES_pAPU.cpp
    InfoNES.cpp
    InfoNES_Region.cpp
    InfoNES_FDS.cpp
    InfoNES_NSF.cpp
    K6502.cpp
)

# target_include_directories(infones 
# INTERFACE
# )

# target_link_libraries(infones 
# INTERFACE
# )
