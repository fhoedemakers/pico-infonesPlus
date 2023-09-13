/*
 * author : Shuichi TAKANO
 * since  : Thu Jul 29 2021 03:39:11
 */

#include <tusb.h>
#include <stdio.h>
#include "gamepad.h"
#include "gamepads.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if CFG_TUH_HID > 0

    namespace
    {
        #define MAX_REPORT 4
        uint8_t _report_count[CFG_TUH_HID];
        tuh_hid_report_info_t _report_info_arr[CFG_TUH_HID][MAX_REPORT];
    }

    void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
    {
        uint16_t vid, pid;
        tuh_vid_pid_get(dev_addr, &vid, &pid);

        printf("HID device address = %d, instance = %d is mounted\n", dev_addr, instance);
        printf("VID = %04x, PID = %04x\r\n", vid, pid);

        const char *protocol_str[] = {"None", "Keyboard", "Mouse"}; // hid_protocol_type_t
        uint8_t const interface_protocol = tuh_hid_interface_protocol(dev_addr, instance);

        // Parse report descriptor with built-in parser
        _report_count[instance] = tuh_hid_parse_report_descriptor(_report_info_arr[instance], MAX_REPORT, desc_report, desc_len);
        printf("HID has %u reports and interface protocol = %d:%s\n", _report_count[instance],
               interface_protocol, protocol_str[interface_protocol]);

        if (!tuh_hid_receive_report(dev_addr, instance))
        {
            printf("Error: cannot request to receive report\r\n");
        }
    }

    void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
    {
        printf("HID device address = %d, instance = %d is unmounted\n", dev_addr, instance);
    }

    // Note:
    //   This method is called twice, first when you press the button and second time when you release it.
    //   This mechanism is used in to clear state of gamepads in called drivers by this method.
    void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
    {
        uint8_t const rpt_count = _report_count[instance];
        tuh_hid_report_info_t *rpt_info_arr = _report_info_arr[instance];
        tuh_hid_report_info_t *rpt_info = NULL;

        // Well known gamepad driver, so call it.
        if (drv2pad[dev_addr]){
            drv2pad[dev_addr]->read(dev_addr, report, len);
        } else {

            if (rpt_count == 1 && rpt_info_arr[0].report_id == 0)
            {
                // Simple report without report ID as 1st byte
                rpt_info = &rpt_info_arr[0];
            }
            else
            {
                // Composite report, 1st byte is report ID, data starts from 2nd byte
                uint8_t const rpt_id = report[0];

                // Find report id in the arrray
                for (uint8_t i = 0; i < rpt_count; i++)
                {
                    if (rpt_id == rpt_info_arr[i].report_id)
                    {
                        rpt_info = &rpt_info_arr[i];
                        break;
                    }
                }

                report++;
                len--;
            }

            if (!rpt_info)
            {
                printf("Couldn't find the report info for this report !\n");
                return;
            }

            // printf("usage %d, %d\n", rpt_info->usage_page, rpt_info->usage);
            if (rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP)
            {
                switch (rpt_info->usage)
                {
                case HID_USAGE_DESKTOP_KEYBOARD:
                    TU_LOG1("HID receive keyboard report\n");
                    keyboard_read(dev_addr, report, len);
                break;

                case HID_USAGE_DESKTOP_MOUSE:
                    TU_LOG1("HID receive mouse report\n");
                break;

                case HID_USAGE_DESKTOP_JOYSTICK:
                    TU_LOG1("HID receive joystick report\n");
                    joystick_read(dev_addr, report, len);
                break;

                case HID_USAGE_DESKTOP_GAMEPAD:
                    TU_LOG1("HID receive gamepad report\n");
                break;

                default:
                    break;
                }
            }
        }

        if (!tuh_hid_receive_report(dev_addr, instance))
        {
            printf("Error: cannot request to receive report\r\n");
        }
    }

#endif
#ifdef __cplusplus
}
#endif
