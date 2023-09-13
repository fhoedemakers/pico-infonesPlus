/*
 * author : Shuichi TAKANO
 * since  : Thu Jul 29 2021 03:23:36
 */
#ifndef _95F3C883_0134_6410_3159_CA2D9FE7D961
#define _95F3C883_0134_6410_3159_CA2D9FE7D961

#ifdef __cplusplus
extern "C"
{
#endif

#define CFG_TUSB_RHPORT0_MODE OPT_MODE_HOST

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

#define CFG_TUH_ENUMERATION_BUFSZIE 256

#define CFG_TUH_HUB 1
#define CFG_TUH_CDC 0
#define CFG_TUH_HID 4 // When set to 0 pico hang when usb hub is plugged.
#define CFG_TUH_MSC 0
#define CFG_TUH_VENDOR 0
#define CFG_TUSB_HOST_DEVICE_MAX (4 + 1)
#define CFG_TUH_HID_EP_BUFSIZE 64

#define CFG_TUH_DEVICE_MAX 8
#define CFG_TUH_API_EDPT_XFER 1

// #define TUSB_OPT_MUTEX 1

#ifdef __cplusplus
}
#endif

#endif /* _95F3C883_0134_6410_3159_CA2D9FE7D961 */
