/**
 * @file fs_manager.h
 * @brief File system manager for SD card operations
 *
 * This module provides an interface for managing SD card operations including
 * initialization, mounting, and detecting card insertion/removal events.
 */

#ifndef FS_MANAGER_H
#define FS_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Callback function type for SD card insertion events
 */
typedef void (*FSManager_CardInsertedCallback)(void);

/**
 * @brief Callback function type for SD card removal events
 */
typedef void (*FSManager_CardRemovedCallback)(void);

/**
 * @brief Initialize the file system manager
 *
 * This function initializes the SD card hardware and attempts to mount
 * the file system if a card is present. It should be called once at
 * system startup before any other FSManager functions.
 *
 * @return true if initialization was successful, false otherwise
 */
bool FSManager_init(void);

/**
 * @brief Deinitialize the file system manager
 *
 * This function unmounts the file system if it's currently mounted and
 * deinitializes the SD card hardware. It should be called before system
 * shutdown or when the SD card functionality is no longer needed.
 */
void FSManager_deinit(void);

/**
 * @brief Check if the SD card is currently mounted
 *
 * This function checks if an SD card is present and properly mounted.
 * It can be used to determine if file operations can be performed.
 *
 * @return true if the SD card is mounted and ready for use, false otherwise
 */
bool FSManager_isMounted(void);

/**
 * @brief Register a callback for SD card insertion events
 *
 * This function registers a callback that will be called when an SD card
 * is inserted. Only one callback can be registered at a time.
 *
 * @param callback Function pointer to call when an SD card is inserted
 */
void FSManager_registerCardInsertedCallback(FSManager_CardInsertedCallback callback);

/**
 * @brief Register a callback for SD card removal events
 *
 * This function registers a callback that will be called when an SD card
 * is removed. Only one callback can be registered at a time.
 *
 * @param callback Function pointer to call when an SD card is removed
 */
void FSManager_registerCardRemovedCallback(FSManager_CardRemovedCallback callback);

/**
 * @brief Attempt to mount the SD card
 *
 * This function attempts to mount the SD card file system. It can be called
 * after a card insertion event to make the card available for file operations.
 *
 * @return true if the mount operation was successful, false otherwise
 */
bool FSManager_mount(void);

/**
 * @brief Unmount the SD card
 *
 * This function unmounts the SD card file system. It should be called before
 * a card is removed or when file operations are no longer needed.
 */
void FSManager_unmount(void);

#endif /* FS_MANAGER_H */