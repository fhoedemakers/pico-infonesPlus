#include "gamepads.h"

struct PSCReport {
  uint8_t unknow1;
  uint8_t report_id;
  uint8_t buttons1;
  uint8_t buttons2;
  uint8_t unknow[16];

  struct Button1 {
    inline static constexpr int UP = 0x01;
    inline static constexpr int DOWN = 0x02;
    inline static constexpr int LEFT = 0x04;
    inline static constexpr int RIGHT = 0x08;
    inline static constexpr int SELECT = 0x20;
    inline static constexpr int START = 0x10;
  };

  struct Button2 {
    inline static constexpr int L = 0x01;
    inline static constexpr int R = 0x02;
    inline static constexpr int A = 0x20;
    inline static constexpr int B = 0x10;
    inline static constexpr int X = 0x80;
    inline static constexpr int Y = 0x40;
    inline static constexpr int START_SELECT = 0x04;
  };
};

bool psclassic_open(uint16_t vid, uint16_t pid)
{
  return (vid == 0x054c && pid == 0x0cda) || (vid == 0x045e && pid == 0x028e);
}

void psclassic_read(uint8_t dev_addr, uint8_t const *report, uint16_t len)
{
  // printf("PSclassic: len = %d\n", len);
  // for(uint32_t i=0; i<len; i++)
  // {
  //  if (i%16 == 0) printf("\r\n  ");
  //  printf("%02X ", report[i]);
  // }
  // printf("\r\n");

  uint32_t curr_data = 0;
  static uint32_t prev_data[2]={0};

  if (sizeof(PSCReport) <= len)
  {
    auto r = reinterpret_cast<const PSCReport *>(report);
    if (r->report_id != 0x14)
    {
      printf("Invalid report_id %d\n", r->report_id);
      return;
    }

    curr_data =
      (r->buttons1 & PSCReport::Button1::UP ? io::GamePadState::Button::UP : 0) |
      (r->buttons1 & PSCReport::Button1::DOWN ? io::GamePadState::Button::DOWN : 0) |
      (r->buttons1 & PSCReport::Button1::LEFT ? io::GamePadState::Button::LEFT : 0) |
      (r->buttons1 & PSCReport::Button1::RIGHT ? io::GamePadState::Button::RIGHT : 0) |
      (r->buttons1 & PSCReport::Button1::SELECT ? io::GamePadState::Button::SELECT : 0) |
      (r->buttons1 & PSCReport::Button1::START ? io::GamePadState::Button::START : 0) |
      (r->buttons2 & PSCReport::Button2::A ? io::GamePadState::Button::A : 0) |
      (r->buttons2 & PSCReport::Button2::B ? io::GamePadState::Button::B : 0) |
      (r->buttons2 & PSCReport::Button2::X ? io::GamePadState::Button::X : 0) |
      (r->buttons2 & PSCReport::Button2::Y ? io::GamePadState::Button::Y : 0) |
      (r->buttons2 & PSCReport::Button2::START_SELECT ? io::GamePadState::Button::START | io::GamePadState::Button::SELECT : 0);

    // Like for nespad controller we set pad only once when data change.
    if (curr_data != prev_data[(dev_addr-1) & 1]){
      prev_data[(dev_addr-1) & 1] = curr_data;
      auto &gp = io::getCurrentGamePadState((dev_addr-1) & 1);
      gp.buttons = curr_data;
      // printf("psclassic_read gp.buttons pad: %d: %d -> 0x%x\n", (dev_addr-1 & 1), gp.buttons, gp.buttons);
    }

  } else {
    printf("Invalid PSclassic report size %zd\n", len);
    return;
  }
}
