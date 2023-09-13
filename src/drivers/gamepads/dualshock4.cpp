#include "gamepads.h"

// https://www.psdevwiki.com/ps4/DS4-USB
struct DS4Report {

    struct Button1 {
      inline static constexpr int SQUARE = 1 << 4;
      inline static constexpr int CROSS = 1 << 5;
      inline static constexpr int CIRCLE = 1 << 6;
      inline static constexpr int TRIANGLE = 1 << 7;
    };

    struct Button2 {
      inline static constexpr int L1 = 1 << 0;
      inline static constexpr int R1 = 1 << 1;
      inline static constexpr int L2 = 1 << 2;
      inline static constexpr int R2 = 1 << 3;
      inline static constexpr int SHARE = 1 << 4;
      inline static constexpr int OPTIONS = 1 << 5;
      inline static constexpr int L3 = 1 << 6;
      inline static constexpr int R3 = 1 << 7;
    };

    uint8_t reportID;
    uint8_t stickL[2];
    uint8_t stickR[2];
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t ps : 1;
    uint8_t tpad : 1;
    uint8_t counter : 6;
    uint8_t triggerL;
    uint8_t triggerR;

    int getHat() const { return buttons1 & 15; }
};

// bool ds4_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *desc_itf, uint16_t max_len){
bool ds4_open(uint16_t vid, uint16_t pid)
{
  return vid == 0x054c && (pid == 0x09cc || pid == 0x05c4);
}

void ds4_read(uint8_t dev_addr, uint8_t const *report, uint16_t len)
{

  if (sizeof(DS4Report) <= len)
  {
    auto r = reinterpret_cast<const DS4Report *>(report);
    if (r->reportID != 1)
    {
        printf("Invalid reportID %d\n", r->reportID);
        return;
    }

    auto &gp = io::getCurrentGamePadState((dev_addr-1) & 1);
    gp.axis[0] = r->stickL[0];
    gp.axis[1] = r->stickL[1];
    gp.buttons =
      (r->buttons1 & DS4Report::Button1::CROSS ? io::GamePadState::Button::B : 0) |
      (r->buttons1 & DS4Report::Button1::CIRCLE ? io::GamePadState::Button::A : 0) |
      (r->buttons1 & DS4Report::Button1::TRIANGLE ? io::GamePadState::Button::X : 0) |
      (r->buttons1 & DS4Report::Button1::SQUARE ? io::GamePadState::Button::Y : 0) |
      (r->buttons2 & DS4Report::Button2::SHARE ? io::GamePadState::Button::SELECT : 0) |
      (r->tpad ? io::GamePadState::Button::SELECT : 0) |
      (r->buttons2 & DS4Report::Button2::OPTIONS ? io::GamePadState::Button::START : 0);
    gp.hat = static_cast<io::GamePadState::Hat>(r->getHat());
    gp.convertButtonsFromAxis(0, 1);
    gp.convertButtonsFromHat();
  } else {
    printf("Invalid DS4 report size %zd\n", len);
    return;
  }
}
