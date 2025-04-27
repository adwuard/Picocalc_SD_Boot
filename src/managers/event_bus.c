#include "event_bus.h"
#include "pico/multicore.h"

void event_bus_init(void) {
    // Clear any existing events in the FIFO
    while (multicore_fifo_rvalid()) {
        (void)multicore_fifo_pop_blocking();
    }
}

bool event_bus_post(event_type_t event) {
    if (event <= EVENT_NONE || event >= EVENT_MAX) {
        return false;
    }
    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking((uint32_t)event);
        return true;
    }
    return false;
}

void event_bus_post_blocking(event_type_t event) {
    if (event <= EVENT_NONE || event >= EVENT_MAX) {
        return;
    }
    multicore_fifo_push_blocking((uint32_t)event);
}

bool event_bus_available(void) {
    return multicore_fifo_rvalid();
}

event_type_t event_bus_get(void) {
    uint32_t value;
    // Non-blocking pop with zero timeout
    if (multicore_fifo_pop_timeout_us(0, &value)) {
        if (value > EVENT_NONE && value < EVENT_MAX) {
            return (event_type_t)value;
        }
    }
    return EVENT_NONE;
}

event_type_t event_bus_get_blocking(void) {
    uint32_t value = multicore_fifo_pop_blocking();
    if (value > EVENT_NONE && value < EVENT_MAX) {
        return (event_type_t)value;
    }
    return EVENT_NONE;
}

void event_bus_clear(void) {
    // Drain all pending events
    while (multicore_fifo_rvalid()) {
        (void)multicore_fifo_pop_blocking();
    }
}