#include "gamepads.h"

bool nintendo_open(uint16_t vid, uint16_t pid)
{
  return vid == 0x057e && ( pid == 0x2009 || pid == 0x2017);
}

void nintendo_read(uint8_t dev_addr, uint8_t const *report, uint16_t len)
{
  (void)(report);
  printf("Nintendo report from device %d: len = %d\n", dev_addr, len);
}
