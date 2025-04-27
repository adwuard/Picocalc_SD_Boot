/**
 * PicoCalc SD Firmware Loader
 *
 * Author: Hsuan Han Lai
 * Email: hsuan.han.lai@gmail.com
 * Website: https://hsuanhanlai.com
 * Year: 2025
 *
 *
 * This project is a bootloader for the PicoCalc device, designed to load and execute
 * firmware applications from an SD card.
 *
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "debug.h"
#include <hardware/flash.h>
#include <errno.h>
#include <hardware/watchdog.h>
#include "config.h"

#include "blockdevice/sd.h"
#include "filesystem/fat.h"
#include "filesystem/vfs.h"

// Manager modules
#include "managers/fs_manager.h"
#include "managers/ui_manager.h"
#include "managers/input_manager.h"
#include "managers/msc_manager.h"
#include "managers/event_bus.h"

const uint LEDPIN = 25;

// Vector and RAM offset
#if PICO_RP2040
#define VTOR_OFFSET M0PLUS_VTOR_OFFSET
#define MAX_RAM 0x20040000
#elif PICO_RP2350
#define VTOR_OFFSET M33_VTOR_OFFSET
#define MAX_RAM 0x20080000
#endif


static bool __not_in_flash_func(is_same_existing_program)(FILE *fp)
{
    uint8_t buffer[FLASH_SECTOR_SIZE] = {0};
    size_t program_size = 0;
    size_t len = 0;
    while ((len = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        uint8_t *flash = (uint8_t *)(XIP_BASE + SD_BOOT_FLASH_OFFSET + program_size);
        if (memcmp(buffer, flash, len) != 0)
            return false;
        program_size += len;
    }
    return true;
}

// This function must run from RAM since it erases and programs flash memory
static bool __not_in_flash_func(load_program)(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        DEBUG_PRINT("open %s fail: %s\n", filename, strerror(errno));
        return false;
    }
    if (is_same_existing_program(fp))
    {
        // Program is up to date
    }

    // Check file size to ensure it doesn't exceed the available flash space
    if (fseek(fp, 0, SEEK_END) == -1)
    {
        DEBUG_PRINT("seek err: %s\n", strerror(errno));
        fclose(fp);
        return false;
    }

    long file_size = ftell(fp);
    if (file_size <= 0)
    {
        DEBUG_PRINT("invalid size: %ld\n", file_size);
        fclose(fp);
        return false;
    }

    if (file_size > MAX_APP_SIZE)
    {
        DEBUG_PRINT("file too large: %ld > %d\n", file_size, MAX_APP_SIZE);
        fclose(fp);
        return false;
    }

    DEBUG_PRINT("updating: %ld bytes\n", file_size);
    if (fseek(fp, 0, SEEK_SET) == -1)
    {
        DEBUG_PRINT("seek err: %s\n", strerror(errno));
        fclose(fp);
        return false;
    }

    size_t program_size = 0;
    uint8_t buffer[FLASH_SECTOR_SIZE] = {0};
    size_t len = 0;

    // Erase and program flash in FLASH_SECTOR_SIZE chunks
    while ((len = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        // Ensure we don't write beyond the application area
        if ((program_size + len) > MAX_APP_SIZE)
        {
            DEBUG_PRINT("err: write beyond app area\n");
            fclose(fp);
            return false;
        }

        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(SD_BOOT_FLASH_OFFSET + program_size, FLASH_SECTOR_SIZE);
        flash_range_program(SD_BOOT_FLASH_OFFSET + program_size, buffer, len);
        restore_interrupts(ints);

        program_size += len;
    }
    DEBUG_PRINT("program loaded\n");
    fclose(fp);
    return true;
}

// This function jumps to the application entry point
// It must update the vector table and stack pointer before jumping
void __not_in_flash_func(launch_application_from)(uint32_t *app_location)
{
    // https://vanhunteradams.com/Pico/Bootloader/Bootloader.html
    uint32_t *new_vector_table = app_location;
    volatile uint32_t *vtor = (uint32_t *)(PPB_BASE + VTOR_OFFSET);
    *vtor = (uint32_t)new_vector_table;
    asm volatile(
        "msr msp, %0\n"
        "bx %1\n"
        :
        : "r"(new_vector_table[0]), "r"(new_vector_table[1])
        :);
}

// Check if a valid application exists in flash by examining the vector table
static bool is_valid_application(uint32_t *app_location)
{
    // Check that the initial stack pointer is within a plausible RAM region (assumed range for Pico: 0x20000000 to 0x20040000)
    uint32_t stack_pointer = app_location[0];
    if (stack_pointer < 0x20000000 || stack_pointer > MAX_RAM)
    {
        return false;
    }

    // Check that the reset vector is within the valid flash application area
    uint32_t reset_vector = app_location[1];
    if (reset_vector < (0x10000000 + SD_BOOT_FLASH_OFFSET) || reset_vector > (0x10000000 + PICO_FLASH_SIZE_BYTES))
    {
        return false;
    }
    return true;
}

int load_firmware_by_path(const char *path)
{
    text_directory_ui_set_status("STAT: loading app...");

    // Attempt to load the application from the SD card
    // bool load_success = load_program(FIRMWARE_PATH);
    bool load_success = load_program(path);

    // Get the pointer to the application flash area
    uint32_t *app_location = (uint32_t *)(XIP_BASE + SD_BOOT_FLASH_OFFSET);

    // Check if there is an already valid application in flash
    bool has_valid_app = is_valid_application(app_location);



    if (load_success || has_valid_app)
    {
        text_directory_ui_set_status("STAT: launching app...");
        DEBUG_PRINT("launching app\n");
        // Small delay to allow printf to complete
        sleep_ms(100);
        launch_application_from(app_location);
    }
    else
    {
        text_directory_ui_set_status("ERR: No valid app");
        DEBUG_PRINT("no valid app, halting\n");

        sleep_ms(2000);

        // Trigger a watchdog reboot
        watchdog_reboot(0, 0, 0);
    }

    // We should never reach here
    while (1)
    {
        tight_loop_contents();
    }
}

void final_selection_callback(const char *path)
{
    // Trigger firmware loading with the selected path
    DEBUG_PRINT("selected: %s\n", path);

    char status_message[128];
    const char *extension = ".bin";
    size_t path_len = strlen(path);
    size_t ext_len = strlen(extension);

    if (path_len < ext_len || strcmp(path + path_len - ext_len, extension) != 0)
    {
        DEBUG_PRINT("not a bin: %s\n", path);
        snprintf(status_message, sizeof(status_message), "Err: FILE is not a .bin file");
        text_directory_ui_set_status(status_message);
        return;
    }

    snprintf(status_message, sizeof(status_message), "SEL: %s", path);
    text_directory_ui_set_status(status_message);

    sleep_ms(200);

    load_firmware_by_path(path);
}

int main()
{
    stdio_init_all();

    uart_init(uart0, 115200);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE); // 8-N-1
    uart_set_fifo_enabled(uart0, false);

    // Initialize SD card detection pin
    gpio_init(SD_DET_PIN);
    gpio_set_dir(SD_DET_PIN, GPIO_IN);
    gpio_pull_up(SD_DET_PIN); // Enable pull-up resistor
    event_bus_init();

    
    keypad_init();
    lcd_init();
    lcd_clear();
    text_directory_ui_init();

    // Initialize the managers
    if (!InputManager_init()) {
        DEBUG_PRINT("Failed to initialize input manager\n");
        sleep_ms(2000);
        watchdog_reboot(0, 0, 0);
    }

    if (!UIManager_init()) {
        DEBUG_PRINT("Failed to initialize UI manager\n");
        sleep_ms(2000);
        watchdog_reboot(0, 0, 0);
    }

    // Set up the UI callback for file selection
    UIManager_setFinalCallback(final_selection_callback);

    // Initialize the MSC manager
    if (!MSCManager_init()) {
        DEBUG_PRINT("Failed to initialize MSC manager\n");
        UIManager_setStatus("Failed to initialize USB");
        sleep_ms(2000);
        watchdog_reboot(0, 0, 0);
    }

    // Launch core 1 to handle MSC operations
    multicore_launch_core1(MSCManager_core1_entry);

    // Check for SD card presence and initialize filesystem
    if (!FSManager_isMounted()) {
        DEBUG_PRINT("SD card not detected or not mounted\n");
        UIManager_setStatus("SD card not detected. Please insert SD card.");
        
        // Wait for card to be inserted and mounted
        while (!FSManager_isMounted()) {
            // Check if the card was inserted and try to mount it
            if (!FSManager_mount()) {
                sleep_ms(100);
            }
        }
    }
    
    // Main UI loop
    while (1) {
        // Show the directory UI
        UIManager_showDirectory();
        
        // If we get here, the user has selected USB MSC mode
        DEBUG_PRINT("Entering USB MSC mode\n");
        
        // Show the MSC mode popup overlay
        UIManager_showMscPopup();
        
        // Signal core 1 to start MSC mode
        event_bus_post(EVENT_MSC_START);
        
        // Wait for MSC exit event
        bool msc_active = true;
        while (msc_active) {
            if (event_bus_available()) {
                event_type_t event = event_bus_get();
                
                if (event == EVENT_MSC_EXIT || event == EVENT_ESC_PRESSED || event == EVENT_CARD_REMOVED) {
                    msc_active = false;
                    DEBUG_PRINT("MSC mode exit event received: %d\n", event);
                }
            }
            
            // Small delay to prevent tight polling
            sleep_ms(10);
        }
        
        // Hide the MSC mode popup overlay
        UIManager_hideMscPopup();
        
        // When USB is disconnected or ESC pressed, reinitialize the filesystem
        DEBUG_PRINT("USB MSC mode exited, remounting filesystem\n");
        UIManager_setStatus("USB MSC mode exited. Remounting...");
        
        if (!FSManager_mount()) {
            UIManager_setStatus("Failed to remount filesystem!");
            sleep_ms(2000);
            watchdog_reboot(0, 0, 0);
        }
        
        // Return to the UI
        UIManager_setStatus("Filesystem remounted. Returning to UI.");
        sleep_ms(500);
    }
}
