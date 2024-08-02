#include "esp8266.h"
#include "usart.h"
#include "OLED.h"


/*********************ESP8266_UART2********************************* */
extern UART_HandleTypeDef huart2;
ESP8266_UART_Buffer esp8266_uart_buff=
{
    .receive_start=0,
    .receive_count=0,
    .receive_finish=0,
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
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    if (ESP8266_at_test() != ESP8266_EOK)
    {
        return ESP8266_ERROR;
    }

    return ESP8266_EOK;
}

uint8_t ESP8266_send_at_cmd(unsigned char *cmd, unsigned char len, char *ack)
{
    unsigned char retval =0;
    unsigned int count = 0;

    HAL_UART_Transmit(&huart2, cmd, len, 1000);

    while ((esp8266_uart_buff.receive_start == 0)&&(count<1000))
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
        do
        {
            esp8266_uart_buff.receive_finish++;
            HAL_Delay(1);
        } while (esp8266_uart_buff.receive_finish < 500);

        retval = 2;

        if (strstr((const char*)esp8266_uart_buff.receive_buff, ack))
        {
            retval = 0;
        }
    }

    ESP8266_uart_rx_clear(esp8266_uart_buff.receive_count);
    return retval;
}

uint8_t ESP8266_sw_reset(void)
{
    uint8_t ret;
    ret = ESP8266_send_at_cmd((uint8_t *)"AT+RST\r\n", strlen("AT+RST\r\n"), "OK");

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_at_test(void)
{
    uint8_t ret;
    uint8_t i;

    for (i = 0; i < 10; i++)
    {
        ret = ESP8266_send_at_cmd((uint8_t *)"AT\r\n", strlen("AT\r\n"), "OK");

        if (ret == ESP8266_EOK)
        {
            return ESP8266_EOK;
        }
    }

    return ESP8266_ERROR;
}

uint8_t ESP8266_restore(void)
{
    uint8_t ret;
    ret = ESP8266_send_at_cmd((uint8_t *)"AT+RESTORE\r\n", strlen("ready\r\n"), "OK");

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_set_mode(uint8_t mode)
{
    uint8_t ret;

    switch (mode)
    {
        case 1:
        {
            ret = ESP8266_send_at_cmd((uint8_t *)"AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"), "OK");    /* Station模式 */
            break;
        }

        case 2:
        {
            ret = ESP8266_send_at_cmd((uint8_t *)"AT+CWMODE=2\r\n", strlen("AT+CWMODE=2\r\n"), "OK");    /* AP模式 */
            break;
        }

        case 3:
        {
            ret = ESP8266_send_at_cmd((uint8_t *)"AT+CWMODE=3\r\n", strlen("AT+CWMODE=3\r\n"), "OK");    /* AP+Station模式 */
            break;
        }

        default:
        {
            return ESP8266_EINVAL;
        }
    }

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_ate_config(uint8_t cfg)
{
    uint8_t ret;

    switch (cfg)
    {
        case 0:
        {
            ret = ESP8266_send_at_cmd((uint8_t *)"ATE0\r\n", strlen("ATE0\r\n"), "OK");   /* 关闭回显 */
            break;
        }

        case 1:
        {
            ret = ESP8266_send_at_cmd((uint8_t *)"ATE1\r\n", strlen("ATE1\r\n"), "OK");   /* 打开回显 */
            break;
        }

        default:
        {
            return ESP8266_EINVAL;
        }
    }

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_get_ip(char *buf)
{
	uint8_t ret;
    char *p_start;
    char *p_end;
    
    ret =  ESP8266_send_at_cmd((uint8_t *)"AT+CIFSR\r\n",strlen("AT+CIFSR\r\n"),"OK");
    if (ret != ESP8266_EOK)
    {
        return ESP8266_ERROR;
    }
    
    p_start = strstr((const char *)esp8266_uart_buff.receive_buff, "\"");
    p_end = strstr(p_start + 1, "\"");
    *p_end = '\0';
    sprintf(buf, "%s", p_start + 1);
    return ESP8266_EOK;
}

uint8_t ESP8266_enter_unvarnished(void)
{
	 uint8_t ret;
    
    ret  =  ESP8266_send_at_cmd((uint8_t *)"AT+CIPMODE=1\r\n",strlen("AT+CIPMODE=1\r\n"), "OK");
    ret += ESP8266_send_at_cmd((uint8_t *)"AT+CIPSEND\r\n",strlen("AT+CIPSEND\r\n"), ">");
    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}	
void ESP8266_exit_unvarnished(void)
{
	ESP8266_uart_printf("+++");
}
/*********************ESP8266 链接WIFI&TCP服务函数**********************************/

uint8_t ESP8266_join_wifi(void)
{
    uint8_t retval =0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWD"\"\r\n", strlen("AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWD"\"\r\n"), 1000);

    while ((esp8266_uart_buff.receive_start == 0)&&(count<1000))
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
        HAL_Delay(8000);

        if (strstr((const char*)esp8266_uart_buff.receive_buff, "OK"))
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

uint8_t ESP8266_config_mqtt(void)
{
    uint8_t retval =0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTUSERCFG=0,1,\"NULL""\",\""MQTT_USER_NAME"\",\""MQTT_PASSWD"\",0,0,\"\"\r\n",
								strlen("AT+MQTTUSERCFG=0,1,\"NULL""\",\""MQTT_USER_NAME"\",\""MQTT_PASSWD"\",0,0,\"\"\r\n"), 1000);
	
    while ((esp8266_uart_buff.receive_start == 0)&&(count<1000))
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
        HAL_Delay(5000);

        if (strstr((const char*)esp8266_uart_buff.receive_buff, "OK"))
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
uint8_t ESP8266_get_mqttid(void)
{
	uint8_t retval =0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTCLIENTID=0,\""MQTT_CLIENT_ID"\"\r\n",
								strlen("AT+MQTTCLIENTID=0,\""MQTT_CLIENT_ID"\"\r\n"), 1000);
	
    while ((esp8266_uart_buff.receive_start == 0)&&(count<1000))
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
        HAL_Delay(5000);

        if (strstr((const char*)esp8266_uart_buff.receive_buff, "OK"))
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
    uint8_t retval=0;
    uint16_t count = 0;

    HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTCONN=0,\""BROKER_ASDDRESS"\",1883,0\r\n", strlen("AT+MQTTCONN=0,\""BROKER_ASDDRESS"\",1883,0\r\n"), 1000);

    while ((esp8266_uart_buff.receive_start == 0)&&(count<1000))
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
        HAL_Delay(5000);

        if (strstr((const char*)esp8266_uart_buff.receive_buff, "OK"))
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

uint8_t esp8266_send_msg(void)
{
    uint8_t retval =0;
    uint16_t count = 0;
    static uint8_t error_count=0;
    unsigned char msg_buf[256];

    HAL_UART_Transmit(&huart2, (unsigned char *)msg_buf, strlen((const char *)msg_buf), 1000);
    HAL_UART_Transmit(&huart1, (unsigned char *)msg_buf, strlen((const char *)msg_buf), 1000);

    while ((esp8266_uart_buff.receive_start == 0)&&(count<500))
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

        if (strstr((const char*)esp8266_uart_buff.receive_buff, "OK"))
        {
            retval = 0;
            error_count=0;
        }
        else
        {
            error_count++;

            if (error_count==5)
            {
                error_count=0;
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
    uint8_t retval =0;
    int msg_len=0;
    uint8_t msg_body[128] = {0};

    if (esp8266_uart_buff.receive_start == 1)
    {
        do
        {
            esp8266_uart_buff.receive_finish++;
            HAL_Delay(1);
        } while (esp8266_uart_buff.receive_finish < 5);

        if (strstr((const char*)esp8266_uart_buff.receive_buff, "+MQTTSUBRECV:"))
        {
            sscanf((const char *)esp8266_uart_buff.receive_buff, "+MQTTSUBRECV:0,\""SUB_TOPIC"\",%d,%s", &msg_len, msg_body);
            printf("len:%d,msg:%s\r\n", msg_len, msg_body);

            if (strlen((const char*)msg_body)== msg_len)
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
    uint8_t retval =0;
    JSONStatus_t result;
    char query[] = "params.light";
    size_t queryLength = sizeof(query) - 1;
    char * value;
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

uint8_t ESP8266_Topic_Aliyun_Theam(void)
{
    uint8_t retval =0;
    retval=ESP8266_send_at_cmd((uint8_t *)"AT+MQTTSUB=0,\""SUB_TOPIC"\",0\r\n",strlen("AT+MQTTSUB=0,\""SUB_TOPIC"\",0\r\n"),"OK");
    return retval;
}



