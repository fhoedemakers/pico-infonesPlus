#include "hardware/pio.h"

#define nespad_wrap_target 0
#define nespad_wrap 6

static const uint16_t nespad_program_instructions[] = {
    //     .wrap_target
    0xc020, //  0: irq    wait 0          side 0
    0xea01, //  1: set    pins, 1         side 0 [10]
    0xe027, //  2: set    x, 7            side 0
    0xe000, //  3: set    pins, 0         side 0
    0x4401, //  4: in     pins, 1         side 0 [4]
    0xf500, //  5: set    pins, 0         side 1 [5]
    0x0044, //  6: jmp    x--, 4          side 0
            //     .wrap
};

static const struct pio_program nespad_program = {
    .instructions = nespad_program_instructions,
    .length = 7,
    .origin = -1,
};

static inline pio_sm_config nespad_program_get_default_config(uint offset) {
  pio_sm_config c = pio_get_default_sm_config();
  sm_config_set_wrap(&c, offset + nespad_wrap_target, offset + nespad_wrap);
  sm_config_set_sideset(&c, 1, false, false);
  return c;
}

static PIO pio = pio0;
static uint8_t sm = -1;
uint8_t nespad_state = 0;

bool nespad_begin(uint32_t cpu_khz, uint8_t clkPin, uint8_t dataPin,
                  uint8_t latPin) {
  if (pio_can_add_program(pio, &nespad_program) &&
      ((sm = pio_claim_unused_sm(pio, true)) >= 0)) {
    uint offset = pio_add_program(pio, &nespad_program);
    pio_sm_config c = nespad_program_get_default_config(offset);

    sm_config_set_sideset_pins(&c, clkPin);
    sm_config_set_in_pins(&c, dataPin);
    sm_config_set_set_pins(&c, latPin, 1);
    pio_gpio_init(pio, clkPin);
    pio_gpio_init(pio, dataPin);
    pio_gpio_init(pio, latPin);
    gpio_set_pulls(dataPin, true, false); // Pull data high, 0xFF if unplugged
    pio_sm_set_pindirs_with_mask(pio, sm,
                                 (1 << clkPin) | (1 << latPin), // Outputs
                                 (1 << clkPin) | (1 << dataPin) |
                                     (1 << latPin)); // All pins
    sm_config_set_in_shift(&c, true, true, 8); // R shift, autopush @ 8 bits

    sm_config_set_clkdiv_int_frac(&c, cpu_khz / 1000, 0); // 1 MHz clock

    // IRQ is just a 'go' flag, don't assert a system interrupt
    pio_set_irq0_source_enabled(
        pio, (pio_interrupt_source)(pis_interrupt0 + sm), false);

    pio_sm_clear_fifos(pio, sm);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    return true; // Success
  }
  return false;
}

// Initiate nespad read. Non-blocking; result will be available in ~100 uS
// via nespad_read_finish(). Must first call nespad_begin() once to set up PIO.
void nespad_read_start(void) { pio_interrupt_clear(pio, 0); }

// Finish nespad read. Ideally should be called ~100 uS after
// nespad_read_start(), but can be sooner (will block until ready), or later
// (will introduce latency). Sets value of global nespad_state variable, a
// bitmask of button/D-pad state (1 = pressed). 0x01=Right, 0x02=Left,
// 0x04=Down, 0x08=Up, 0x10=Start, 0x20=Select, 0x40=B, 0x80=A. Must first
// call nespad_begin() once to set up PIO. Result will be 0 if PIO failed to
// init (e.g. no free state machine).
void nespad_read_finish(void) {
  // Right-shift was used in sm config so bit order matches NES controller
  // bits used elsewhere in picones, but does require shifting down...
  nespad_state = (sm >= 0) ? ((pio_sm_get_blocking(pio, sm) >> 24) ^ 0xFF) : 0;
}
