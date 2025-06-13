#ifndef __IIC_HAL_H
#define __IIC_HAL_H

#include "stm32f1xx_hal.h"

typedef struct
{
	GPIO_TypeDef * IIC_SDA_PORT;
	GPIO_TypeDef * IIC_SCL_PORT;
	uint16_t IIC_SDA_PIN;
	uint16_t IIC_SCL_PIN;
	//void (*CLK_ENABLE)(void);
}iic_bus_t;

void IICStart(iic_bus_t *bus);
void IICStop(iic_bus_t *bus);
unsigned char IICWaitAck(iic_bus_t *bus);
void IICSendAck(iic_bus_t *bus);
void IICSendNotAck(iic_bus_t *bus);
void IICSendByte(iic_bus_t *bus, unsigned char cSendByte);
unsigned char IICReceiveByte(iic_bus_t *bus);

uint8_t IICInit(iic_bus_t *bus);
uint8_t IICDeinit(iic_bus_t *bus);
uint8_t IIC_Write_One_Byte(iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t data);
uint8_t IIC_Write_Multi_Byte(iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]);
unsigned char IIC_Read_One_Byte(iic_bus_t *bus, uint8_t daddr,uint8_t reg);
uint8_t IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]);

/*public function*/
uint8_t AS5600_IIC_Init(void);
uint8_t AS5600_IIC_Deinit(void);
uint8_t AS5600_IIC_Read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
uint8_t AS5600_IIC_Write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
void as5600_debug_print(const char *const fmt, ...);

#endif

