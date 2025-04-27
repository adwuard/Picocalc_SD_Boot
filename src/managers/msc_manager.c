#include "msc_manager.h"
#include "usb_msc.h"
#include "managers/event_bus.h"
#include "tusb.h"
#include <stdbool.h>

// Internal flag to signal exit from the core1 MSC loop
static volatile bool exit_flag = false;
// User-registered callback for MSC exit
static msc_exit_callback_t exit_callback = NULL;

bool MSCManager_init(void) {
    // Initialize the event bus for inter-core signaling
    event_bus_init();
    exit_flag = false;
    exit_callback = NULL;
    return true;
}

void MSCManager_onExit(msc_exit_callback_t callback) {
    exit_callback = callback;
}

void MSCManager_stop(void) {
    // Signal the core1 loop to exit
    exit_flag = true;
}

bool MSCManager_isMounted(void) {
    // Query underlying USB MSC layer
    return usb_msc_is_mounted();
}

void MSCManager_core1_entry(void) {
    // Ensure exit flag is clear
    exit_flag = false;

    // Start the USB MSC interface
    usb_msc_init();

    // Run USB device tasks and handle exit events
    while (!exit_flag) {
        // Process TinyUSB device tasks
        tud_task();

        // Check for exit events from the event bus
        if (event_bus_available()) {
            event_type_t ev = event_bus_get();
            switch (ev) {
                case EVENT_ESC_PRESSED:
                case EVENT_CARD_REMOVED:
                    // Exit on user ESC press or SD card removal
                    exit_flag = true;
                    break;
                default:
                    // Ignore other events
                    break;
            }
        }
    }

    // Stop the USB MSC interface
    usb_msc_stop();

    // Invoke the registered exit callback if present
    if (exit_callback) {
        exit_callback();
    }
}
