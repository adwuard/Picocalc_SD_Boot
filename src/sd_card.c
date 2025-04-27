#include "sd_card.h"
#include <hardware/gpio.h>

bool sd_card_inserted(void) {
    // SD card detect pin is active low
    return !gpio_get(SD_DET_PIN);
}
