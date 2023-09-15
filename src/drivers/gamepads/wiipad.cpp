#include "gamepads.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Support for Wii Classic Controller and similar devices over I2C
// SHOULD NOT BE USED right now. Although it "works" in the sense
// that the game-select menu can be navigated, program CRASHES HARD
// when copying ROM file to flash. Perhaps RAM constraint or ?

#if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0

#define WII_I2C i2c1
#define WII_BAUDRATE 100000 // 400000
#define WII_ADDR 0x52

void wiipad_init(void) {
    i2c_init(WII_I2C, WII_BAUDRATE);
    gpio_set_function(WII_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(WII_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(WII_PIN_SDA);
    gpio_pull_up(WII_PIN_SCL);
    uint8_t init1[] = { 0xF0, 0x55 };
    uint8_t init2[] = { 0xFB, 0x00 };
    i2c_write_timeout_us(WII_I2C, WII_ADDR, init1, 2, false, 200);
    sleep_ms(100);
    i2c_write_timeout_us(WII_I2C, WII_ADDR, init2, 2, false, 200);
    printf("Wiipad driver initialized.\n");
}

// TODO:
//   Add support for second pad.
void wiipad_read(void) {
    uint32_t curr_data = 0;
    static uint32_t prev_data = 0;

    uint8_t req[] = { 0x00 };
    uint8_t buf[6] = { 0 };

    i2c_write_timeout_us(WII_I2C, WII_ADDR, req, 1, false, 200);
    sleep_us(200);

    if (i2c_read_timeout_us(WII_I2C, WII_ADDR, buf, sizeof buf, false, 800) == sizeof buf) {
        if (!(buf[5] & 0x01)) curr_data |= io::GamePadState::Button::UP;
        if (!(buf[4] & 0x40)) curr_data |= io::GamePadState::Button::DOWN;
        if (!(buf[5] & 0x02)) curr_data |= io::GamePadState::Button::LEFT;
        if (!(buf[4] & 0x80)) curr_data |= io::GamePadState::Button::RIGHT;
        if (!(buf[5] & 0x10)) curr_data |= io::GamePadState::Button::A;
        if (!(buf[5] & 0x40)) curr_data |= io::GamePadState::Button::B;
        if (!(buf[4] & 0x10)) curr_data |= io::GamePadState::Button::SELECT;
        if (!(buf[4] & 0x04)) curr_data |= io::GamePadState::Button::START;
    }

    auto &gp = io::getCurrentGamePadState(0);
    // Only set once gb.buttons if new data arrive in this driver.
    // This logic is needed to avoid clearing gp.buttons value that
    // can be already set by other gamepad in the system.
    if (prev_data != curr_data){
      prev_data = curr_data;
      gp.buttons = curr_data;
    }
}

void wiipad_close(void) {
    i2c_deinit(WII_I2C);
    gpio_set_pulls(WII_PIN_SDA, 0, 0);
    gpio_set_pulls(WII_PIN_SCL, 0, 0);
    gpio_set_function(WII_PIN_SDA, GPIO_FUNC_NULL);
    gpio_set_function(WII_PIN_SCL, GPIO_FUNC_NULL);
    printf("Wiipad driver closed.\n");
}

#endif // end WII_PIN_SDA + WII_PIN_SCL
