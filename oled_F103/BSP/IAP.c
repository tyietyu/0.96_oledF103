#include "IAP.h"
#include "esp8266.h"
#include "usart.h"
#include "flash.h"

ESP8266_UART_Buffer esp8266_uart_buff;
MQTT_CB Aliyun_mqtt;
extern uint32_t BootStaFlag;  

void OTA_Init(void)
{
    ESP8266_sw_rese();
    ESP8266_set_mode(1);
    ESP8266_ate_config(0);
    ESP8266_join_wifi();
    ESP8266_Connect_Aliyun();
    // ESP8266_Sub_Pub_Topic_Aliyun(0);
}

/*-------------------------------------------------*/
/*函数名：处理mqtt的数据                          */
/*参  数：data：数据指针      datalen：数据长度    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void processing_mqtt_data(uint8_t *data, uint16_t datalen)
{
    #if 1
    ESP8266_Sub_Pub_Topic_Aliyun(0); // 订阅主题
    if ((datalen == 4) && (data[0] == 0x20))                                    // 如果接收到4个字节 且是 第一个字节是0x20，进入if
    {                                 
        printf("收到CONNACK报文\r\n");                                           // 串口发送数据
        if (data[3] == 0x00)
        {                                                                         // 判断第4个字节，如果是0x00，进入if
            printf("CONNECT报文成功连接服务器\r\n");                              // 串口发送数据
            BootStaFlag |= CONNECT_OK;                                            // 设置标志位，表示CONNECT报文成功
            MQTT_SubcribPack(DEVICE_ATTRIBUTES);                                    // 发送订阅Topic报文
            //MQTT_SubcribPack("/sys/a1HvxPcHnkX/D001/thing/file/download_reply");  // 发送订阅Topic报文
            OTA_Version();                                                        // 上传当前版本号
        }
        else
        {                                            // 判断第4个字节，如果不是0x00，进入else
            printf("CONNECT报文错误,准备重启\r\n"); // 串口发送数据
        }
    }
    #endif

    if ((datalen == 5) && (data[0] == 0x90))
    {                                 // 如果接收到5个字节 且是 第一个字节是0x90，进入if
        printf("收到SUBACK报文\r\n"); // 串口发送数据
        if ((data[datalen - 1] == 0x00) || (data[datalen - 1] == 0x01))
        {                                       // 判断接收的最后一个字节，如果是0x00或则0x01.进入if
            printf("SUBCRIBE订阅报文成功\r\n"); // 串口发送数据
        }
        else
        {                                                 // 判断接收的最后一个字节，如果不是0x00或则0x01.进入else
            printf("SUBCRIBE订阅报文错误,准备重启\r\n"); // 串口发送数据
        }
    }
    #if 0
    if ((BootStaFlag & CONNECT_OK) && (data[0] == 0x30))
    {                                           // 如果CONNECT报文成功 且是 第一个字节是0x30，进入if
        printf("收到等级0的PUBLISH报文\r\n");   // 串口发送数据
        MQTT_DealPublishData(data, datalen);    // 提取数据
        printf("%s\r\n", Aliyun_mqtt.CMD_buff); // 串口输出提取的数据
        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch1\":0}"))
        {                                     // 搜索关键词，如果搜索到进入if
            printf("关闭开关1\r\n");          // 串口发送数据
            gpio_bit_set(GPIOB, GPIO_PIN_12); // 高电平，熄灭LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch1\":0}}");
        }
        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch1\":1}"))
        {                                       // 搜索关键词，如果搜索到进入if
            printf("打开开关1\r\n");            // 串口发送数据
            gpio_bit_reset(GPIOB, GPIO_PIN_12); // 低电平，点亮LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch1\":1}}");
        }

        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch2\":0}"))
        {                                     // 搜索关键词，如果搜索到进入if
            printf("关闭开关2\r\n");          // 串口发送数据
            gpio_bit_set(GPIOB, GPIO_PIN_13); // 高电平，熄灭LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch2\":0}}");
        }
        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch2\":1}"))
        {                                       // 搜索关键词，如果搜索到进入if
            printf("打开开关2\r\n");            // 串口发送数据
            gpio_bit_reset(GPIOB, GPIO_PIN_13); // 低电平，点亮LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch2\":1}}");
        }

        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch3\":0}"))
        {                                     // 搜索关键词，如果搜索到进入if
            printf("关闭开关3\r\n");          // 串口发送数据
            gpio_bit_set(GPIOB, GPIO_PIN_14); // 高电平，熄灭LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch3\":0}}");
        }
        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch3\":1}"))
        {                                       // 搜索关键词，如果搜索到进入if
            printf("打开开关3\r\n");            // 串口发送数据
            gpio_bit_reset(GPIOB, GPIO_PIN_14); // 低电平，点亮LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch3\":1}}");
        }

        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch4\":0}"))
        {                                     // 搜索关键词，如果搜索到进入if
            printf("关闭开关4\r\n");          // 串口发送数据
            gpio_bit_set(GPIOB, GPIO_PIN_15); // 高电平，熄灭LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch4\":0}}");
        }
        if (strstr((char *)Aliyun_mqtt.CMD_buff, "{\"Switch4\":1}"))
        {                                       // 搜索关键词，如果搜索到进入if
            printf("打开开关4\r\n");            // 串口发送数据
            gpio_bit_reset(GPIOB, GPIO_PIN_15); // 低电平，点亮LED
            // 发送数据，同步开关状态
            MQTT_PublishDataQs0("/sys/a1HvxPcHnkX/D001/thing/event/property/post", "{\"params\":{\"Switch4\":1}}");
        }
        #endif

        // 搜索关键词，如果搜索到进入if
        if (strstr((char *)Aliyun_mqtt.CMD_buff, DOWNLOAD_INFORMATION_SUB ))
        {
            // 按格式提取关键数据成功，进入if
            if (sscanf((char *)Aliyun_mqtt.CMD_buff, "DOWNLOAD_INFORMATION_SUB {\"code\":\"1000\",\"data\":{\"size\":%d,\"streamId\":%d,\"sign\":\"%*32s\",\"dProtocol\":\"mqtt\",\"version\":\"%26s\",\"signMethod\":\"Md5\",\"streamFileId\":1,\"md5\":\"%*32s\"},\"id\":%*d,\"message\":\"success\"}", &Aliyun_mqtt.size, &Aliyun_mqtt.streamId, Aliyun_mqtt.OTA_tempver) == 3)
            {
                printf("OTA固件大小：%d\r\n", Aliyun_mqtt.size);          // 串口发送数据
                printf("OTA固件ID：%d\r\n", Aliyun_mqtt.streamId);        // 串口发送数据
                printf("OTA固件版本号：%s\r\n", Aliyun_mqtt.OTA_tempver); // 串口发送数据
                BootStaFlag |= OTA_EVENT;                                 // 置位标志位，OT发生
                //W25Q64_Erase64K(0);                                       // 清除0号块数据
                if (Aliyun_mqtt.size % 256 == 0)
                {                                                 // 判断固件大小是否是256的整数倍，是的话进入if
                    Aliyun_mqtt.counter = Aliyun_mqtt.size / 256; // 每次下载256字节，计算下载次数
                }
                else
                {                                                     // 判断固件大小是否是256的整数倍，不是的话进入else
                    Aliyun_mqtt.counter = Aliyun_mqtt.size / 256 + 1; // 每次下载256字节，计算下载次数
                }
                Aliyun_mqtt.num = 1;                                            // 当前第1次下载
                Aliyun_mqtt.downlen = 256;                                      // 记录本次下载量
                OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256); // 发送下载数据
            }
            else
            {                                      // 按格式提取关键数据失败，进入else
                printf("提取OTA下载命令错误\r\n"); // 串口发送数据
            }
        }
        // 搜索关键词，如果搜索到进入if
        if (strstr((char *)Aliyun_mqtt.CMD_buff, DEVICE_DOWNLOAD_FILE))
        {
            iap_write_flash(&data[datalen - Aliyun_mqtt.downlen - 2], Aliyun_mqtt.num - 1); // 保存本次下载的256字节
            Aliyun_mqtt.num++;                                                               // 当前下载次数+1
            if (Aliyun_mqtt.num < Aliyun_mqtt.counter)
            {                                                                   // 如果小于总下载次数，进入if
                Aliyun_mqtt.downlen = 256;                                      // 记录本次下载量
                OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256); // 发送下载数据
            }
            else if (Aliyun_mqtt.num == Aliyun_mqtt.counter)
            { // 如果等于总下载次数，进入if，说明是最后一次下载
                if (Aliyun_mqtt.size % 256 == 0)
                {                                                                   // 判断固件大小是否是256的整数倍，是的话进入if
                    Aliyun_mqtt.downlen = 256;                                      // 记录本次下载量
                    OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256); // 发送下载数据
                }
                else
                {                                                                   // 判断固件大小是否是256的整数倍，不是的话进入else
                    Aliyun_mqtt.downlen = Aliyun_mqtt.size % 256;                   // 记录本次下载量
                    OTA_Download(Aliyun_mqtt.downlen, (Aliyun_mqtt.num - 1) * 256); // 发送下载数据
                }
            }
            else
            {                                                          // 如果大于总下载次数，进入else，说明下载完毕
                printf("OTA下载完毕\r\n");                             // 串口发送数据
                memset(OTA_Info.OTA_ver, 0, 32);                       // 清除版本号缓冲区
                memcpy(OTA_Info.OTA_ver, Aliyun_mqtt.OTA_tempver, 26); // 拷贝版本号数据
                OTA_Info.Firelen[0] = Aliyun_mqtt.size;                // 记录固件大小
                OTA_Info.OTA_flag = OTA_SET_FLAG;                      // 设置标志位变量
                //M24C02_WriteOTAInfo();                                 // 保存到EEPROM
                //NVIC_SystemReset();                                    // 重启回B区更新
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
    MQTT_PublishDataQs1(UPLOAD_INFORMATION_PUB, temp); // 发送数据到服务器
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
    printf("当前第%d/%d次\r\n", Aliyun_mqtt.num, Aliyun_mqtt.counter);      // 串口输出数据
    MQTT_PublishDataQs0(DEVICE_DOWNLOAD_FILE_REPLY, temp); // 发送数据到服务器
    HAL_Delay(300);                                                          // 延时，阿里云限速，不能发太快
}

/*-------------------------------------------------*/
/*函数名：构建MQTT协议CONNECT报文                  */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MQTT_ConnectPack(void)
{
    Aliyun_mqtt.MessageID = 1;                                                                    // 初始化报文标识符变量，从1开始利用
    Aliyun_mqtt.Fixed_len = 1;                                                                    // 固定报头长度，暂定1
    Aliyun_mqtt.Variable_len = 10;                                                                // 可变报头长度，10
    Aliyun_mqtt.Payload_len = 2 + strlen(MQTT_CLIENT_ID) + 2 + strlen(MQTT_USER_NAME) + 2 + strlen(MQTT_PASSWD ); // 计算负载长度
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;               // 计算剩余长度

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

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 10] = strlen(MQTT_CLIENT_ID) / 256;             // 负载，MQTT_CLIENT_ID字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 11] = strlen(MQTT_CLIENT_ID) % 256;             // 负载，MQTT_CLIENT_ID字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12], MQTT_CLIENT_ID, strlen(MQTT_CLIENT_ID)); // 负载，拷贝MQTT_CLIENT_ID字符串

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12 + strlen(MQTT_CLIENT_ID)] = strlen(MQTT_USER_NAME) / 256;             // 负载，MQTT_USER_NAME字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 13 + strlen(MQTT_CLIENT_ID)] = strlen(MQTT_USER_NAME) % 256;             // 负载，MQTT_USER_NAME字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(MQTT_CLIENT_ID)], MQTT_USER_NAME, strlen(MQTT_USER_NAME)); // 负载，拷贝MQTT_USER_NAME字符串

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(MQTT_CLIENT_ID) + strlen(MQTT_USER_NAME)] = strlen(MQTT_PASSWD ) / 256;             // 负载，MQTT_PASSWD 字符串长度表示高字节
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 15 + strlen(MQTT_CLIENT_ID) + strlen(MQTT_USER_NAME)] = strlen(MQTT_PASSWD ) % 256;             // 负载，MQTT_PASSWD 字符串长度表示低字节
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 16 + strlen(MQTT_CLIENT_ID) + strlen(MQTT_USER_NAME)], MQTT_PASSWD , strlen(MQTT_PASSWD )); // 负载，拷贝MQTT_PASSWD 字符串

    //u2_sdata(Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len); // CONNECT报文数据加入发送缓冲区
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

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)] = 0; // 负载，订阅的Topic服务质量等级

    //u2_sdata(Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len); // Subcrib报文数据加入发送缓冲区
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

    //u2_sdata(Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len); // Publish报文数据加入发送缓冲区
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

    //u2_sdata(Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len); // Publish报文数据加入发送缓冲区
    HAL_UART_Transmit(&huart2, Aliyun_mqtt.Pack_buff, Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len, 1000);
}


