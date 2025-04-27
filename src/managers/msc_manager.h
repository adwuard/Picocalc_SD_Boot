/**
 * @file msc_manager.h
 * @brief USB Mass Storage Class (MSC) Manager
 *
 * This module manages the USB Mass Storage Class interface, running the
 * TinyUSB device task on core 1 to prevent USB halts when waiting for key events.
 * It provides functions to initialize, start, and stop MSC operations, as well as
 * register callbacks for MSC exit events.
 */

#ifndef MSC_MANAGER_H
#define MSC_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function pointer type for MSC exit callback
 * 
 * This callback is invoked when the MSC mode is exited, either by user action
 * or by SD card removal.
 */
typedef void (*msc_exit_callback_t)(void);

/**
 * @brief Initialize the MSC Manager
 * 
 * Sets up the MSC Manager and prepares it for operation. This should be called
 * before any other MSC Manager functions.
 * 
 * @return true if initialization was successful, false otherwise
 */
bool MSCManager_init(void);

/**
 * @brief Core 1 entry function for MSC operations
 * 
 * This function should be launched on core 1 using multicore_launch_core1().
 * It runs the TinyUSB device task and handles MSC-related events.
 * This function does not return until MSC operations are stopped.
 */
void MSCManager_core1_entry(void);

/**
 * @brief Stop MSC operations
 * 
 * Stops the MSC operations and cleans up resources. This will cause
 * MSCManager_core1_entry() to return on core 1.
 */
void MSCManager_stop(void);

/**
 * @brief Register a callback for MSC exit events
 * 
 * @param callback Function to call when MSC mode is exited
 */
void MSCManager_onExit(msc_exit_callback_t callback);

/**
 * @brief Check if MSC is currently mounted by a host
 * 
 * @return true if MSC is mounted, false otherwise
 */
bool MSCManager_isMounted(void);

#ifdef __cplusplus
}
#endif

#endif /* MSC_MANAGER_H */