#include "esp8266.h"
#include "core_json.h"
#include "usart.h"

extern UART_HandleTypeDef huart2;
// 初始化配置
esp8266_config_t esp8266_config = {
    .wifi = {
        .ssid = "XiaomiPro",
        .password = "123456789l"},
    .device_info = {
        .product_key = "k1644sbngGw", 
        .device_name = "AT_MQTT", 
        .device_secret = "1038f2eead281b6e90427a69d9cd532b"},
    .mqtt = {
        .username = "AT_MQTT&k1644sbngGw", 
        .client_id = "k1644sbngGw.AT_MQTT|securemode=2\\,signmethod=hmacsha256\\,timestamp=1722604000001|", 
        .password = "35d93f05a230fb364ac51438ed45f67c42d0c0fd4a2f792298297d106afdbad3", 
        .broker_address = "k1644sbngGw.iot-as-mqtt.cn-shanghai.aliyuncs.com"},
    .mqtt_topics = {
        .sub_topic = "/k1644sbngGw/AT_MQTT/user/get", 
        .pub_topic = "/sys/k1644sbngGw/AT_MQTT/thing/event/property/post", 
        .json_format = "{\"params\":{\"esp8266_adc_data\":%d,\"LED\":%d},\"version\":\"1.0.0\"}", 
        .json_format_firmware = "{\"id\":\"0011\",\"params\":{\"version\":\"1.0.0\"}}", 
        .device_attributes = "/sys/k1644sbngGw/AT_MQTT/thing/service/property/set"},
    .ota = {
        .upload_info_pub = "/ota/device/inform/k1644sbngGw/AT_MQTT", 
        .download_info_sub = "/ota/device/upgrade/k1644sbngGw/AT_MQTT", 
        .device_active_info_pub = "/sys/k1644sbngGw/AT_MQTT/thing/ota/firmware/get", 
        .device_report_progress_pub = "/ota/device/progress/k1644sbngGw/AT_MQTT", 
        .device_download_file = "/sys/k1644sbngGw/AT_MQTT/thing/file/download_reply", 
        .device_download_file_reply = "/sys/k1644sbngGw/AT_MQTT/thing/file/download"}
};

uart_init_t uart_init = {
    .uart_port = &huart2,
    .delay_ms = HAL_Delay,
    .uart_send = hal_uart_send,
    .uart_receive = hal_uart_receive,
    .esp8266_buffer.send_buff = {0},
    .esp8266_buffer.receive_buff = {0},
    .esp8266_buffer.receive_start = 0,
    .esp8266_buffer.receive_count = 0,
};

esp8266_init_t esp8266_init = {
    .init = ESP8266_init,
    .connect_wifi = ESP8266_connect_wifi,
    .login_to_cloud = ESP8266_connect_to_cloud,

};

void ESP8266_uart_rx_clear(uint16_t len)
{
    memset((void *)uart_init.esp8266_buffer.receive_buff, 0x00, len);
    uart_init.esp8266_buffer.receive_count = 0;
    uart_init.esp8266_buffer.receive_start = 0;
}

void hal_uart_send(uint8_t *data, size_t length)
{
    if (data == NULL || length == 0)
        return;
    HAL_UART_Transmit(&huart2, data, length, 1000);
}

void hal_uart_receive(uint8_t *data, size_t length)
{
    if (data == NULL || length == 0)
        return;
    HAL_UART_Receive(&huart2, data, length, 1000);
}

void hal_uart2_receiver_handle(void)
{
    uint8_t receive_data = 0;
    hal_uart_receive(&receive_data, sizeof(receive_data));
    uart_init.esp8266_buffer.receive_buff[uart_init.esp8266_buffer.receive_count++] = receive_data;
    if(receive_data == '\0')
    {
        uart_init.esp8266_buffer.receive_start = 1;
        uart_init.esp8266_buffer.receive_count = 0;
    }
}

uint8_t ESP8266_send_at_cmd(unsigned char *cmd, unsigned char len, const char *ack)
{
    uart_init.uart_send((unsigned char *)cmd, len);
    if(uart_init.esp8266_buffer.receive_start == 1)
    {
        if (strstr((const char *)uart_init.esp8266_buffer.receive_buff, ack))
        {
            ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);
            return ESP8266_EOK;
        }
    }
    else
    {
        printf("ESP8266_send_at_cmd error\r\n");
        ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);
        return ESP8266_ERROR;
    }
}

static uint8_t ESP8266_sw_reset(void)
{
    const char *cmd = "AT+RST\r\n";
    return ESP8266_send_at_cmd((unsigned char *)cmd, strlen(cmd), "OK"); // 可根据需要调整超时时间
}

static uint8_t ESP8266_restore(void)
{

    const char *cmd = "AT+RESTORE\r\n";
    return ESP8266_send_at_cmd((unsigned char *)cmd, strlen(cmd), "OK"); // 可根据需要调整超时时间
}

static uint8_t ESP8266_set_mode(uint8_t mode)
{
    const char *cmd_template = "AT+CWMODE=%d\r\n";
    char cmd[20];
    uint8_t ret;

    if (mode >= 1 && mode <= 3) // 根据模式设置命令
    {
        snprintf(cmd, sizeof(cmd), cmd_template, mode);
        ret = ESP8266_send_at_cmd((uint8_t *)cmd, strlen(cmd), "OK");
    }
    else
    {
        return ESP8266_EINVAL;
    }
    return (ret == ESP8266_EOK) ? ESP8266_EOK : ESP8266_ERROR;
}

static uint8_t ESP8266_ate_config(uint8_t cfg)
{
    const char *cmd;
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
    return ESP8266_send_at_cmd((unsigned char *)cmd, strlen(cmd), "OK");
}

static uint8_t ESP8266_set_unvarnished_mode(uint8_t enter)
{
    uint8_t ret;
    if (enter)
    {
        // 发送AT命令以进入透传模式
        ret = ESP8266_send_at_cmd((uint8_t *)"AT+CIPMODE=1\r\n", strlen("AT+CIPMODE=1\r\n"), "OK");
        if (ret != ESP8266_EOK)
        {
            return ESP8266_ERROR; // 若第一个命令失败，返回错误
        }
        ret = ESP8266_send_at_cmd((uint8_t *)"AT+CIPSEND\r\n", strlen("AT+CIPSEND\r\n"), ">");
    }
    else
    {
        char *cmd = "+++";
        hal_uart_send((uint8_t *)cmd, strlen(cmd));
        return ESP8266_EOK;
    }

    // 返回结果
    return (ret == ESP8266_EOK) ? ESP8266_EOK : ESP8266_ERROR;
}

uint8_t ESP8266_connect_wifi(const char *wifi_ssid, const char *password)
{
    uint8_t retval = 1; // 默认为失败状态
    uint16_t count = 0;

    // 发送连接WiFi的AT命令
    char cmd[100]; // 预留足够的空间来构建AT命令
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", wifi_ssid, password);
    uart_init.uart_send((unsigned char *)cmd, strlen(cmd));

    while ((uart_init.esp8266_buffer.receive_start == 0) && (count < 1000))
    {
        count++;
        uart_init.delay_ms(1);
    }
    // 检查是否超时
    if (count < 1000)
    {
        uart_init.delay_ms(5000); // 等待连接WiFi的时间
        if (strstr((const char *)uart_init.esp8266_buffer.receive_buff, "OK"))
        {
            retval = 0; // 成功连接
        }
    }
    // 清理接收缓冲区
    ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);
    return retval; // 返回结果
}

uint8_t ESP8266_connect_to_cloud(const char *mqtt_name, const char *mqtt_password, const char *mqtt_client_id, const char *broker_address)
{
    uint8_t retval = 0;
    uint16_t count = 0;

    // 1. 发送 MQTT 用户配置
    char cmd1[128];
    snprintf(cmd1, sizeof(cmd1), "AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"\r\n",
             mqtt_name, mqtt_password);
    uart_init.uart_send((unsigned char *)cmd1, strlen(cmd1));
    uart_init.delay_ms(10);

    count = 0;
    while ((uart_init.esp8266_buffer.receive_start == 0) && (count < 1000))
    {
        count++;
        uart_init.delay_ms(1);
    }
    if (count >= 1000 || strstr((const char *)uart_init.esp8266_buffer.receive_buff, "OK") == NULL)
    {
        return 1;
    }
    ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);

    // 2. 发送 MQTT 连接命令
    char cmd2[128];
    snprintf(cmd2, sizeof(cmd2), "AT+MQTTCONN=0,\"%s\"\r\n", mqtt_client_id);
    uart_init.uart_send((unsigned char *)cmd2, strlen(cmd2));

    count = 0;
    while ((uart_init.esp8266_buffer.receive_start == 0) && (count < 1000))
    {
        count++;
        uart_init.delay_ms(1);
    }
    if (count >= 1000 || strstr((const char *)uart_init.esp8266_buffer.receive_buff, "OK") == NULL)
    {
        return 1; // 返回错误
    }

    ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);

    // 3. 发送 TCP 连接命令
    char cmd3[128];
    snprintf(cmd3, sizeof(cmd3), "AT+MQTTCONN=0,\"%s\",1883,0\r\n", broker_address);
    uart_init.uart_send((unsigned char *)cmd3, strlen(cmd3));

    count = 0;
    while ((uart_init.esp8266_buffer.receive_start == 0) && (count < 1000))
    {
        count++;
        uart_init.delay_ms(1);
    }

    if (count >= 1000 || strstr((const char *)uart_init.esp8266_buffer.receive_buff, "OK") == NULL)
    {
        return 1;
    }
    ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);
    return retval;
}

uint8_t ESP8266_init(uint8_t esp8266_mode, uint8_t esp8266_cfg)
{
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    const char *at_cmd = "AT\r\n";
    const char *expected_ack = "OK";
    const uint8_t max_attempts = 10;

    for (uint8_t i = 0; i < max_attempts; i++)
    {
        if (ESP8266_send_at_cmd((uint8_t *)at_cmd, strlen(at_cmd), expected_ack) == ESP8266_EOK)
        {
            while (ESP8266_set_mode(esp8266_mode))
                ;
            while (ESP8266_ate_config(esp8266_cfg))
                ;
            while (ESP8266_connect_wifi(esp8266_config.wifi.ssid, esp8266_config.wifi.password))
                ;
            while (ESP8266_connect_to_cloud(esp8266_config.mqtt.username, esp8266_config.mqtt.password, esp8266_config.mqtt.client_id, esp8266_config.mqtt.broker_address))
                ;

            return ESP8266_EOK; // 成功响应
        }
    }
    return ESP8266_ERROR; // 超过最大尝试次数
}

uint8_t ESP8266_send_msg(const char *topic, const char *msg_format, ...)
{
    uint8_t retval = 0;
    uint16_t count = 0;
    static uint8_t error_count = 0;
    unsigned char msg_buf[128];

    va_list args;
    va_start(args, msg_format);
    vsnprintf((char *)msg_buf, sizeof(msg_buf), msg_format, args);
    snprintf((char *)msg_buf, sizeof(msg_buf), "AT+MQTTPUB=0,\"%s\",\"%s\",1,0\r\n", topic, (const char *)msg_buf);
    va_end(args);
    uart_init.uart_send((uint8_t *)msg_buf, strlen((const char *)msg_buf));

    while ((uart_init.esp8266_buffer.receive_start == 0) && (count < 500))
    {
        count++;
        uart_init.delay_ms(1);
    }

    if (count >= 500)
    {
        retval = 1;
    }
    else
    {
        uart_init.delay_ms(50);

        if (strstr((const char *)uart_init.esp8266_buffer.receive_buff, "OK"))
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
            }
        }
    }

    ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);
    return retval;
}

static uint8_t parse_json_msg(uint8_t *json_msg, uint8_t json_len)
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

uint8_t ESP8266_receive_msg(const char *topic, uint8_t *msg_data, uint16_t msg_len)
{
    uint8_t retval = 0;
    if (uart_init.esp8266_buffer.receive_start == 1)
    {
        do
        {
            uart_init.esp8266_buffer.receive_finish++;
        } while (uart_init.esp8266_buffer.receive_finish < 5);

        if (strstr((const char *)uart_init.esp8266_buffer.receive_buff, "+MQTTSUBRECV:"))
        {
            sscanf((const char *)uart_init.esp8266_buffer.receive_buff, "+MQTTSUBRECV:0,\"%s\",%d,%s", topic, msg_data, msg_len);
            // printf("len:%d,msg:%s\r\n", msg_len, msg_body);
            if (strlen((const char *)msg_data) == msg_len)
            {
                retval = parse_json_msg(msg_data, msg_len);
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
    ESP8266_uart_rx_clear(uart_init.esp8266_buffer.receive_count);
    return retval;
}
