#include "gamepads.h"

struct DS5Report {

  uint8_t reportID;
  uint8_t stickL[2];
  uint8_t stickR[2];
  uint8_t triggerL;
  uint8_t triggerR;
  uint8_t counter;
  uint8_t buttons[3];

  struct Button {
    inline static constexpr int SQUARE = 1 << 4;
    inline static constexpr int CROSS = 1 << 5;
    inline static constexpr int CIRCLE = 1 << 6;
    inline static constexpr int TRIANGLE = 1 << 7;
    inline static constexpr int L1 = 1 << 8;
    inline static constexpr int R1 = 1 << 9;
    inline static constexpr int L2 = 1 << 10;
    inline static constexpr int R2 = 1 << 11;
    inline static constexpr int SHARE = 1 << 12;
    inline static constexpr int OPTIONS = 1 << 13;
    inline static constexpr int L3 = 1 << 14;
    inline static constexpr int R3 = 1 << 15;
    inline static constexpr int PS = 1 << 16;
    inline static constexpr int TPAD = 1 << 17;
  };

  int getHat() const { return buttons[0] & 15; }
};

bool ds5_open(uint16_t vid, uint16_t pid)
{
  return vid == 0x054c && pid == 0x0ce6;
}

void ds5_read(uint8_t dev_addr, uint8_t const *report, uint16_t len)
{
  if (sizeof(DS5Report) <= len)
  {
    auto r = reinterpret_cast<const DS5Report *>(report);
    if (r->reportID != 1)
    {
      printf("Invalid reportID %d\n", r->reportID);
      return;
    }

    auto buttons = r->buttons[0] | (r->buttons[1] << 8) | (r->buttons[2] << 16);

    auto &gp = io::getCurrentGamePadState((dev_addr-1) & 1);
    gp.axis[0] = r->stickL[0];
    gp.axis[1] = r->stickL[1];
    gp.buttons =
      (buttons & DS5Report::Button::CROSS ? io::GamePadState::Button::B : 0) |
      (buttons & DS5Report::Button::CIRCLE ? io::GamePadState::Button::A : 0) |
      (buttons & DS5Report::Button::TRIANGLE ? io::GamePadState::Button::X : 0) |
      (buttons & DS5Report::Button::SQUARE ? io::GamePadState::Button::Y : 0) |
      (buttons & (DS5Report::Button::SHARE | DS5Report::Button::TPAD) ? io::GamePadState::Button::SELECT : 0) |
      (buttons & DS5Report::Button::OPTIONS ? io::GamePadState::Button::START : 0);
    gp.hat = static_cast<io::GamePadState::Hat>(r->getHat());
    gp.convertButtonsFromAxis(0, 1);
    gp.convertButtonsFromHat();
  } else {
    printf("Invalid DS5 report size %zd\n", len);
    return;
  }
}
