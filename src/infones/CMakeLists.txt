add_library(infones INTERFACE)

target_sources(infones
INTERFACE
    InfoNES_Mapper.cpp
    InfoNES_pAPU.cpp
    InfoNES.cpp
    K6502.cpp
)

# target_include_directories(infones 
# INTERFACE
# )

# target_link_libraries(infones 
# INTERFACE
# )
