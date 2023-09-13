#include "gamepads.h"

static gampads_platform_driver_t const gampads_platform_drivers[] =
{
  {
    .init  = nespad_init,
    .read  = nespad_read,
    .close = NULL
  },
  #if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0
  {
    .init  = wiipad_init,
    .read  = wiipad_read,
    .close = wiipad_close
  },
  #endif
  {
    .init  = bluepad_init,
    .read  = bluepad_read,
    .close = NULL
  }
};

static gampads_usb_driver_t const gampads_usb_drivers[] =
{
  {
    .open  = ds4_open,
    .read  = ds4_read,
    //.close = ds4_close
  },
  {
    .open  = ds5_open,
    .read  = ds5_read,
    //.close = ds5_close
  },
  {
    .open  = joystick_open,
    .read  = joystick_read,
    //.close = joystick_close
  },
  {
    .open  = keyboard_open,
    .read  = keyboard_read,
    //.close = keyboard_close
  },
  {
    .open  = nintendo_open,
    .read  = nintendo_read,
    //.close = nintendo_close
  },
  {
    .open  = psclassic_open,
    .read  = psclassic_read,
    //.close = psclassic_close
  },
};

enum { GAMEPADS_PLATFORM_DRIVER_COUNT = ( sizeof(gampads_platform_drivers) / sizeof(gampads_platform_drivers[0]) ) };
enum { GAMEPADS_USB_DRIVER_COUNT = ( sizeof(gampads_usb_drivers) / sizeof(gampads_usb_drivers[0]) ) };

const gampads_usb_driver_t* drv2pad[CFG_TUH_DEVICE_MAX];

const gampads_usb_driver_t* gamepads_attach(uint16_t vid, uint16_t pid){
  // Find driver for this device.
  for (uint8_t drv_id = 0; drv_id < GAMEPADS_USB_DRIVER_COUNT; drv_id++)
  {
    gampads_usb_driver_t const * driver = &gampads_usb_drivers[drv_id];
    if ( driver->open(vid, pid) )
    {
      printf("Found gamepad: %x:%x for driver %d.\n", vid, pid, drv_id);
      return driver;
    }
  }
  return NULL;
}

void gamepads_init(void){
  for (uint8_t drv_id = 0; drv_id < GAMEPADS_PLATFORM_DRIVER_COUNT; drv_id++)
  {
    gampads_platform_driver_t const * driver = &gampads_platform_drivers[drv_id];
    if (driver->init){
      driver->init();
    }
  }
}

void gamepads_read(void){
  for (uint8_t drv_id = 0; drv_id < GAMEPADS_PLATFORM_DRIVER_COUNT; drv_id++)
  {
    gampads_platform_driver_t const * driver = &gampads_platform_drivers[drv_id];
    driver->read();
  }
}

void gamepads_close(void){
  for (uint8_t drv_id = 0; drv_id < GAMEPADS_PLATFORM_DRIVER_COUNT; drv_id++)
  {
    gampads_platform_driver_t const * driver = &gampads_platform_drivers[drv_id];
    if (driver->close){
      driver->close();
    }
  }
}

