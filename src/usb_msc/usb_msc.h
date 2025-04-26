#ifndef USB_MSC_H
#define USB_MSC_H

#include <stdbool.h>
#include "blockdevice/blockdevice.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the USB Mass Storage Class interface.
// Sets up TinyUSB device stack and creates an SD blockdevice
// using the blockdevice_sd_create() function.
void usb_msc_init(void);

// Stops the USB Mass Storage Class interface.
// Deinitializes TinyUSB device stack and frees the SD blockdevice.
void usb_msc_stop(void);

// Returns true if MSC interface is currently mounted/connected to a host.
// 
// Note: The implementation automatically calls text_directory_ui_show_msc_popup()
// and text_directory_ui_hide_msc_popup() when the MSC device is mounted or
// unmounted by the host.
bool usb_msc_is_mounted(void);

#ifdef __cplusplus
}
#endif

#endif // USB_MSC_H
