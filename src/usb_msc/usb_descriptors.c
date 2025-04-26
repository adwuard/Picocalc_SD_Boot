#include "tusb.h"
#include "usb_msc.h"

//--------------------------------------------------------------------
// USB Device and Configuration Descriptors for MSC
//--------------------------------------------------------------------

// Vendor and Product ID, adjust as needed
#define USB_VID                 0xCafe
#define USB_PID                 0x4000
#define USB_BCD                 0x0100

// Interface number
#define ITF_NUM_MSC             0
#define ITF_NUM_TOTAL           1

// Endpoint numbers and size
#define EPNUM_MSC_OUT           0x01
#define EPNUM_MSC_IN            0x81
#define MSC_EP_BUFSIZE          512

// Configuration descriptor total length
#define CONFIG_TOTAL_LEN        (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

// String descriptor indices
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_MSC_INTERFACE,
};

// Device descriptor
static uint8_t const desc_device[] = {
  18,                     // bLength
  TUSB_DESC_DEVICE,       // bDescriptorType
  0x00, 0x02,             // bcdUSB
  0x00,                   // bDeviceClass
  0x00,                   // bDeviceSubClass
  0x00,                   // bDeviceProtocol
  CFG_TUD_ENDPOINT0_SIZE, // bMaxPacketSize0
  USB_VID & 0xFF, (USB_VID >> 8) & 0xFF, // idVendor
  USB_PID & 0xFF, (USB_PID >> 8) & 0xFF, // idProduct
  USB_BCD & 0xFF, (USB_BCD >> 8) & 0xFF, // bcdDevice
  STRID_MANUFACTURER,     // iManufacturer
  STRID_PRODUCT,          // iProduct
  STRID_SERIAL,           // iSerialNumber
  1                       // bNumConfigurations
};

// Configuration descriptor
static uint8_t const desc_configuration[] = {
  // Config number, interface count, string index, total len, attribute, power (mA)
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // MSC Interface
  TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, STRID_MSC_INTERFACE, EPNUM_MSC_OUT, EPNUM_MSC_IN, MSC_EP_BUFSIZE)
};

//--------------------------------------------------------------------
// String Descriptors
//--------------------------------------------------------------------
static char const* string_desc_arr[] = {
  (const char[]){ 0x09, 0x04 },       // 0: Supported language is English (0x0409)
  "PicoCalc",                        // 1: Manufacturer
  "SD Bootloader MSC",               // 2: Product
  "000000000000",                    // 3: Serial (dummy)
  "Mass Storage"                     // 4: MSC Interface
};

// Buffer for USB string descriptor
static uint16_t _desc_str[32];

//--------------------------------------------------------------------
// USB Callback Implementations
//--------------------------------------------------------------------

// Invoked when received GET DEVICE DESCRIPTOR request
uint8_t const* tud_descriptor_device_cb(void)
{
  return desc_device;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR request
uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations, use index
  return desc_configuration;
}

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;
  if ( index == 0 ) {
    // return supported language descriptor
    _desc_str[0] = (1 << 8) | TUSB_DESC_STRING;
    _desc_str[1] = string_desc_arr[0][0] | (string_desc_arr[0][1] << 8);
    return _desc_str;
  }

  if ( index >= (sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

  // Convert ASCII string into UTF-16
  const char* str = string_desc_arr[index];
  uint8_t len = strlen(str);
  if ( len > 31 ) len = 31;

  _desc_str[0] = (TUSB_DESC_STRING << 8) | (len + 1);
  for ( uint8_t i = 0; i < len; i++ ) {
    _desc_str[i+1] = str[i];
  }

  return _desc_str;
}