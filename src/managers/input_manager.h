/**
 * @file input_manager.h
 * @brief Input manager module for handling keyboard input
 *
 * This module provides a wrapper around the keypad functions to handle
 * keyboard input in a more modular way.
 */

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Key code definitions
 */
// #define KEY_ARROW_UP     0xB5
// #define KEY_ARROW_DOWN   0xB6
// #define KEY_ARROW_LEFT   0xB4
// #define KEY_ARROW_RIGHT  0xB7
// #define KEY_ENTER        0x0A
// #define KEY_ESC          0x1B
// #define KEY_BACKSPACE    0x08

/**
 * @brief Initialize the input manager
 *
 * This function initializes the input manager by setting up the keypad.
 */
int InputManager_init(void);

/**
 * @brief Poll for key presses
 *
 * This function polls the keypad for key presses and returns the key code
 * of the pressed key, or 0 if no key is pressed.
 *
 * @return Key code of the pressed key, or 0 if no key is pressed
 */
int InputManager_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_MANAGER_H */