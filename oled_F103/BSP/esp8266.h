#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "main.h"
#include "core_json.h"

#define WIFI_SSID        "XiaomiPro"
#define WIFI_PASSWD      "123456789l"

#define MQTT_USER_NAME   "AT_MQTT&k1644sbngGw"
#define MQTT_CLIENT_ID   "k1644sbngGw.AT_MQTT|securemode=2\\,signmethod=hmacsha256\\,timestamp=1722604000001|"   
#define MQTT_PASSWD      "35d93f05a230fb364ac51438ed45f67c42d0c0fd4a2f792298297d106afdbad3"
#define BROKER_ASDDRESS  "k1644sbngGw.iot-as-mqtt.cn-shanghai.aliyuncs.com"

#define SUB_TOPIC        "/sys/k1644sbngGw/ESP_OTA/thing/service/property/set"
#define PUB_TOPIC        "/sys/k1644sbngGw/ESP_OTA/thing/event/property/post"
#define JSON_FORMAT      "{\\\"params\\\":{\\\"temp\\\":%d\\,\\\"humi\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

/* 错误代码 */
#define ESP8266_EOK         0   /* 没有错误 */
#define ESP8266_ERROR       1   /* 通用错误 */
#define ESP8266_ETIMEOUT    2   /* 超时错误 */
#define ESP8266_EINVAL      3   /* 参数错误 */

#define ESP8266_UART_RX_BUF_SIZE            128
#define ESP8266_UART_TX_BUF_SIZE            128

typedef struct {
    unsigned char receive_buff[ESP8266_UART_RX_BUF_SIZE];  // 接收缓冲区
    unsigned char send_buff[ESP8266_UART_TX_BUF_SIZE];     // 发送缓冲区
    uint8_t receive_start;                                 // 接收开始标志位
    uint16_t receive_count;                                // 接收计数器
    uint16_t receive_finish;                               // 接收结束标志位
} ESP8266_UART_Buffer;

extern ESP8266_UART_Buffer esp8266_uart_buff;

void ESP8266_uart_printf(char *fmt, ...);       
void ESP8266_uart_rx_clear(uint16_t len);             /* ESP8266 UART重新开始接收数据 */

/* 操作函数 */
uint8_t ESP8266_send_at_cmd(unsigned char *cmd,unsigned char len, char *ack);    /* ESP8266发送AT指令 */
uint8_t ESP8266_init(void);                                				/* ESP8266初始化 */
uint8_t ESP8266_restore(void);                                          /* ESP8266恢复出厂设置 */
uint8_t ESP8266_at_test(void);                                          /* ESP8266 AT指令测试 */
uint8_t ESP8266_set_mode(uint8_t mode);                                 /* 设置ESP8266工作模式 */
uint8_t ESP8266_sw_reset(void);                                         /* ESP8266软件复位 */
uint8_t ESP8266_ate_config(uint8_t cfg);                                /* ESP8266设置回显模式 */
uint8_t ESP8266_join_wifi(void);                         				/* ESP8266连接WIFI */
uint8_t ESP8266_get_ip(char *buf);                                      /* ESP8266获取IP地址 */
uint8_t ESP8266_config_mqtt(void);                                      /*ESP8266配置用户MQTT*/
uint8_t ESP8266_connect_Aliyun(void);
uint8_t ESP8266_connect_tcp_server(void); 								/* ESP8266连接TCP服务器 */
uint8_t ESP8266_enter_unvarnished(void);                                /* ESP8266进入透传 */
void ESP8266_exit_unvarnished(void);                                    /* ESP8266退出透传 */
uint8_t parse_json_msg(uint8_t *json_msg,uint8_t json_len); 
uint8_t ESP8266_Topic_Aliyun_Theam(void);

#endif

