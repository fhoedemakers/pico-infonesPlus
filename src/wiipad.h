#pragma once

#define WII_I2C i2c1
#define WII_ADDR 0x52

extern void wiipad_begin(void);
extern uint8_t wiipad_read(void);
extern void wiipad_end(void);
