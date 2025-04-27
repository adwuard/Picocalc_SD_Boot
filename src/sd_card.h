#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdbool.h>
#include "config.h"

/**
 * @brief Check if an SD card is inserted in the slot
 * 
 * @return true if an SD card is inserted
 * @return false if no SD card is inserted
 */
bool sd_card_inserted(void);

#endif // SD_CARD_H