#ifndef __IAP_H
#define __IAP_H

#include "main.h"

#define PACK_BUFF_SIZE  512
#define CMD_BUFF_SIZE   512

#define  CONNECT_OK         0x00000001        //置位表明CONNECT报文成功
#define  OTA_EVENT          0x00000002        //置位表明OTA事件发生  
#define  OTA_SET_FLAG        0xAABB1122        //OTA_flag对勾状态对应的数值，如果OTA_flag等于该值，说明需要OTA更新A区

#define OTA_PACK_ADDERS   0x08002000        //OTA固件起始地址

typedef struct{
	uint8_t   Pack_buff[PACK_BUFF_SIZE];     //报文数据缓冲区
	uint16_t  MessageID;          //报文标识符变量
	uint16_t  Fixed_len;          //报文固定报头长度
	uint16_t  Variable_len;       //报文可变报头长度
	uint16_t  Payload_len;        //报文负载长度
	uint16_t  Remaining_len;      //报文剩余长度
	uint8_t   CMD_buff[CMD_BUFF_SIZE];      //提取的数据缓冲区
	int size;                     //OTA下载固件大小
	int streamId;                 //OTA下载固件ID编号
	int counter;                  //OTA下载总共下载次数
	int num;                      //OTA下载当前次数
	int downlen;                  //OTA下载当前次数下载量
	uint8_t  OTA_tempver[32];     //OTA下载临时版本号缓冲区
}MQTT_CB;

volatile uint16_t mqtt_receive_count = 0;
volatile uint8_t mqtt_receive_complete = 0;

typedef struct{          
	uint32_t OTA_flag;                        //标志性的变量，等于OTA_SET_FLAG定义的值时，表明需要OTA更新A区
	uint32_t Firelen[11];                     //W25Q64中不同块中程序固件的长度，0号成员固定对应W25Q64中编码0的块，用于OTA
	uint8_t  OTA_ver[32];
}OTA_InfoCB;  
extern OTA_InfoCB  OTA_Info;      //外部变量声明
extern MQTT_CB  Aliyun_mqtt;      //外部变量声明                            

void processing_mqtt_data(uint8_t *data, uint16_t datalen);  //函数声明
void OTA_Version(void);                           //函数声明
void OTA_Download(int size, int offset);          //函数声明

void MQTT_ConnectPack(void);                                  //函数声明
void MQTT_SubcribPack(char *topic);                           //函数声明
void MQTT_DealPublishData(uint8_t *data, uint16_t data_len);  //函数声明
void MQTT_PublishDataQs0(char *topic, char *data);            //函数声明
void MQTT_PublishDataQs1(char *topic, char *data);            //函数声明

void OTA_Init(void);


#endif



