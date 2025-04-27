/**
 * @file managers/ui_manager.c
 * @brief High-level UI manager implementation delegating to the Text Directory UI.
 */

#include "ui_manager.h"
#include "text_directory_ui.h"

/**
 * @brief Initialize the UI manager.
 *
 * This sets up the text-based directory navigation UI and mounts the SD card filesystem.
 *
 * @return true if initialization succeeded, false otherwise
 */
bool UIManager_init(void)
{
    return text_directory_ui_init();
}

/**
 * @brief Display and run the directory navigation UI.
 *
 * This enters the UI event loop for browsing directories and selecting files.
 */
void UIManager_showDirectory(void)
{
    text_directory_ui_run();
}

/**
 * @brief Show an overlay popup indicating that USB Mass Storage mode is active.
 *
 * This overlay informs the user that the device is acting as an MSC drive.
 */
void UIManager_showMscPopup(void)
{
    text_directory_ui_show_msc_popup();
}

/**
 * @brief Hide the USB Mass Storage mode overlay popup.
 *
 * This restores the directory navigation UI after MSC mode ends.
 */
void UIManager_hideMscPopup(void)
{
    text_directory_ui_hide_msc_popup();
}

/**
 * @brief Set a status or error message to be displayed in the status bar.
 *
 * The message will auto-clear after a few seconds.
 *
 * @param msg The message to display
 */
void UIManager_setStatus(const char *msg)
{
    text_directory_ui_set_status(msg);
}

/**
 * @brief Register a callback for when the user makes a final selection.
 *
 * The provided callback is invoked with the selected file path when the user confirms a choice.
 *
 * @param callback Function to call upon final file selection
 */
void UIManager_setFinalCallback(UIManager_FinalSelectionCallback callback)
{
    text_directory_ui_set_final_callback(callback);
}
