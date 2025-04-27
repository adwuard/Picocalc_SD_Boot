/**
 * @file input_manager.c
 * @brief Input manager module implementation for handling keyboard input
 */

#include "input_manager.h"
#include "key_event.h"
#include "event_bus.h"

int InputManager_init(void) {
    // Initialize the underlying keypad hardware
    return keypad_init();
}

int InputManager_poll(void) {
    // Poll the keypad for a key press
    int key = keypad_get_key();

    // If ESC key is pressed, post an ESC_PRESSED event
    if (key == KEY_ESC) {
        event_bus_post(EVENT_ESC_PRESSED);
    }

    return key;
}
