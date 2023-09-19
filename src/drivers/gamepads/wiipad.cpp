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

// According http://wiibrew.org/wiki/Wiimote/Extension_Controllers
// there are two way to initialize the extension (pads in our case).
//
// Default is "The New Way", you can try to use both way at the same time.
#define WII_USE_OLD_WAY 0
#define WII_USE_NEW_WAY 1

// If you have a psclassic controller and use just "The New Way" inicialization sequence,
// sometimes it works, sometimes not (but now seams to be stable).
// This depends on used initialization mode or how often we call wiipad_ext_init().
//
// In that case you can use "The Old Way" initialization, or set
// WII_BUFSIZE to 8 instead 6 and read data from last two bytes.
//
// I'm not sure though if reading 8 bytes are always the best option for every wii pads,
// as I own only psclassic controllers.
#define WII_BUFSIZE 6

void wiipad_ext_init(void) {
    // Do not flood i2c bus, instead probe it every ~100 calls of this method.
    // This also prevent latency when pads are disconnected in main loop.
    static uint8_t brake = 0;
    if (brake++ < 100){
        return;
    }
    brake=0;

// "The Old Way" initialization.
#if WII_USE_OLD_WAY == 1
    uint8_t init0[] = { 0x40, 0x00 };
    if (i2c_write_timeout_us(WII_I2C, WII_ADDR, init0, 2, false, 400) != sizeof init0){
        // printf("Wiipad driver initialization failed on init0.\n");
        return;
    }
    sleep_us(400);
#endif

// "The New Way" initialization.
#if WII_USE_NEW_WAY == 1
    uint8_t init1[] = { 0xF0, 0x55 };
    uint8_t init2[] = { 0xFB, 0x00 };
    if (i2c_write_timeout_us(WII_I2C, WII_ADDR, init1, 2, false, 400) != sizeof init1){
        // printf("Wiipad driver initialization failed on init1.\n");
        return;
    }
    sleep_us(400);
    if (i2c_write_timeout_us(WII_I2C, WII_ADDR, init2, 2, false, 400) != sizeof init2){
        // printf("Wiipad driver initialization failed on init2.\n");
        return;
    }
#endif

}

void wiipad_init(void) {
    i2c_init(WII_I2C, WII_BAUDRATE);
    gpio_set_function(WII_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(WII_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(WII_PIN_SDA);
    gpio_pull_up(WII_PIN_SCL);
    wiipad_ext_init();
    printf("Wiipad driver initialized.\n");
}

// TODO:
//   Add support for second pad.
void wiipad_read(void) {
    uint32_t curr_data = 0;
    static uint32_t prev_data = 0;

    uint8_t req[1] = { 0x00 };
    uint8_t buf[WII_BUFSIZE] = { 0x00 };

    // Check if device is still present on bus and send required byte to read state,
    // if fail then problably pad is disconnected and we should try to reinit it.
    if (i2c_write_timeout_us(WII_I2C, WII_ADDR, req, sizeof req, false, 400) != sizeof req){
        wiipad_ext_init();
        return;
    }

    sleep_us(200);

    if (i2c_read_timeout_us(WII_I2C, WII_ADDR, buf, sizeof buf, false, 1000) == sizeof buf){
        // Don't allow to set those two bytes to 0x00, otherwise
        // all below bytes in curr_data are set and this lead to mess with logic in main.cpp.
        //
        // In short all bytes in `v/pushed` variables in main.cpp are set and picones back
        // to main menu with FPS and firemask enabled.
        if ( buf[4] == 0 && buf[5] == 0 ){
            buf[4] = buf[6] != 0 ? buf[6] : 0xff;
            buf[5] = buf[7] != 0 ? buf[7] : 0xff;
        }

        // TODO:
        //   Refactor this as unfortunate there might be more not allowed values
        //   in buffer that will set all bytes by using below logic.
        curr_data =
            ((!(buf[5] & 0x01)) ? io::GamePadState::Button::UP : 0) |
            ((!(buf[4] & 0x40)) ? io::GamePadState::Button::DOWN : 0) |
            ((!(buf[5] & 0x02)) ? io::GamePadState::Button::LEFT : 0) |
            ((!(buf[4] & 0x80)) ? io::GamePadState::Button::RIGHT : 0) |
            ((!(buf[4] & 0x10)) ? io::GamePadState::Button::SELECT : 0) |
            ((!(buf[4] & 0x04)) ? io::GamePadState::Button::START  : 0) |
            ((!(buf[5] & 0x10)) ? io::GamePadState::Button::A : 0) |
            ((!(buf[5] & 0x40)) ? io::GamePadState::Button::B : 0);

        // Check if we have illegal state, and discard this read cycle when happen.
        if (curr_data == (
            io::GamePadState::Button::UP |
            io::GamePadState::Button::DOWN |
            io::GamePadState::Button::LEFT |
            io::GamePadState::Button::RIGHT |
            io::GamePadState::Button::SELECT|
            io::GamePadState::Button::START|
            io::GamePadState::Button::A|
            io::GamePadState::Button::B)){
            return;
        }

        auto &gp = io::getCurrentGamePadState(0);
        // Only set once gb.buttons if new data arrive in this driver.
        // This logic is needed to avoid clearing gp.buttons value that
        // can be already set by other gamepad in the system.
        if (prev_data != curr_data){
            prev_data = curr_data;
            gp.buttons = curr_data;

            // for(uint8_t i=0; i<sizeof buf; i++)
            // {
            //     if (i%16 == 0) printf("\r\n  ");
            //     printf(" %02X:%ld ", buf[i], buf[i]);
            // }
            // printf("\r\n");
            // printf("UP     is %s\n", (curr_data & io::GamePadState::Button::UP) ? "on" : "off");
            // printf("DOWN   is %s\n", (curr_data & io::GamePadState::Button::DOWN) ? "on" : "off");
            // printf("LEFT   is %s\n", (curr_data & io::GamePadState::Button::LEFT) ? "on" : "off");
            // printf("RIGHT  is %s\n", (curr_data & io::GamePadState::Button::RIGHT) ? "on" : "off");
            // printf("SELECT is %s\n", (curr_data & io::GamePadState::Button::SELECT) ? "on" : "off");
            // printf("START  is %s\n", (curr_data & io::GamePadState::Button::START) ? "on" : "off");
            // printf("A      is %s\n", (curr_data & io::GamePadState::Button::A) ? "on" : "off");
            // printf("B      is %s\n", (curr_data & io::GamePadState::Button::B) ? "on" : "off");
        }
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
