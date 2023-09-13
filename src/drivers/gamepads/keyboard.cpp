#include "gamepads.h"
#include "class/hid/hid.h"

// Regular HID keyboard driver.
bool keyboard_open(uint16_t vid, uint16_t pid)
{
  (void)(vid);
  (void)(pid);
  // return (vid == 0x062a && pid == 0x4101) || (vid == 0x046d && pid == 0xc31c);
  return false;
}

void keyboard_read(uint8_t dev_addr, uint8_t const *report, uint16_t len)
{
  (void)(len);
  uint8_t const *keycode = &report[2];
  auto &gp = io::getCurrentGamePadState((dev_addr-1) & 1);
  gp.buttons = 0;
  for(uint8_t i=0; i<6; i++){
    if (keycode[i] > 0){
      gp.buttons |= (keycode[i] == HID_KEY_A ? io::GamePadState::Button::A : 0);
      gp.buttons |= (keycode[i] == HID_KEY_B ? io::GamePadState::Button::B : 0);
      gp.buttons |= (keycode[i] == HID_KEY_X ? io::GamePadState::Button::X : 0);
      gp.buttons |= (keycode[i] == HID_KEY_Y ? io::GamePadState::Button::Y : 0);
      gp.buttons |= (keycode[i] == HID_KEY_ARROW_LEFT ? io::GamePadState::Button::LEFT : 0);
      gp.buttons |= (keycode[i] == HID_KEY_ARROW_RIGHT ? io::GamePadState::Button::RIGHT : 0);
      gp.buttons |= (keycode[i] == HID_KEY_ARROW_UP ? io::GamePadState::Button::UP : 0);
      gp.buttons |= (keycode[i] == HID_KEY_ARROW_DOWN ? io::GamePadState::Button::DOWN : 0);
      gp.buttons |= (keycode[i] == HID_KEY_ENTER ? io::GamePadState::Button::START : 0);
      gp.buttons |= (keycode[i] == HID_KEY_SPACE ? io::GamePadState::Button::SELECT : 0);
      // printf("keycode: %d -> 0x%x | after map: 0x%x\n", keycode[i], keycode[i], gp.buttons);
    }
  }
}
