#include <tusb.h>
#include <stdio.h>
#include "gamepad.h"
#include "gamepads.h"
#include "common/tusb_private.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if CFG_TUH_API_EDPT_XFER > 0
    namespace
    {
      #define BUF_COUNT 6
      tusb_desc_device_t desc_device;
      void print_device_descriptor(tuh_xfer_t* xfer);
      void parse_config_descriptor(uint8_t dev_addr, tusb_desc_configuration_t const* desc_cfg);
      void xfer_open_interface(uint8_t daddr, tusb_desc_interface_t const *desc_itf, uint16_t max_len);
      void xfer_report_received(tuh_xfer_t* xfer);
      uint8_t* xfer_get_buf(uint8_t daddr);
      void xfer_free_buf(uint8_t daddr);
      uint8_t buf_pool[BUF_COUNT][64];
      uint8_t buf_owner[BUF_COUNT] = { 0 }; // device address that owns buffer
      uint16_t temp_buf[128];
    }

    // Invoked when device is mounted (configured)
    void tuh_mount_cb (uint8_t daddr)
    {
      printf("Device attached, address = %d\r\n", daddr);
      // Get Device Descriptor
      // TODO: invoking control transfer now has issue with mounting hub with multiple devices attached, fix later
      tuh_descriptor_get_device(daddr, &desc_device, 18, print_device_descriptor, 0);
    }

    // Invoked when device is unmounted (bus reset/unplugged)
    void tuh_umount_cb(uint8_t daddr)
    {
      printf("Device removed, address = %d\r\n", daddr);
      xfer_free_buf(daddr);
    }

    void print_device_descriptor(tuh_xfer_t* xfer)
    {
      if ( XFER_RESULT_SUCCESS != xfer->result )
      {
        printf("Failed to get device descriptor\r\n");
        return;
      }

      uint8_t const daddr = xfer->daddr;

      printf("Device %u: ID %04x:%04x\r\n", daddr, desc_device.idVendor, desc_device.idProduct);
      printf("Device Descriptor:\r\n");
      printf("  bLength             %u\r\n"     , desc_device.bLength);
      printf("  bDescriptorType     %u\r\n"     , desc_device.bDescriptorType);
      printf("  bcdUSB              %04x\r\n"   , desc_device.bcdUSB);
      printf("  bDeviceClass        %u\r\n"     , desc_device.bDeviceClass);
      printf("  bDeviceSubClass     %u\r\n"     , desc_device.bDeviceSubClass);
      printf("  bDeviceProtocol     %u\r\n"     , desc_device.bDeviceProtocol);
      printf("  bMaxPacketSize0     %u\r\n"     , desc_device.bMaxPacketSize0);
      printf("  idVendor            0x%04x\r\n" , desc_device.idVendor);
      printf("  idProduct           0x%04x\r\n" , desc_device.idProduct);
      printf("  bcdDevice           %04x\r\n"   , desc_device.bcdDevice);

      drv2pad[daddr] = gamepads_attach(desc_device.idVendor, desc_device.idProduct);

      // Get configuration descriptor with sync API
      if (XFER_RESULT_SUCCESS == tuh_descriptor_get_configuration_sync(daddr, 0, temp_buf, sizeof(temp_buf)))
      {
        parse_config_descriptor(daddr, (tusb_desc_configuration_t*) temp_buf);
      }
    }

    // simple configuration parser to open and listen to HID Endpoint IN
    void parse_config_descriptor(uint8_t dev_addr, tusb_desc_configuration_t const* desc_cfg)
    {
      uint8_t const* desc_end = ((uint8_t const*) desc_cfg) + tu_le16toh(desc_cfg->wTotalLength);
      uint8_t const* p_desc   = tu_desc_next(desc_cfg);

      // parse each interfaces
      while( p_desc < desc_end )
      {
        uint8_t assoc_itf_count = 1;

        // Class will always starts with Interface Association (if any) and then Interface descriptor
        if ( TUSB_DESC_INTERFACE_ASSOCIATION == tu_desc_type(p_desc) )
        {
          tusb_desc_interface_assoc_t const * desc_iad = (tusb_desc_interface_assoc_t const *) p_desc;
          assoc_itf_count = desc_iad->bInterfaceCount;

          p_desc = tu_desc_next(p_desc); // next to Interface
        }

        // must be interface from now
        if( TUSB_DESC_INTERFACE != tu_desc_type(p_desc) ) return;
        tusb_desc_interface_t const* desc_itf = (tusb_desc_interface_t const*) p_desc;

        uint16_t const drv_len = tu_desc_get_interface_total_len(desc_itf, assoc_itf_count, (uint16_t) (desc_end-p_desc));

        // probably corrupted descriptor
        if(drv_len < sizeof(tusb_desc_interface_t)) return;

        // NOTE:
        // Above code is almost the same like in _parse_configuration_descriptor before driver open calls.

        // only open and listen to HID endpoint IN
        //if (desc_itf->bInterfaceClass == TUSB_CLASS_HID)
        
        // only open and listen to VENDOR endpoint IN
        if (desc_itf->bInterfaceClass == TUSB_CLASS_VENDOR_SPECIFIC)
        {
          xfer_open_interface(dev_addr, desc_itf, drv_len);
        }
        // next Interface or IAD descriptor
        p_desc += drv_len;
      }
    }

    // This is similar to hidh_open()...
    void xfer_open_interface(uint8_t daddr, tusb_desc_interface_t const *desc_itf, uint16_t max_len)
    {
      // len = interface + hid + n*endpoints
      uint16_t const drv_len = (uint16_t) (sizeof(tusb_desc_interface_t) + sizeof(tusb_hid_descriptor_hid_t) + desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t));

      // corrupted descriptor
      if (max_len < drv_len) return;

      uint8_t const *p_desc = (uint8_t const *) desc_itf;

      // HID descriptor
      p_desc = tu_desc_next(p_desc);
      tusb_hid_descriptor_hid_t const *desc_hid = (tusb_hid_descriptor_hid_t const *) p_desc;
      if(HID_DESC_TYPE_HID != desc_hid->bDescriptorType) return;

      // Endpoint descriptor
      p_desc = tu_desc_next(p_desc);
      tusb_desc_endpoint_t const * desc_ep = (tusb_desc_endpoint_t const *) p_desc;

      if (desc_ep->bEndpointAddress!=0x81) return;

      for(int i = 0; i < desc_itf->bNumEndpoints; i++)
      {
        if (TUSB_DESC_ENDPOINT != desc_ep->bDescriptorType) return;

        if(tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_IN)
        {
          // skip if failed to open endpoint
          if ( ! tuh_edpt_open(daddr, desc_ep) ) return;

          uint8_t* buf = xfer_get_buf(daddr);
          if (!buf) return; // out of memory

          tuh_xfer_t xfer =
          {
            .daddr       = daddr,
            .ep_addr     = desc_ep->bEndpointAddress,
            .buflen      = 64,
            .buffer      = buf,
            .complete_cb = xfer_report_received,
            .user_data   = (uintptr_t) buf, // since buffer is not available in callback, use user data to store the buffer
          };

          // submit transfer for this EP
          tuh_edpt_xfer(&xfer);

          printf("Listen to [dev %u: ep %02x]\r\n", daddr, desc_ep->bEndpointAddress);
        }

        p_desc = tu_desc_next(p_desc);
        desc_ep = (tusb_desc_endpoint_t const *) p_desc;
      }
    }

    // Note:
    //   Oposite to tuh_hid_report_received_cb this method is fired all the time even for the same incoming data,
    //   at least for the pad that I have, so it means there must be additonal logic to clear the state of gamepads.
    void xfer_report_received(tuh_xfer_t* xfer)
    {
      // Note: not all field in xfer is available for use (i.e filled by tinyusb stack) in callback to save sram
      // For instance, xfer->buffer is NULL. We have used user_data to store buffer when submitted callback
      uint8_t* buf = (uint8_t*) xfer->user_data;

      // static bool release = false;

      if (xfer->result == XFER_RESULT_SUCCESS)
      {
        drv2pad[xfer->daddr]->read(xfer->daddr, buf, xfer->actual_len);

        /*
        if ((buf[2]>0 || buf[3]>0) && release == false){
          release = true;
          drv2pad[xfer->daddr]->read(xfer->daddr, buf, xfer->actual_len);
          // printf("XFER PRESS [dev %u: ep %02x] HID Report:\n", xfer->daddr, xfer->ep_addr);
        } else if (release) {
          release = false;
          // printf("XFER RELEASE [dev %u: ep %02x] HID Report:\n", xfer->daddr, xfer->ep_addr);
          drv2pad[xfer->daddr]->read(xfer->daddr, buf, xfer->actual_len);
        }
        */

        // for(uint32_t i=0; i<xfer->actual_len; i++)
        // {
        //   if (i%16 == 0) printf("\r\n  ");
        //   printf("%02X ", buf[i]);
        // }
        // printf("\r\n");
      }

      // continue to submit transfer, with updated buffer
      // other field remain the same
      xfer->buflen = 64;
      xfer->buffer = buf;

      tuh_edpt_xfer(xfer);
    }

    // get an buffer from pool
    uint8_t* xfer_get_buf(uint8_t daddr)
    {
      for(size_t i=0; i<BUF_COUNT; i++)
      {
        if (buf_owner[i] == 0)
        {
          buf_owner[i] = daddr;
          return buf_pool[i];
        }
      }

      // out of memory, increase BUF_COUNT
      return NULL;
    }

    // free all buffer owned by device
    void xfer_free_buf(uint8_t daddr)
    {
      for(size_t i=0; i<BUF_COUNT; i++)
      {
        if (buf_owner[i] == daddr) buf_owner[i] = 0;
      }
    }
    
#endif
#ifdef __cplusplus
}
#endif
