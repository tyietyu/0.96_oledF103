#include "esp8266.h"
#include "usart.h"
#include "OLED.h"

extern uint8_t led_status;
extern uint8_t led_vol;

/*********************ESP8266_UART2********************************* */
extern UART_HandleTypeDef huart2;
ESP8266_UART_Buffer esp8266_uart_buff =
    {
        .receive_start = 0,
        .receive_count = 0,
        .receive_finish = 0,
};

void ESP8266_uart_printf(char *fmt, ...)
{
    va_list ap;
    uint16_t len;

    va_start(ap, fmt);
    vsprintf((char *)esp8266_uart_buff.send_buff, fmt, ap);
    va_end(ap);

    len = strlen((const char *)esp8266_uart_buff.send_buff);
    HAL_UART_Transmit(&huart2, esp8266_uart_buff.send_buff, len, HAL_MAX_DELAY);
}

void ESP8266_uart_rx_clear(uint16_t len)
{
    memset(esp8266_uart_buff.receive_buff, 0x00, len);
    esp8266_uart_buff.receive_count = 0;
    esp8266_uart_buff.receive_start = 0;
    esp8266_uart_buff.receive_finish = 0;
}

/*********************ESP8266 基本AT指令函数********************************* */

uint8_t ESP8266_init(void)
{
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE); // 启用UART接收中断

    // 检测AT命令是否可用，若不可用则返回错误
    return (ESP8266_at_test() == ESP8266_EOK) ? ESP8266_EOK : ESP8266_ERROR;
}

uint8_t ESP8266_send_at_cmd(unsigned char *cmd, unsigned char len, const char *ack)
{
    // 发送AT命令
    HAL_UART_Transmit(&huart2, cmd, len, 1000);

    // 等待接收开始信号
    for (unsigned int count = 0; count < 1000; count++)
    {
        if (esp8266_uart_buff.receive_start)
            break;
        HAL_Delay(1);
    }

    // 检查接收状态
    if (esp8266_uart_buff.receive_start == 0)
    {
        return 1; // 超时
    }

    // 等待接收完成
    for (unsigned int count = 0; count < 500; count++)
    {
        esp8266_uart_buff.receive_finish++;
        HAL_Delay(1);
    }

    // 检查接收到的响应
    if (strstr((const char *)esp8266_uart_buff.receive_buff, ack))
    {
        ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count); // 清除接收缓冲区
        return 0; // 成功
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count); // 清除接收缓冲区
    return 2; // 未找到确认
}

uint8_t ESP8266_sw_reset(void)
{
    // 发送软重置命令，并返回结果
    return ESP8266_send_at_cmd((uint8_t *)"AT+RST\r\n", "OK", 2000); // 可根据需要调整超时时间
}

uint8_t ESP8266_at_test(void)
{
    const char *at_cmd = "AT\r\n";
    const char *expected_ack = "OK";
    const uint8_t max_attempts = 10;

    for (uint8_t i = 0; i < max_attempts; i++)
    {
        if (ESP8266_send_at_cmd((uint8_t *)at_cmd, strlen(at_cmd), expected_ack) == ESP8266_EOK)
        {
            return ESP8266_EOK; // 成功响应
        }
    }

    return ESP8266_ERROR; // 超过最大尝试次数
}

uint8_t ESP8266_restore(void)
{
    // 发送恢复出厂设置命令，并返回结果
    return ESP8266_send_at_cmd((uint8_t *)"AT+RESTORE\r\n", "OK", 2000); // 可根据需要调整超时时间
}

uint8_t ESP8266_set_mode(uint8_t mode)
{
    const char *cmd_template = "AT+CWMODE=%d\r\n"; // AT命令模板
    char cmd[20]; // 命令缓冲区
    uint8_t ret;

    // 根据模式设置命令
    if (mode >= 1 && mode <= 3)
    {
        snprintf(cmd, sizeof(cmd), cmd_template, mode); // 格式化命令
        ret = ESP8266_send_at_cmd((uint8_t *)cmd, strlen(cmd), "OK");
    }
    else
    {
        return ESP8266_EINVAL; // 无效模式
    }

    return (ret == ESP8266_EOK) ? ESP8266_EOK : ESP8266_ERROR; // 返回结果
}

uint8_t ESP8266_ate_config(uint8_t cfg)
{
    const char *cmd;

    // 根据cfg选择相应的AT命令
    switch (cfg)
    {
    case 0:
        cmd = "ATE0\r\n"; // 关闭回显
        break;
    case 1:
        cmd = "ATE1\r\n"; // 打开回显
        break;
    default:
        return ESP8266_EINVAL; // 返回无效参数错误
    }

    // 发送AT命令，并检查返回值
    return ESP8266_send_at_cmd((uint8_t *)cmd, "OK", 1000);
}

uint8_t ESP8266_get_ip(char *buf, size_t buf_size)
{
    uint8_t ret;

    // 发送获取IP的AT命令
    ret = ESP8266_send_at_cmd((uint8_t *)"AT+CIFSR\r\n", strlen("AT+CIFSR\r\n"), "OK");
    if (ret != ESP8266_EOK)
    {
        return ESP8266_ERROR; // 返回错误
    }

    // 查找IP地址的开始和结束位置
    char *p_start = strstr((const char *)esp8266_uart_buff.receive_buff, "\"");
    if (p_start == NULL)
    {
        return ESP8266_ERROR; // 未找到开始引号
    }
    
    char *p_end = strstr(p_start + 1, "\"");
    if (p_end == NULL)
    {
        return ESP8266_ERROR; // 未找到结束引号
    }

    // 确保缓冲区足够大以容纳IP地址
    size_t ip_length = p_end - (p_start + 1);
    if (ip_length >= buf_size)
    {
        return ESP8266_ERROR; // 缓冲区不足
    }

    // 复制IP地址到buf
    strncpy(buf, p_start + 1, ip_length);
    buf[ip_length] = '\0'; // 确保字符串以NULL结束

    return ESP8266_EOK; // 成功
}

uint8_t ESP8266_enter_unvarnished(void)
{
    uint8_t ret;

    // 发送AT命令以进入未加工模式
    ret = ESP8266_send_at_cmd((uint8_t *)"AT+CIPMODE=1\r\n", strlen("AT+CIPMODE=1\r\n"), "OK");
    if (ret != ESP8266_EOK)
    {
        return ESP8266_ERROR; // 若第一个命令失败，直接返回错误
    }

    // 发送AT命令以开始发送数据
    ret = ESP8266_send_at_cmd((uint8_t *)"AT+CIPSEND\r\n", strlen("AT+CIPSEND\r\n"), ">");
    
    return (ret == ESP8266_EOK) ? ESP8266_EOK : ESP8266_ERROR; // 返回结果
}

void ESP8266_exit_unvarnished(void)
{
    ESP8266_uart_printf("+++");
}
/*********************ESP8266 链接WIFI&TCP服务函数**********************************/

uint8_t ESP8266_join_wifi(void)
{
    uint8_t retval = 1; // 默认为失败状态
    uint16_t count = 0;

    // 发送连接WiFi的AT命令
    char cmd[100]; // 预留足够的空间来构建AT命令
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWD);
    HAL_UART_Transmit(&huart2, (unsigned char *)cmd, strlen(cmd), 1000);

    // 等待接收缓冲区开始接收数据
    while ((esp8266_uart_buff.receive_start == 0) && (count < 1000))
    {
        count++;
        HAL_Delay(1);
    }
    // 检查是否超时
    if (count < 1000)
    {
        HAL_Delay(8000); // 等待连接WiFi的时间

        // 检查AT命令的返回结果
        if (strstr((const char *)esp8266_uart_buff.receive_buff, "OK"))
        {
            retval = 0; // 成功连接
        }
    }
    // 清理接收缓冲区
    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval; // 返回结果
}

uint8_t ESP8266_config_mqtt(void)
{
    uint8_t retval = 0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTUSERCFG=0,1,\"NULL""\",\"" MQTT_USER_NAME "\",\"" MQTT_PASSWD "\",0,0,\"\"\r\n",
                      strlen("AT+MQTTUSERCFG=0,1,\"NULL""\",\"" MQTT_USER_NAME "\",\"" MQTT_PASSWD "\",0,0,\"\"\r\n"),1000);
    while ((esp8266_uart_buff.receive_start == 0) && (count < 1000))
    {
        count++;
        HAL_Delay(1);
    }

    if (count >= 1000)
    {
        retval = 1;
    }
    else
    {
        if (strstr((const char *)esp8266_uart_buff.receive_buff, "OK"))
        {
            retval = 0;
        }
        else
        {
            retval = 1;
        }
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}

uint8_t ESP8266_connect_mqtt(void)
{
    uint8_t retval = 0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTCLIENTID=0,\"" MQTT_CLIENT_ID "\"\r\n",
                      strlen("AT+MQTTCLIENTID=0,\"" MQTT_CLIENT_ID "\"\r\n"), 1000);

    while ((esp8266_uart_buff.receive_start == 0) && (count < 1000))
    {
        count++;
        HAL_Delay(1);
    }

    if (count >= 1000)
    {
        retval = 1;
    }
    else
    {
        if (strstr((const char *)esp8266_uart_buff.receive_buff, "OK"))
        {
            retval = 0;
        }
        else
        {
            retval = 1;
        }
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}

uint8_t ESP8266_connect_tcp_server(void)
{
    uint8_t retval = 0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTCONN=0,\"" BROKER_ASDDRESS "\",1883,0\r\n", strlen("AT+MQTTCONN=0,\"" BROKER_ASDDRESS "\",1883,1\r\n"), 1000);

    while ((esp8266_uart_buff.receive_start == 0) && (count < 1000))
    {
        count++;
        HAL_Delay(1);
    }

    if (count >= 1000)
    {
        retval = 1;
    }
    else
    {
        if (strstr((const char *)esp8266_uart_buff.receive_buff, "OK"))
        {
            retval = 0;
        }
        else
        {
            retval = 1;
        }
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}

uint8_t ESP8266_Connect_Aliyun(void)
{
    while(ESP8266_config_mqtt() != 0)
    {
        HAL_Delay(1000);
    }
    while(ESP8266_connect_mqtt() != 0)
    {
        HAL_Delay(1000);
    }
    while(ESP8266_connect_tcp_server() != 0)
    {
        HAL_Delay(1000);
    }
    return 0;
}

uint8_t esp8266_send_msg(void)
{
    uint8_t retval = 0;
    uint16_t count = 0;
    static uint8_t error_count = 0;
    unsigned char msg_buf[256];

    sprintf((char *)msg_buf, "AT+MQTTPUB=0,\"" PUB_TOPIC "\",\"" JSON_FORMAT "\",1,0\r\n", led_vol, led_status);
    HAL_UART_Transmit(&huart2, (unsigned char *)msg_buf, strlen((const char *)msg_buf), 1000);
    HAL_UART_Transmit(&huart1, (unsigned char *)msg_buf, strlen((const char *)msg_buf), 1000);

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

/**
 * @brief          esp8266接收数据
 * @param[in]      none
 * @retval         返回0接收数据正常,返回1接收数据异常或无数据
 */
uint8_t esp8266_receive_msg(void)
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
            sscanf((const char *)esp8266_uart_buff.receive_buff, "+MQTTSUBRECV:0,\"" SUB_TOPIC "\",%d,%s", &msg_len, msg_body);
            //printf("len:%d,msg:%s\r\n", msg_len, msg_body);
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

uint8_t parse_json_msg(uint8_t *json_msg, uint8_t json_len)
{
    uint8_t retval = 0;
    JSONStatus_t result;
    char query[] = "params.light";
    size_t queryLength = sizeof(query) - 1;
    char *value;
    size_t valueLength;
    result = JSON_Validate((const char *)json_msg, json_len);

    if (result == JSONSuccess)
    {
        result = JSON_Search((char *)json_msg, json_len, query, queryLength, &value, &valueLength);

        if (result == JSONSuccess)
        {
            char save = value[valueLength];
            value[valueLength] = '\0';
            printf("Found: %s %d-> %s\n", query, valueLength, value);
            value[valueLength] = save;
            retval = 0;
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

    return retval;
}

uint8_t ESP8266_Sub_Pub_Topic_Aliyun(uint8_t subTopicMode)
{
    uint8_t retval = 0;
    switch (subTopicMode)
    {
    case 0: // 获取Aliyun IoT平台下发的消息
        retval = ESP8266_send_at_cmd((uint8_t *)"AT+MQTTSUB=0,\"" SUB_TOPIC "\",1\r\n", strlen("AT+MQTTSUB=0,\"" SUB_TOPIC "\",1\r\n"), "OK");
        return retval;
        break;
    case 1: // 上报设备信息到Aliyun IoT平台
        retval = ESP8266_send_at_cmd((uint8_t *)"AT+MQTTPUB=0,\"" PUB_TOPIC "\",1\r\n", strlen("AT+MQTPUB=0,\"" PUB_TOPIC "\",1\r\n"), "OK");
        return retval;
        break;
    case 2: // 设备上报固件升级信息
        retval = ESP8266_send_at_cmd((uint8_t *)"AT+MQTTPUB=0,\"" UPLOAD_INFORMATION_PUB "\",1\r\n", strlen("AT+MQTTPUB=0,\"" UPLOAD_INFORMATION_PUB "\",1\r\n"), "OK");
        return retval;
        break;
    case 3: // 固件升级信息下行
        retval = ESP8266_send_at_cmd((uint8_t *)"AT+MQTTSUB=0,\"" DOWNLOAD_INFORMATION_SUB "\",1\r\n", strlen("AT+MQTTSUB=0,\"" DOWNLOAD_INFORMATION_SUB "\",1\r\n"), "OK");
        return retval;
        break;
    case 4: // 设备主动拉取固件升级信息
        retval = ESP8266_send_at_cmd((uint8_t *)"AT+MQTTPUB=0,\"" DEVICE_ACTIVELY_INFORMATION_PUB "\",1\r\n", strlen("AT+MQTTPUB=0,\"" DEVICE_ACTIVELY_INFORMATION_PUB "\",1\r\n"), "OK");
        return retval;
        break;
    case 5: // 设备上报固件升级进度
        retval = ESP8266_send_at_cmd((uint8_t *)"AT+MQTTPUB=0,\"" DEVICE_REPORTS_PROGRESS_PUB "\",1\r\n", strlen("AT+MQTTPUB=0,\"" DEVICE_REPORTS_PROGRESS_PUB "\",1\r\n"), "OK");
        return retval;
        break;
    default:
        break;
    }
}

