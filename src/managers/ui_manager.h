/*
 * ui_manager.h
 *
 * High-level UI manager for the SD card file browser
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <stdbool.h>

/**
 * Callback type: invoked when the user makes a final selection.
 * The selected path is passed as an argument.
 */
typedef void (*UIManager_FinalSelectionCallback)(const char *selected_path);

/**
 * Initialize the UI manager.
 * This sets up the display and UI components.
 *
 * @return true if initialization succeeded, false otherwise
 */
bool UIManager_init(void);

/**
 * Display and run the directory navigation UI.
 * This function handles the directory listing, navigation, and selection.
 */
void UIManager_showDirectory(void);

/**
 * Show an overlay popup indicating that USB Mass Storage mode is active.
 */
void UIManager_showMscPopup(void);

/**
 * Hide the USB Mass Storage mode overlay popup.
 */
void UIManager_hideMscPopup(void);

/**
 * Set a status or error message to be displayed in the status bar.
 * The message will auto-clear after a few seconds.
 *
 * @param msg The message to display
 */
void UIManager_setStatus(const char *msg);

/**
 * Register a callback that will be called when the user makes a final selection.
 *
 * @param callback The function to call when a final selection is made
 */
void UIManager_setFinalCallback(UIManager_FinalSelectionCallback callback);

#endif // UI_MANAGER_H