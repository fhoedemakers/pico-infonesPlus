#include <cstdio>
#include "gamepad.h"
#include "tusb_config.h"

bool ds4_open(uint16_t vid, uint16_t pid);
void ds4_read(uint8_t dev_addr, uint8_t const *report, uint16_t len);

bool ds5_open(uint16_t vid, uint16_t pid);
void ds5_read(uint8_t dev_addr, uint8_t const *report, uint16_t len);

bool joystick_open(uint16_t vid, uint16_t pid);
void joystick_read(uint8_t dev_addr, uint8_t const *report, uint16_t len);

bool keyboard_open(uint16_t vid, uint16_t pid);
void keyboard_read(uint8_t dev_addr, uint8_t const *report, uint16_t len);

bool nintendo_open(uint16_t vid, uint16_t pid);
void nintendo_read(uint8_t dev_addr, uint8_t const *report, uint16_t len);

bool psclassic_open(uint16_t vid, uint16_t pid);
void psclassic_read(uint8_t dev_addr, uint8_t const *report, uint16_t len);

void nespad_init(void);
void nespad_read(void);
void nespad_close(void);

void wiipad_init(void);
void wiipad_read(void);
void wiipad_close(void);

void bluepad_init(void);
void bluepad_read(void);
void bluepad_close(void);

typedef struct {
  bool (* const open)(uint16_t vid, uint16_t pid);
  void (* const read)(uint8_t dev_addr, uint8_t const *report, uint16_t len);
  //void (* const close )(uint8_t dev_addr);
} gampads_usb_driver_t;

typedef struct {
  void (* const init)(void);
  void (* const read)(void);
  void (* const close)(void);
} gampads_platform_driver_t;

// Check below url for more controlers types:
// https://github.com/libsdl-org/SDL/blob/57c3b2c95089600d4e1cdbbfb58ffd6ba84ca402/src/joystick/controller_type.c
const gampads_usb_driver_t* gamepads_attach(uint16_t vid, uint16_t pid);

void gamepads_init(void);
void gamepads_read(void);
void gamepads_close(void);

extern const gampads_usb_driver_t* drv2pad[CFG_TUH_DEVICE_MAX];

void nespad_read_start(void);