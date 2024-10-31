#include "IAP.h"
#include "esp8266.h"
#include "usart.h"
#include "flash.h"

extern ESP8266_UART_Buffer esp8266_uart_buff;
MQTT_CB Aliyun_mqtt;
OTA_InfoCB OTA_Info;
extern uint32_t BootStaFlag;

void OTA_Init(void)
{
    ESP8266_sw_reset();
    ESP8266_set_mode(1);
    ESP8266_ate_config(0);
    ESP8266_join_wifi();
    ESP8266_Connect_Aliyun();
    ESP8266_Sub_Pub_Topic_Aliyun(0);
}

/*
 * 函数名：OTA升级处理函数
 * 参  数：无
 */
void OTA_Deal_MQTT_Data(uint8_t *data, uint16_t datalen)
{
    if ((datalen == 4) && (data[0] == 0x20)) // 如果接收到4个字节 且是 第一个字节是0x20，进入if
    {
        printf("收到CONNACK报文\r\n"); // 串口发送数据
        if (data[3] == 0x00)
        {                                            // 判断第4个字节，如果是0x00，进入if
            printf("CONNECT报文成功连接服务器\r\n"); // 串口发送数据
            BootStaFlag |= CONNECT_OK;               // 设置标志位，表示CONNECT报文成功
            MQTT_SubcribPack(DEVICE_ATTRIBUTES);     // 发送订阅Topic报文
            OTA_Version();                           // 上传当前版本号
        }
        else
        {                                           // 判断第4个字节，如果不是0x00，进入else
            printf("CONNECT报文错误,准备重启\r\n"); // 串口发送数据
        }
    }

    if ((datalen == 5) && (data[0] == 0x90))
    {
        printf("收到SUBACK报文\r\n"); // 串口发送数据
        if ((data[datalen - 1] == 0x00) || (data[datalen - 1] == 0x01))
        {
            printf("SUBCRIBE订阅报文成功\r\n");
        }
        else
        {
            printf("SUBCRIBE订阅报文错误,准备重启\r\n");
            HAL_NVIC_SystemReset();
        }
    }

    if ((BootStaFlag & CONNECT_OK) && (data[0] == 0x30))
    {
        printf("收到等级0的PUBLISH报文\r\n");
        MQTT_DealPublishData(data, datalen);
        printf("%s\r\n", Aliyun_mqtt.CMD_buff);

        if (strstr((char *)Aliyun_mqtt.CMD_buff, DOWNLOAD_INFORMATION_SUB))
        {
            if (sscanf((char *)Aliyun_mqtt.CMD_buff, "/ota/device/upgrade/k1644sbngGw/AT_MQTT{\"code\":\"1000\",\"data\":{\"size\":%d,\"streamId\":%d,\"sign\":\"%*32s\",\"dProtocol\":\"mqtt\",\"version\":\"%26s\",\"signMethod\":\"Md5\",\"streamFileId\":1,\"md5\":\"%*32s\"},\"id\":%*d,\"message\":\"success\"}", &Aliyun_mqtt.size, &Aliyun_mqtt.streamId, Aliyun_mqtt.OTA_tempver) == 3)
            {
                printf("OTA固件大小:%d\r\n", Aliyun_mqtt.size);
                printf("OTA固件ID:%d\r\n", Aliyun_mqtt.streamId);
                printf("OTA固件版本号:%s\r\n", Aliyun_mqtt.OTA_tempver);
                BootStaFlag |= OTA_EVENT;
                if (Aliyun_mqtt.size % 256 == 0)
                {
                    Aliyun_mqtt.counter = Aliyun_mqtt.size / 256;
                }
                else
                {
                    Aliyun_mqtt.counter = Aliyun_mqtt.size / 256 + 1;
                }
                Aliyun_mqtt.num = 1;
                Aliyun_mqtt.downlen = 256;
                OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256);
            }
            else
            {
                printf("OTA固件信息解析错误\r\n");
            }
        }
        if (strstr((char *)Aliyun_mqtt.CMD_buff, DEVICE_DOWNLOAD_FILE_REPLY))
        {
            uint16_t temp[(Aliyun_mqtt.num - 1 + 1) / 2]; // +1 和 /2 是为了向上取整
            for (int i = 0; i < Aliyun_mqtt.num - 1; i += 2)
            {
                temp[i / 2] = (data[datalen - Aliyun_mqtt.downlen - 2 + i] << 8) | data[datalen - Aliyun_mqtt.downlen - 2 + i + 1];
            }
            iap_write_flash(OTA_PACK_ADDERS, temp, Aliyun_mqtt.num - 1);
            Aliyun_mqtt.num++;
            if (Aliyun_mqtt.num < Aliyun_mqtt.counter)
            {
                Aliyun_mqtt.downlen = 256;
                OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256);
            }
            else if (Aliyun_mqtt.num == Aliyun_mqtt.counter)
            {
                if (Aliyun_mqtt.size % 256 == 0)
                {
                    Aliyun_mqtt.downlen = 256;
                    OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256);
                }
                else
                {
                    Aliyun_mqtt.downlen = Aliyun_mqtt.size % 256;
                    OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256);
                }
            }
            else
            {
                printf("OTA固件下载完成\r\n");
                memset(OTA_Info.OTA_ver, 0, 32);
                memcpy(OTA_Info.OTA_ver, Aliyun_mqtt.OTA_tempver, strlen((const char *)Aliyun_mqtt.OTA_tempver));
                OTA_Info.Firelen[0] = Aliyun_mqtt.size;
                OTA_Info.OTA_flag = OTA_SET_FLAG;
                HAL_NVIC_SystemReset();
            }
        }
    }
}
/*-------------------------------------------------*/
/*函数名：上传OTA版本号                            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void OTA_Version(void)
{
    char temp[128];

    memset(temp, 0, 128);                                                                 // 清空缓冲区
    sprintf(temp, "{\"id\": \"1\",\"params\": {\"version\": \"%s\"}}", OTA_Info.OTA_ver); // 构建数据
    MQTT_PublishDataQs1(UPLOAD_INFORMATION_PUB, temp);                                    // 发送数据到服务器
}
/*-------------------------------------------------*/
/*函数名：OTA下载数据                              */
/*参  数：size：本次下载量                         */
/*参  数：offset：本次下载偏移量                   */
/*返回值：无                                       */
/*-------------------------------------------------*/
void OTA_Download(int size, int offset)
{
    char temp[256];

    memset(temp, 0, 256); // 清空缓冲区
    // 构建数据
    sprintf(temp, "{\"id\": \"1\",\"params\": {\"fileInfo\":{\"streamId\":%d,\"fileId\":1},\"fileBlock\":{\"size\":%d,\"offset\":%d}}}", Aliyun_mqtt.streamId, size, offset);
    printf("当前第%d/%d次\r\n", Aliyun_mqtt.num, Aliyun_mqtt.counter); // 串口输出数据
    MQTT_PublishDataQs0(DEVICE_DOWNLOAD_FILE_REPLY, temp);             // 发送数据到服务器
    HAL_Delay(300);                                                    // 延时，阿里云限速，不能发太快
}

/*-------------------------------------------------*/
/*函数名：构建MQTT协议CONNECT报文                  */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_ConnectPack(void)
{
    Aliyun_mqtt.MessageID = 1;                                                                                   // 初始化报文标识符变量，从1开始利用
    Aliyun_mqtt.Fixed_len = 1;                                                                                   // 固定报头长度，暂定1
    Aliyun_mqtt.Variable_len = 10;                                                                               // 可变报头长度，10
    Aliyun_mqtt.Payload_len = 2 + strlen(MQTT_CLIENT_ID) + 2 + strlen(MQTT_USER_NAME) + 2 + strlen(MQTT_PASSWD); // 计算负载长度
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;                              // 计算剩余长度

    Aliyun_mqtt.Pack_buff[0] = 0x10; // CONNECT报文固定报头第1个字节，0x01

    do
    {
        if (Aliyun_mqtt.Remaining_len / 128 == 0) // 判断剩余长度是否够128，如果不够，进入if
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len; // 记录数值
        }
        else // 判断剩余长度是否够128，如果够，进入else，需要向前进一个字节
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len % 128) | 0x80; // 记录数值，并置位BIT7，表示需要向前进一个字节
        }

        Aliyun_mqtt.Fixed_len++;                                     // 固定报头长度+1
        Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len / 128; // 取整128
    } while (Aliyun_mqtt.Remaining_len); // 如果不是0，则再一次循环，如果是0就退出

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = 0x00; // CONNECT报文可变报头：0x00
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = 0x04; // CONNECT报文可变报头：0x04
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = 0x4D; // CONNECT报文可变报头：0x4D
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = 0x51; // CONNECT报文可变报头：0x51
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4] = 0x54; // CONNECT报文可变报头：0x54
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 5] = 0x54; // CONNECT报文可变报头：0x54
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 6] = 0x04; // CONNECT报文可变报头：0x04
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 7] = 0xC2; // CONNECT报文可变报头：0xC2
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 8] = 0x00; // CONNECT报文可变报头：0x00
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 9] = 0x64; // CONNECT报文可变报头：0x64

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 10] = strlen(MQTT_CLIENT_ID) / 256;                   // 负载，MQTT_CLIENT_ID字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 11] = strlen(MQTT_CLIENT_ID) % 256;                   // 负载，MQTT_CLIENT_ID字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12], MQTT_CLIENT_ID, strlen(MQTT_CLIENT_ID)); // 负载，拷贝MQTT_CLIENT_ID字符串

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12 + strlen(MQTT_CLIENT_ID)] = strlen(MQTT_USER_NAME) / 256;                   // 负载，MQTT_USER_NAME字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 13 + strlen(MQTT_CLIENT_ID)] = strlen(MQTT_USER_NAME) % 256;                   // 负载，MQTT_USER_NAME字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(MQTT_CLIENT_ID)], MQTT_USER_NAME, strlen(MQTT_USER_NAME)); // 负载，拷贝MQTT_USER_NAME字符串

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(MQTT_CLIENT_ID) + strlen(MQTT_USER_NAME)] = strlen(MQTT_PASSWD) / 256;                // 负载，MQTT_PASSWD 字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 15 + strlen(MQTT_CLIENT_ID) + strlen(MQTT_USER_NAME)] = strlen(MQTT_PASSWD) % 256;                // 负载，MQTT_PASSWD 字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 16 + strlen(MQTT_CLIENT_ID) + strlen(MQTT_USER_NAME)], MQTT_PASSWD, strlen(MQTT_PASSWD)); // 负载，拷贝MQTT_PASSWD 字符串

    HAL_UART_Transmit(&huart2, Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len, 1000);
}

/*-------------------------------------------------*/
/*函数名：构建MQTT协议Subcrib报文                  */
/*参  数：topic：需要订阅的主题Topic               */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_SubcribPack(char *topic)
{
    Aliyun_mqtt.Fixed_len = 1;                                                      // 固定报头长度，暂定1
    Aliyun_mqtt.Variable_len = 2;                                                   // 可变报头长度，2
    Aliyun_mqtt.Payload_len = 2 + strlen(topic) + 1;                                // 计算负载长度
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len; // 计算剩余长度

    Aliyun_mqtt.Pack_buff[0] = 0x82; // Subcrib报文固定报头第1个字节，0x82

    do
    {
        if (Aliyun_mqtt.Remaining_len / 128 == 0) // 判断剩余长度是否够128，如果不够，进入if
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len; // 记录数值
        }
        else // 判断剩余长度是否够128，如果够，进入else，需要向前进一个字节
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len % 128) | 0x80; // 记录数值，并置位BIT7，表示需要向前进一个字节
        }

        Aliyun_mqtt.Fixed_len++;                                     // 固定报头长度+1
        Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len / 128; // 取整128
    } while (Aliyun_mqtt.Remaining_len); // 如果不是0，则再一次循环，如果是0就退出

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = Aliyun_mqtt.MessageID / 256; // 可变报头，报文标识符高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = Aliyun_mqtt.MessageID % 256; // 可变报头，报文标识符低字节
    Aliyun_mqtt.MessageID++;                                                        // 报文标识符变量+1

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = strlen(topic) / 256;          // 负载，订阅的Topic字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = strlen(topic) % 256;          // 负载，订阅的Topic字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4], topic, strlen(topic)); // 负载，拷贝订阅的Topic字符串

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)] = 0;                                                                // 负载，订阅的Topic服务质量等级
    HAL_UART_Transmit(&huart2, Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len, 1000); // 发送数据
}

/*-------------------------------------------------*/
/*函数名：处理服务器推送的Publish报文              */
/*参  数：data：数据                               */
/*参  数：data_len：数据长度                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_DealPublishData(uint8_t *data, uint16_t data_len)
{
    uint8_t i;

    for (i = 1; i < 5; i++) // 最多循环4次，判断剩余长度占用几个字节
    {
        if ((data[i] & 0x80) == 0) // 如果BIT7不是1，进入if
            break;                 // 退出for循环，此时i的值就是剩余长度占用的字节数
    }

    memset(Aliyun_mqtt.CMD_buff, 0, 512);                                 // 清空缓冲区
    memcpy(Aliyun_mqtt.CMD_buff, &data[1 + i + 2], data_len - 1 - i - 2); // 拷贝数据到缓冲区
}

/*-------------------------------------------------*/
/*函数名：向服务器发送等级0的Publish报文           */
/*参  数：topic：需要发送数据的Topic               */
/*参  数：data：数据                               */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_PublishDataQs0(char *topic, char *data)
{
    Aliyun_mqtt.Fixed_len = 1;                                                      // 固定报头长度，暂定1
    Aliyun_mqtt.Variable_len = 2 + strlen(topic);                                   // 计算可变报头长度
    Aliyun_mqtt.Payload_len = strlen(data);                                         // 计算负载长度
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len; // 计算剩余长度长度

    Aliyun_mqtt.Pack_buff[0] = 0x30; // 等级0的Publish报文固定报头第1个字节，0x30

    do
    {
        if (Aliyun_mqtt.Remaining_len / 128 == 0) // 判断剩余长度是否够128，如果不够，进入if
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len; // 记录数值
        }
        else // 判断剩余长度是否够128，如果够，进入else，需要向前进一个字节
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len % 128) | 0x80; // 记录数值，并置位BIT7，表示需要向前进一个字节
        }

        Aliyun_mqtt.Fixed_len++;                                     // 固定报头长度+1
        Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len / 128; // 取整128
    } while (Aliyun_mqtt.Remaining_len); // 如果不是0，则再一次循环，如果是0就退出

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = strlen(topic) / 256;                        // 可变报头，发送数据的Topic字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = strlen(topic) % 256;                        // 可变报头，发送数据的Topic字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2], topic, strlen(topic));               // 可变报头，拷贝发送数据的Topic字符串
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2 + strlen(topic)], data, strlen(data)); // 负载，拷贝发送的数据

    HAL_UART_Transmit(&huart2, Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len, 1000);
}

/*-------------------------------------------------*/
/*函数名：向服务器发送等级1的Publish报文           */
/*参  数：topic：需要发送数据的Topic               */
/*参  数：data：数据                               */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_PublishDataQs1(char *topic, char *data)
{
    Aliyun_mqtt.Fixed_len = 1;                                                      // 固定报头长度，暂定1
    Aliyun_mqtt.Variable_len = 2 + 2 + strlen(topic);                               // 计算可变报头长度
    Aliyun_mqtt.Payload_len = strlen(data);                                         // 计算负载长度
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len; // 计算剩余长度长度

    Aliyun_mqtt.Pack_buff[0] = 0x32; // 等级1的Publish报文固定报头第1个字节，0x32

    do
    {
        if (Aliyun_mqtt.Remaining_len / 128 == 0) // 判断剩余长度是否够128，如果不够，进入if
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len; // 记录数值
        }
        else // 判断剩余长度是否够128，如果够，进入else，需要向前进一个字节
        {
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len % 128) | 0x80; // 记录数值，并置位BIT7，表示需要向前进一个字节
        }

        Aliyun_mqtt.Fixed_len++;                                     // 固定报头长度+1
        Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len / 128; // 取整128
    } while (Aliyun_mqtt.Remaining_len); // 如果不是0，则再一次循环，如果是0就退出

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = strlen(topic) / 256;          // 可变报头，发送数据的Topic字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = strlen(topic) % 256;          // 可变报头，发送数据的Topic字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2], topic, strlen(topic)); // 可变报头，拷贝发送数据的Topic字符串

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2 + strlen(topic)] = Aliyun_mqtt.MessageID / 256; // 可变报头，报文标识符高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3 + strlen(topic)] = Aliyun_mqtt.MessageID % 256; // 可变报头，报文标识符低字节
    Aliyun_mqtt.MessageID++;                                                                        // 报文标识符变量+1

    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)], data, strlen(data)); // 负载，拷贝发送的数据
    HAL_UART_Transmit(&huart2, Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len, 1000);
}
