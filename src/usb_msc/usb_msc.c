/*
 * usb_msc.c
 *
 * USB Mass Storage Class implementation for PicoCalc SD Bootloader.
 */

#include "usb_msc.h"
#include "tusb.h"
#include "config.h"
#include "blockdevice/sd.h"
#include "text_directory_ui.h"
#include <string.h>
#include <hardware/gpio.h>

// Block device for MSC
static blockdevice_t *msc_blockdev;
static uint32_t msc_block_count;
static uint16_t msc_block_size;

// Check if SD card is inserted (using the detection pin)
static bool sd_card_inserted(void)
{
    // SD card detection pin is typically active low (0 when card inserted)
    return !gpio_get(SD_DET_PIN);
}

// Buffers for sector I/O
static uint8_t msc_read_buffer[512];
static uint8_t msc_write_buffer[512];
static uint32_t msc_last_read_lba = (uint32_t)-1;
static uint32_t msc_last_write_lba = (uint32_t)-1;

void usb_msc_init(void)
{
    // Initialize TinyUSB device stack
    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    // Create SD block device for MSC
    msc_blockdev = blockdevice_sd_create(
        spi0, SD_MOSI_PIN, SD_MISO_PIN, SD_SCLK_PIN, SD_CS_PIN,
        125000000 / 2 / 4, true
    );
    if (!msc_blockdev) {
        text_directory_ui_set_status("USB MSC init failed");
        return; // Failed to initialize block device
    }

    // Retrieve capacity information from blockdevice
    msc_block_size  = msc_blockdev->erase_size;
    msc_block_count = msc_blockdev->size(msc_blockdev) / msc_block_size;
}

//--------------------------------------------------------------------
// USB MSC callbacks
//--------------------------------------------------------------------

// Invoked when device is mounted by the host
void tud_mount_cb(void)
{
    text_directory_ui_show_msc_popup();
}

// Invoked when device is unmounted by the host
void tud_umount_cb(void)
{
    text_directory_ui_hide_msc_popup();
}


// Invoked to determine max LUN
// Max logical unit number (0-based), returns 0 for single LUN
uint8_t tud_msc_get_maxlun_cb(void)
{
    return 0;
}

// SCSI Inquiry data
// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t p_vendor_id[8], uint8_t p_product_id[16], uint8_t p_product_rev[4])
{
    (void) lun;
    static const char vendor[8]  = "PICO";
    static const char product[16] = "SD_MSC_BOOT";
    static const char revision[4] = "1.0 ";

    memcpy(p_vendor_id,  vendor, strlen(vendor));
    memcpy(p_product_id, product, strlen(product));
    memcpy(p_product_rev, revision, strlen(revision));
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
// SCSI Read Capacity
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    (void) lun;
    *block_count = msc_block_count;
    *block_size  = msc_block_size;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void) lun;
    (void) power_condition;
  
    if (load_eject) {
      if (start) {
        // load disk storage
      } else {
        // unload disk storage
      }
    }
  
    return true;
  }

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
// SCSI Read10: transfer data from SD to host
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    // Check if SD card is still inserted
    if (!sd_card_inserted()) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return -1;
    }

    // On new block, load entire sector
    if (offset == 0) {
        if (msc_blockdev->read(msc_blockdev, msc_read_buffer, lba, 1) != 0) {
            return false;
        }
        msc_last_read_lba = lba;
    }

    // Copy requested portion
    memcpy(buffer, msc_read_buffer + offset, bufsize);
    return true;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
    (void) lun;
  
  #if defined(CFG_EXAMPLE_MSC_READONLY) || defined(CFG_EXAMPLE_MSC_DUAL_READONLY)
    return false;
  #else
    return true;
  #endif
  }

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
// SCSI Write10: receive data from host and program to SD
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    // Check if SD card is still inserted
    if (!sd_card_inserted()) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return -1;
    }

    // On start of block, update LBA
    if (offset == 0) {
        msc_last_write_lba = lba;
    }

    // Copy incoming data to buffer
    memcpy(msc_write_buffer + offset, buffer, bufsize);

    // When entire block received, program to SD
    if (offset + bufsize >= msc_block_size) {
        if (msc_blockdev->program(msc_blockdev, msc_write_buffer, msc_last_write_lba, 1) != 0) {
            return false;
        }
    }
    return true;
}

// Flush any pending writes (not needed for SD)
void tud_msc_write10_flush_cb(uint8_t lun)
{
    (void) lun;
}

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    // return sd_card_inserted();
    return true;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks (MUST not be handled here)
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
    (void) buffer;
    (void) bufsize;
  
    switch (scsi_cmd[0]) {
      default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
  
        // negative means error -> tinyusb could stall and/or response with failed status
        return -1;
    }
}

// Returns true if MSC interface is currently mounted/connected to a host
bool usb_msc_is_mounted(void)
{
    return tud_connected();
}

// Stops the USB Mass Storage Class interface
void usb_msc_stop(void)
{
    if (msc_blockdev) {
        blockdevice_sd_free(msc_blockdev);
        msc_blockdev = NULL;
    }
    tud_disconnect();
}
