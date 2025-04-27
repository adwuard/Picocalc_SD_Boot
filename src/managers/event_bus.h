/**
 * @file event_bus.h
 * @brief Event bus system for inter-core communication
 * 
 * This module provides a simple event bus implementation using the Raspberry Pi Pico's
 * multicore FIFO for communication between cores. It allows posting and subscribing
 * to events such as MSC_START, MSC_EXIT, ESC_PRESSED, and CARD_REMOVED.
 */

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/multicore.h"

/**
 * @brief Event types that can be posted to the event bus
 */
typedef enum {
    EVENT_NONE = 0,        /**< No event / invalid event */
    EVENT_MSC_START,       /**< Signal to start USB Mass Storage mode */
    EVENT_MSC_EXIT,        /**< Signal that USB Mass Storage mode has exited */
    EVENT_ESC_PRESSED,     /**< ESC key was pressed */
    EVENT_CARD_REMOVED,    /**< SD card was removed */
    EVENT_MAX              /**< Marker for the maximum event value */
} event_type_t;

/**
 * @brief Initialize the event bus system
 * 
 * This function should be called once at startup before using any other
 * event bus functions. It initializes the underlying multicore FIFO.
 * 
 * @note This should be called after multicore_launch_core1() if using both cores.
 */
void event_bus_init(void);

/**
 * @brief Post an event to the event bus
 * 
 * This function posts an event to the event bus, which can then be received
 * by subscribers on either core. It uses the multicore FIFO to transmit the event.
 * 
 * @param event The event type to post
 * @return true if the event was successfully posted, false otherwise
 */
bool event_bus_post(event_type_t event);

/**
 * @brief Post an event to the event bus, blocking if the FIFO is full
 * 
 * This function posts an event to the event bus, blocking if the FIFO is full
 * until space becomes available.
 * 
 * @param event The event type to post
 */
void event_bus_post_blocking(event_type_t event);

/**
 * @brief Check if there are any events available on the bus
 * 
 * @return true if there are events available to be read, false otherwise
 */
bool event_bus_available(void);

/**
 * @brief Get the next event from the event bus
 * 
 * This function checks if there are any events available and returns the next one.
 * If no events are available, it returns EVENT_NONE.
 * 
 * @return The next event, or EVENT_NONE if no events are available
 */
event_type_t event_bus_get(void);

/**
 * @brief Get the next event from the event bus, blocking until one is available
 * 
 * This function blocks until an event is available on the bus, then returns it.
 * 
 * @return The next event from the bus
 */
event_type_t event_bus_get_blocking(void);

/**
 * @brief Clear any pending events from the event bus
 * 
 * This function clears all pending events from the event bus FIFO.
 */
void event_bus_clear(void);

#endif /* EVENT_BUS_H */