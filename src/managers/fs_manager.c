#include "fs_manager.h"
#include "config.h"
#include "hardware/gpio.h"
#include "filesystem/vfs.h"
#include "sd_card.h"

#include <stdbool.h>

// External filesystem interface from main.c
extern bool fs_deinit(void);

// Callback pointers and internal state
static FSManager_CardInsertedCallback cardInsertedCb = NULL;
static FSManager_CardRemovedCallback  cardRemovedCb  = NULL;
static bool mounted                    = false;
static bool cardInsertedState         = false;

// Internal GPIO IRQ handler for SD detect pin
static void sd_detect_callback(uint gpio, uint32_t events) {
    (void) gpio;
    (void) events;
    bool insertedNow = sd_card_inserted();
    if (insertedNow && !cardInsertedState) {
        // Card inserted
        cardInsertedState = true;
        if (FSManager_mount() && cardInsertedCb) {
            cardInsertedCb();
        }
    } else if (!insertedNow && cardInsertedState) {
        // Card removed
        cardInsertedState = false;
        FSManager_unmount();
        if (cardRemovedCb) {
            cardRemovedCb();
        }
    }
}

bool FSManager_init(void) {
    // Configure SD detect pin (active low)
    gpio_init(SD_DET_PIN);
    gpio_set_dir(SD_DET_PIN, GPIO_IN);
    gpio_pull_up(SD_DET_PIN);

    // Enable IRQ on both edges to detect insertion/removal
    gpio_set_irq_enabled_with_callback(
        SD_DET_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        sd_detect_callback
    );

    // Check current card state and mount if present
    cardInsertedState = sd_card_inserted();
    if (cardInsertedState) {
        if (!fs_init()) {
            mounted = false;
            return false;
        }
        mounted = true;
        if (cardInsertedCb) {
            cardInsertedCb();
        }
    }
    return true;
}

void FSManager_deinit(void) {
    // Unmount and disable detection
    FSManager_unmount();
    gpio_set_irq_enabled(SD_DET_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    cardInsertedCb = NULL;
    cardRemovedCb  = NULL;
}

bool FSManager_isMounted(void) {
    return mounted;
}

void FSManager_registerCardInsertedCallback(FSManager_CardInsertedCallback callback) {
    cardInsertedCb = callback;
}

void FSManager_registerCardRemovedCallback(FSManager_CardRemovedCallback callback) {
    cardRemovedCb = callback;
}

bool FSManager_mount(void) {
    if (mounted) {
        return true;
    }
    if (!sd_card_inserted()) {
        return false;
    }
    if (fs_init()) {
        mounted = true;
        return true;
    }
    return false;
}

void FSManager_unmount(void) {
    if (mounted) {
        fs_deinit();
        mounted = false;
    }
}
