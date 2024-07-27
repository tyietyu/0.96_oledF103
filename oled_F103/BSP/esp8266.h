#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "main.h"

#define ESP8266_RECEIVE_BUFF 512
#define ESP8266_SEND_BUFF 512

#define WIFI_SSID        "RavSense"
#define WIFI_PASSWD      "rwwdz123456"

#define MQTT_CLIENT_ID   "mqtt_stm32|securemode=2\\,signmethod=hmacsha1\\,timestamp=1687594902069|"   
#define MQTT_USER_NAME   "mqtt_stm32&a1TGt6tIcAE"
#define MQTT_PASSWD      "556483AFA86B8FF534E3DB0A14EE7A36D2910B2D"
#define BROKER_ASDDRESS  "a1TGt6tIcAE.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define SUB_TOPIC        "/sys/a1TGt6tIcAE/mqtt_stm32/thing/service/property/set"
#define PUB_TOPIC        "/sys/a1TGt6tIcAE/mqtt_stm32/thing/event/property/post"
#define JSON_FORMAT      "{\\\"params\\\":{\\\"temp\\\":%d\\,\\\"humi\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"


void uart2_receiver_handle(void);
uint8_t esp8266_receive_msg(void);
uint8_t esp8266_send_msg(void);
void esp8266_init(void);
#endif

