#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "main.h"

#define ESP8266_RECEIVE_BUFF 512
#define ESP8266_SEND_BUFF 512

/* 引脚定义 */
#define ESP8266_UART_TX_GPIO_PORT           GPIOA
#define ESP8266_UART_TX_GPIO_PIN            GPIO_PIN_2
#define ESP8266_UART_TX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)     /* PB口时钟使能 */

#define ESP8266_UART_RX_GPIO_PORT           GPIOA
#define ESP8266_UART_RX_GPIO_PIN            GPIO_PIN_3
#define ESP8266_UART_RX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)     /* PB口时钟使能 */

#define ESP8266_UART_INTERFACE              USART2

/* UART收发缓冲大小 */
#define ESP8266_UART_RX_BUF_SIZE            128
#define ESP8266_UART_TX_BUF_SIZE            64

#define WIFI_SSID        "RavSense"
#define WIFI_PASSWD      "rwwdz123456"

#define MQTT_CLIENT_ID   "mqtt_stm32|securemode=2\\,signmethod=hmacsha1\\,timestamp=1687594902069|"   
#define MQTT_USER_NAME   "mqtt_stm32&a1TGt6tIcAE"
#define MQTT_PASSWD      "556483AFA86B8FF534E3DB0A14EE7A36D2910B2D"
#define BROKER_ASDDRESS  "a1TGt6tIcAE.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define SUB_TOPIC        "/sys/a1TGt6tIcAE/mqtt_stm32/thing/service/property/set"
#define PUB_TOPIC        "/sys/a1TGt6tIcAE/mqtt_stm32/thing/event/property/post"
#define JSON_FORMAT      "{\\\"params\\\":{\\\"temp\\\":%d\\,\\\"humi\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

/* 错误代码 */
#define ESP8266_EOK         0   /* 没有错误 */
#define ESP8266_ERROR       1   /* 通用错误 */
#define ESP8266_ETIMEOUT    2   /* 超时错误 */
#define ESP8266_EINVAL      3   /* 参数错误 */

void ESP8266_uart_printf(char *fmt, ...);       
void ESP8266_uart_rx_restart(void);             /* ESP8266 UART重新开始接收数据 */
uint8_t *ESP8266_uart_rx_get_frame(void);       /* 获取ESP8266 UART接收到的一帧数据 */
uint16_t ESP8266_uart_rx_get_frame_len(void);   /* 获取ESP8266 UART接收到的一帧数据的长度 */
void ESP8266_uart_init(uint32_t baudrate);      /* ESP8266 UART初始化 */
void ESP8266_usart2Hander(void);

/* 操作函数 */
uint8_t ESP8266_send_at_cmd(char *cmd, char *ack, uint32_t timeout);    /* ESP8266发送AT指令 */
uint8_t ESP8266_init(uint32_t baudrate);                                /* ESP8266初始化 */
uint8_t ESP8266_restore(void);                                          /* ESP8266恢复出厂设置 */
uint8_t ESP8266_at_test(void);                                          /* ESP8266 AT指令测试 */
uint8_t ESP8266_set_mode(uint8_t mode);                                 /* 设置ESP8266工作模式 */
uint8_t ESP8266_sw_reset(void);                                         /* ESP8266软件复位 */
uint8_t ESP8266_ate_config(uint8_t cfg);                                /* ESP8266设置回显模式 */
uint8_t ESP8266_join_ap(char *ssid, char *pwd);                         /* ESP8266连接WIFI */
uint8_t ESP8266_get_ip(char *buf);                                      /* ESP8266获取IP地址 */
uint8_t ESP8266_connect_tcp_server(char *server_ip, char *server_port); /* ESP8266连接TCP服务器 */
uint8_t ESP8266_enter_unvarnished(void);                                /* ESP8266进入透传 */
void ESP8266_exit_unvarnished(void);                                    /* ESP8266退出透传 */
uint8_t ESP8266_connect_Ali_Clode(char *id, char *pwd);                    /* ESP8266连接原子云服务器 */
uint8_t ESP8266_disconnect_Ali_Clode(void);  

#endif

