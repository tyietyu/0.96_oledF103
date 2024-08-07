#ifndef _FLASH_H
#define _FLASH_H

#include "main.h"
#include "IAP.h"
#include "esp8266.h"

void iap_write_flash(uint32_t address, uint8_t *data, uint32_t size);
void iap_read_flash(uint32_t address, uint8_t *data, uint32_t size);

#endif // !_FLASH_H