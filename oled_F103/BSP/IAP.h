#ifndef __IAP_H
#define __IAP_H

#include "main.h"

#define BOOTLOADER_START_ADDR   0x8000000  //BootLoader起始地址
#define BOOTLOADER_SIZE         0x3C00  //BootLoader大小
#define FIRMWARE_VERSION_STORE_ADDR  (BOOTLOADER_START_ADDR+BOOTLOADER_SIZE)   //存储固件信息的FLASH地址
#define FIRMWARE_VERSION_SIZE   16 //存储固件信息的字节数

void IAP_Init(void);
uint8_t  Device_Get_OTA_Firmware(void);
uint8_t esp8266_send_firmware_version(void);
uint8_t esp8266_receive_firmware_version(void);


#endif





















#endif // __IAP_H



