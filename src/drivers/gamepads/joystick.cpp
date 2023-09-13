#include "gamepads.h"

// Regular HID JoyStick driver.
struct JoyStickReport
{
  uint8_t axis[3];
  uint8_t buttons;
};

bool joystick_open(uint16_t vid, uint16_t pid)
{
  (void)(vid);
  (void)(pid);
  // BUFFALO BGC-FC801
  // return vid == 0x0411 && pid == 0x00c6;
  return false;
}

void joystick_read(uint8_t dev_addr, uint8_t const *report, uint16_t len)
{
  (void)(len);
  auto *rep = reinterpret_cast<const JoyStickReport *>(report);
  // printf("x %d y %d button %02x\n", rep->axis[0], rep->axis[1], rep->buttons);
  auto &gp = io::getCurrentGamePadState((dev_addr-1) & 1);
  gp.axis[0] = rep->axis[0];
  gp.axis[1] = rep->axis[1];
  gp.axis[2] = rep->axis[2];
  gp.buttons = rep->buttons;
  gp.convertButtonsFromAxis(0, 1);
}
