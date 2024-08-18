#pragma once

#include "hardware/pio.h"

//extern uint8_t nespad_state;
extern uint8_t nespad_states[2];
extern bool nespad_begin(uint8_t padnum, uint32_t cpu_khz, uint8_t clkPin, uint8_t dataPin,
                         uint8_t latPin, PIO _pio);
extern void nespad_read_start(void);
extern void nespad_read_finish(void);
