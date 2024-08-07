#include "IAP.h"
#include "esp8266.h"
#include "usart.h"
#include "flash.h"

ESP8266_UART_Buffer esp8266_uart_buff;

void OTA_Init(void)
{
    ESP8266_sw_rese();
    ESP8266_set_mode(1);
    ESP8266_ate_config(0);
    ESP8266_join_wifi();
    ESP8266_Connect_Aliyun();
    ESP8266_Sub_Pub_Topic_Aliyun(0);
}

uint8_t Pub_Device_Firmware(void)
{
    uint8_t retval = 0;
    uint16_t count = 0;
    static uint8_t error_count = 0;
    unsigned char msg_buf[256];

    sprintf((char *)msg_buf, "AT+MQTTPUB=0,\"" UPLOAD_INFORMATION_PUB "\",\"" JSON_FORMAT_FIRMWARE "\",1,0\r\n");
    HAL_UART_Transmit(&huart2, (unsigned char *)msg_buf, strlen((const char *)msg_buf), 1000);
    while ((esp8266_uart_buff.receive_start == 0) && (count < 500))
    {
        count++;
        HAL_Delay(1);
    }
    if (count >= 500)
    {
        retval = 1;
    }
    else
    {
        HAL_Delay(50);
        if (strstr((const char *)esp8266_uart_buff.receive_buff, "OK"))
        {
            retval = 0;
            error_count = 0;
        }
        else
        {
            error_count++;
            if (error_count == 5)
            {
                error_count = 0;
                printf("ESP8266_Pub_Device_Firmware fail ! ! !\r\n");
                ESP8266_init();
            }
        }
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}

uint8_t Device_Get_OTA_Info(ota_info_t *ota_info)
{
    if(ESP8266_Sub_Pub_Topic_Aliyun(3)==0)
    {
       if (sscanf((char *)esp8266_uart_buff.receive_buff, 
           DOWNLOAD_INFORMATION_SUB "{\"code\":\"1000\",\"data\":{"
           "\"size\":%d,\"streamId\":%d,\"sign\":\"%*32s\",\"dProtocol\":\"mqtt\",\"version\":\"%26s\","
           "\"signMethod\":\"Md5\",\"streamFileId\":1,\"md5\":\"%*32s\"},"
           "\"id\":%*d,\"message\":\"success\"}", 
           &ota_info->file_size, &ota_info->streamId, ota_info->OTA_tempver) == 3)
         printf("OTA固件大小:%d\r\n", ota_info.file_size);
         printf("OTA固件ID:%d\r\n", ota_info.streamId);
         printf("OTA固件版本号:%s\r\n", ota_info.OTA_tempver);
        if(ota_info.file_size % 256 == 0)
        {
            ota_info->counter = ota_info.file_size / 256;
        }
        else
        {
            ota_info->counter = ota_info.file_size / 256 + 1;
        }
        ota_info->num=1;
        ota_info->downlen=256;
        OTA_Download();
    }
    else
    {
       printf("OTA固件信息获取失败\r\n");
    }
}

void OTA_Download(int size, int offset)
{
    char temp[256];
    memset(temp, 0, sizeof(temp));
    sprintf(temp,"{\"id\": \"1\",\"params\": {\"fileInfo\":{\"streamId\":%d,\"fileId\":1},\"fileBlock\":{\"size\":%d,\"offset\":%d}}}", ota_info.streamId, size, offset);
    printf("当前第%d/%d次\r\n",ota_info.num,ota_info.counter);
}
/*
函数功能：将下载的固件更新到APP1中
参数：ota_info_t *ota_info,char *check_md5
返回值：无
*/
void Update_FirmwareToApp(ota_info_t *ota_info, char *check_md5)
{
    if (strcmp(ota_info->md5sum, check_md5) == 0) // 比较MD5值
    {
        printf("OTA Firmware MD5 OK\r\nUpdate APP...\r\n");
        iap_write_flash(FLASH_APP1_ADDR, (u8 *)FLASH_OTA_ADDR, ota_info->file_size);
        Compute_string_md5((u8 *)FLASH_APP1_ADDR, ota_info->file_size, check_md5); // 计算APP MD5值
        printf("APP MD5:%s\r\n", check_md5);
        if ((strcmp(ota_info->md5sum, check_md5) == 0) && (((*(vu32 *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000)) // 判断是否为0X08XXXXXX.
        {
            printf("APP MD5 OK\r\n");
            ota_info->file_size = 0;    // 清除固件
            iap_erase_all_bkp_sector(); // 擦除下载的OTA固件
            memcpy(ota_info->current_version, ota_info->ota_version, sizeof(ota_info->ota_version));
            USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);      // 关闭USART3接收缓冲区非空中断
            Get_OTA_Firmware(ota_info);                          // 上报固件版本,通知服务器OTA升级完成
            Write_CurrentFirmwareVersion(ota_info->ota_version); // 写入APP版本
            printf("Jump Run APP...\r\n");
            iap_load_app(FLASH_APP1_ADDR); // 执行FLASH APP代码
        }
        else
            printf("Update APP Failure\r\n");
    }
    else
        printf("OTA Firmware MD5 Error\r\n");
}

/*
函数功能：读取当前版本信息
参数：char *current_version
返回值：无
*/
void Read_CurrentFirmwareVersion(char *current_version)
{
    uint8_t *version_pt = (uint8_t *)FIRMWARE_VERSION_STORE_ADDR;
    for (uint8_t i = 0; i < FIRMWARE_VERSION_SIZE; i++)
    {
        if (*(version_pt + i) == 0xFF)
            continue;
        *(current_version + i) = *(version_pt + i);
    }
}

/*
函数功能：写入当前版本信息
参数：char *current_version
返回值：无
*/
void Write_CurrentFirmwareVersion(char *current_version)
{
    iap_write_flash(FIRMWARE_VERSION_STORE_ADDR, (uint8_t *)current_version, FIRMWARE_VERSION_SIZE);
}