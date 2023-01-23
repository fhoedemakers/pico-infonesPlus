#pragma once

extern uint8_t nespad_state;

extern bool nespad_begin(uint32_t cpu_khz, uint8_t clkPin, uint8_t dataPin,
                         uint8_t latPin);
extern void nespad_read_start(void);
extern void nespad_read_finish(void);
