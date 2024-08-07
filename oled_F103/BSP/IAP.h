#ifndef __IAP_H
#define __IAP_H

#include "main.h"

#define FLASH_OTA_ADDR          0x08008000  //固件升级的FLASH地址
#define APP1_FLASH_SIZE         0x400
#define BOOTLOADER_START_ADDR   0x8000000  //BootLoader起始地址
#define BOOTLOADER_SIZE         0x3C00  //BootLoader大小
#define FIRMWARE_VERSION_STORE_ADDR  (BOOTLOADER_START_ADDR+BOOTLOADER_SIZE)   //存储固件信息的FLASH地址
#define FIRMWARE_VERSION_SIZE   16 //存储固件信息的字节数
#define FIRMWARE_SIZE         512  //固件大小

typedef struct 
{
	uint32_t file_size;//文件大小
	char md5sum[100];//服务器下发的md5
	char url[512];//下载URL
	char host[256];//OTA服务器域名
	char http_request[512];//HTTP请求报文
	int streamId;
	int counter;//计数器
	int num;
	int downlen;
	uint8_t  OTA_tempver[32];//临时存储的版本号
}ota_info_t;

extern ota_info_t ota_info;

void OTA_Init(void);
uint8_t  Pub_Device_Firmware(void);
uint8_t Device_Get_OTA_Info(ota_info_t *ota_info);
void Start_OTA_Update(ota_info_t *ota_info,char *check_md5);
void Update_FirmwareToApp(ota_info_t *ota_info,char *check_md5);
void Read_CurrentFirmwareVersion(char *current_version);
void Write_CurrentFirmwareVersion(char *current_version);


#endif






















#endif // __IAP_H



