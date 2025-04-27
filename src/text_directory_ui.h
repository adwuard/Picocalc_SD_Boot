/**
 * @file text_directory_ui.h
 * @brief Text-based UI for navigating directories and files on an SD card
 *
 * This module provides a text-based UI for navigating directories and files on an SD card.
 * It uses lcdspi APIs for rendering, key_event APIs for input handling, and pico-vfs/standard 
 * POSIX APIs for filesystem operations.
 *
 * @author Hsuan Han Lai
 * @email hsuan.han.lai@gmail.com
 * @website https://hsuanhanlai.com
 * @year 2025
 */

#ifndef TEXT_DIRECTORY_UI_H
#define TEXT_DIRECTORY_UI_H

#include <stdbool.h>
#include "usb_msc/usb_msc.h"

/**
 * @brief Initialize the text directory UI
 *
 * Sets up the SD card filesystem and the display UI.
 *
 * @return true if initialization succeeded, false otherwise
 */
bool text_directory_ui_init(void);

/**
 * @brief Run the main event loop for the directory navigation UI
 *
 * This function polls for key events, updates the selection cursor,
 * and processes directory changes. It will exit when the ESC key is pressed.
 */
void text_directory_ui_run(void);

/**
 * @brief Register a callback for file selection
 *
 * The callback will be invoked when the user makes a final selection.
 *
 * @param callback Function pointer that accepts a const char* parameter
 *                 containing the selected file path
 */
void UIManager_setFinalCallback(void (*callback)(const char *selected_path));

/**
 * @brief Set a status or error message
 *
 * Displays a message in the status bar. The message will automatically
 * clear after 3 seconds.
 *
 * @param msg The message to display
 */
void text_directory_ui_set_status(const char *msg);

/**
 * @brief Pause the UI and enter USB MSC mode
 *
 * This function will return when USB is disconnected or when the device is reset.
 */
void text_directory_ui_pause(void);

/**
 * @brief Show USB Mass Storage mode active popup
 *
 * Displays an overlay popup indicating that USB Mass Storage mode is active.
 */
void text_directory_ui_show_msc_popup(void);

/**
 * @brief Hide USB Mass Storage mode popup
 *
 * Removes the USB Mass Storage mode overlay popup.
 */
void text_directory_ui_hide_msc_popup(void);

#endif // TEXT_DIRECTORY_UI_H
