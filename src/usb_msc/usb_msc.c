/*
 * usb_msc.c
 *
 * USB Mass Storage Class implementation for PicoCalc SD Bootloader.
 */

#include "usb_msc.h"
#include "tusb.h"

// #include "tusb_scsi.h" // Removed as it is not found and may be unnecessary
#include "config.h"
#include "blockdevice/sd.h"
#include <string.h>

// Block device for MSC
static blockdevice_t *msc_blockdev;
static uint32_t msc_block_count;
static uint16_t msc_block_size;

// Buffers for sector I/O
static uint8_t msc_read_buffer[512];
static uint8_t msc_write_buffer[512];
static uint32_t msc_last_read_lba = (uint32_t)-1;
static uint32_t msc_last_write_lba = (uint32_t)-1;

void usb_msc_init(void)
{
    // Initialize TinyUSB device stack
    tusb_init();

    // Create SD block device for MSC
    msc_blockdev = blockdevice_sd_create(
        SD_SPI0, SD_MOSI_PIN, SD_MISO_PIN, SD_SCLK_PIN, SD_CS_PIN,
        CONF_SD_TRX_FREQUENCY, true
    );
    if (!msc_blockdev) {
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
    (void) msc_blockdev;
}

// Invoked when device is unmounted by the host
void tud_umount_cb(void)
{
    (void) msc_blockdev;
}

// Max logical unit number (0-based), returns 0 for single LUN
uint8_t tud_msc_get_maxlun_cb(void)
{
    return 0;
}

// SCSI Inquiry data
void tud_msc_inquiry_cb(uint8_t lun, uint8_t p_vendor_id[8], uint8_t p_product_id[16], uint8_t p_product_rev[4])
{
    (void) lun;
    static const char vendor[8]  = "PICO    ";
    static const char product[16] = "SD_MSC_BOOT     ";
    static const char revision[4] = "1.0 ";

    memcpy(p_vendor_id,  vendor, 8);
    memcpy(p_product_id, product, 16);
    memcpy(p_product_rev, revision, 4);
}

// SCSI Read Capacity
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    (void) lun;
    *block_count = msc_block_count;
    *block_size  = msc_block_size;
}

// SCSI Read10: transfer data from SD to host
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    (void) lun;

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

// SCSI Write10: receive data from host and program to SD
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    (void) lun;

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
    
    return true;
}

int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
    // Example implementation of scsi_cb
    (void) lun;
    (void) scsi_cmd;
    (void) buffer;
    (void) bufsize;

    // Return a default value indicating success
    return 0;
}

// Returns true if MSC interface is currently mounted/connected to a host
bool usb_msc_is_mounted(void)
{
    return tud_connected();
}

// Stops the USB Mass Storage Class interface
void usb_msc_stop(void)
{
    tud_disconnect();
}
