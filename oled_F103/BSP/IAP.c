#include "IAP.h"
#include "esp8266.h"
#include "usart.h"

volatile uint8_t OTA_Buff[2048];

void IAP_Init(void)
{
    ESP8266_sw_rese();
    ESP8266_set_mode(1);
    ESP8266_ate_config(0);
    ESP8266_join_wifi();
    ESP8266_config_mqtt();
    ESP8266_connect_Aliyun();
    ESP8266_connect_tcp_server();
    ESP8266_Sub_Pub_Topic_Aliyun(0);

}

uint8_t Device_Get_OTA_Firmware(void)
{
    uint8_t device_status = 0;
    if(ESP8266_connect_tcp_server()==0)
    {
        HAL_Delay(1000);
        if( ESP8266_Sub_Pub_Topic_Aliyun(2)==0)     //设备上传固件信息。
        {
            while(esp8266_send_firmware_version() != 0)
            {
                HAL_Delay(1000);
            }
            while (esp8266_receive_firmware_version() != 0)
            {
                HAL_Delay(1000);
            }
            
        }
    }
    else
    {
        
    }
}

uint8_t esp8266_send_firmware_version(void)
{
    uint8_t retval = 0;
    uint16_t count = 0;
    static uint8_t error_count = 0;
    unsigned char msg_buf[256];

    sprintf((char *)msg_buf, "AT+MQTTPUB=0,\""UPLOAD_INFORMATION_PUB"\",\""JSON_FORMAT_FIRMWARE"\",1,0\r\n");
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
                printf("RECONNECT MQTT BROKER!!!\r\n");
                ESP8266_init();
            }
        }
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}

uint8_t esp8266_receive_firmware_version(void)
{
    uint8_t retval = 0;
    int msg_len = 0;
    uint8_t msg_body[128] = {0};

    if (esp8266_uart_buff.receive_start == 1)
    {
        do
        {
            esp8266_uart_buff.receive_finish++;
            HAL_Delay(1);
        } while (esp8266_uart_buff.receive_finish < 5);

        if (strstr((const char *)esp8266_uart_buff.receive_buff, "+MQTTSUBRECV:"))
        {
            sscanf((const char *)esp8266_uart_buff.receive_buff, "+MQTTSUBRECV:0,\""DOWNLOAD_INFORMATION_SUB"\",%d,%s", &msg_len, msg_body);
            // printf("len:%d,msg:%s\r\n", msg_len, msg_body);

            if (strlen((const char *)msg_body) == msg_len)
            {
                retval = parse_json_msg(msg_body, msg_len);
            }
            else
            {
                retval = 1;
            }
        }
        else
        {
            retval = 1;
        }
    }
    else
    {
        retval = 1;
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}