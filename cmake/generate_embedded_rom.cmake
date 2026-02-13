# Generate embedded_rom.c from a NES ROM file using xxd.
# Called via: cmake -DINPUT_FILE=<rom> -DOUTPUT_FILE=<output.c> -P generate_embedded_rom.cmake

file(SIZE "${INPUT_FILE}" ROM_SIZE)

execute_process(
    COMMAND xxd -i "${INPUT_FILE}"
    OUTPUT_VARIABLE XXD_OUTPUT
    RESULT_VARIABLE XXD_RESULT
)
if (NOT XXD_RESULT EQUAL 0)
    message(FATAL_ERROR "xxd failed on ${INPUT_FILE}")
endif()

# Replace the xxd-generated variable names with our fixed names
string(REGEX REPLACE "unsigned char [^][]* *\\[" "__attribute__((aligned(4))) const unsigned char embedded_nes_rom[" XXD_OUTPUT "${XXD_OUTPUT}")
string(REGEX REPLACE "unsigned int [^ ]* =" "const unsigned int embedded_nes_rom_len =" XXD_OUTPUT "${XXD_OUTPUT}")

file(WRITE "${OUTPUT_FILE}" "${XXD_OUTPUT}")
