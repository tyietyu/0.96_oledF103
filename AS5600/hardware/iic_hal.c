#include "iic_hal.h"
#include "delay.h"
#include "gpio.h"
#include <stdarg.h> 
#include <stdio.h> 

extern iic_bus_t AS5600_iic_bus;

/**
 * @brief SDA������ģʽ����
 * @param None
 * @retval None
 */
void SDA_Input_Mode(iic_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = bus->IIC_SDA_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->IIC_SDA_PORT, &GPIO_InitStructure);
}

/**
 * @brief SDA�����ģʽ����
 * @param None
 * @retval None
 */
void SDA_Output_Mode(iic_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = bus->IIC_SDA_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->IIC_SDA_PORT, &GPIO_InitStructure);
}

/**
 * @brief SDA�����һ��λ
 * @param val ���������
 * @retval None
 */
void SDA_Output(iic_bus_t *bus, uint16_t val)
{
    if (val)
    {
        bus->IIC_SDA_PORT->BSRR |= bus->IIC_SDA_PIN;
    }
    else
    {
        bus->IIC_SDA_PORT->BSRR = (uint32_t)bus->IIC_SDA_PIN << 16U;
    }
}

/**
 * @brief SCL�����һ��λ
 * @param val ���������
 * @retval None
 */
void SCL_Output(iic_bus_t *bus, uint16_t val)
{
    if (val)
    {
        bus->IIC_SCL_PORT->BSRR |= bus->IIC_SCL_PIN;
    }
    else
    {
        bus->IIC_SCL_PORT->BSRR = (uint32_t)bus->IIC_SCL_PIN << 16U;
    }
}

/**
 * @brief SDA����һλ
 * @param None
 * @retval GPIO����һλ
 */
uint8_t SDA_Input(iic_bus_t *bus)
{
    if (HAL_GPIO_ReadPin(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN) == GPIO_PIN_SET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief IIC��ʼ�ź�
 * @param None
 * @retval None
 */
void IICStart(iic_bus_t *bus)
{
    SDA_Output(bus, 1);
    // delay1(DELAY_TIME);
    delay_us(2);
    SCL_Output(bus, 1);
    delay_us(1);
    SDA_Output(bus, 0);
    delay_us(1);
    SCL_Output(bus, 0);
    delay_us(1);
}

/**
 * @brief IICֹͣ�ź�
 * @param None
 * @retval None
 */
void IICStop(iic_bus_t *bus)
{
    SCL_Output(bus, 0);
    delay_us(2);
    SDA_Output(bus, 0);
    delay_us(1);
    SCL_Output(bus, 1);
    delay_us(1);
    SDA_Output(bus, 1);
    delay_us(1);
}

/**
 * @brief  IIC�ȴ�ȷ���ź�
 * @param None
 * @retval None
 */
unsigned char IICWaitAck(iic_bus_t *bus)
{
    unsigned short cErrTime = 5;
    SDA_Input_Mode(bus);
    SCL_Output(bus, 1);
    while (SDA_Input(bus))
    {
        cErrTime--;
        delay_us(1);
        if (0 == cErrTime)
        {
            SDA_Output_Mode(bus);
            IICStop(bus);
            return ERROR;
        }
    }
    SDA_Output_Mode(bus);
    SCL_Output(bus, 0);
    delay_us(2);
    return SUCCESS;
}

/**
 * @brief IIC����ȷ���ź�
 * @param None
 * @retval None
 */
void IICSendAck(iic_bus_t *bus)
{
    SDA_Output(bus, 0);
    delay_us(1);
    SCL_Output(bus, 1);
    delay_us(1);
    SCL_Output(bus, 0);
    delay_us(1);
}

/**
 * @brief IIC���ͷ�ȷ���ź�
 * @param None
 * @retval None
 */
void IICSendNotAck(iic_bus_t *bus)
{
    SDA_Output(bus, 1);
    delay_us(1);
    SCL_Output(bus, 1);
    delay_us(1);
    SCL_Output(bus, 0);
    delay_us(2);
}

/**
 * @brief IIC����һ���ֽ�
 * @param cSendByte ��Ҫ���͵��ֽ�
 * @retval None
 */
void IICSendByte(iic_bus_t *bus, unsigned char cSendByte)
{
    unsigned char i = 8;
    while (i--)
    {
        SCL_Output(bus, 0);
        delay_us(2);
        SDA_Output(bus, cSendByte & 0x80);
        delay_us(1);
        cSendByte += cSendByte;
        delay_us(1);
        SCL_Output(bus, 1);
        delay_us(1);
    }
    SCL_Output(bus, 0);
    delay_us(2);
}

/**
 * @brief IIC����һ���ֽ�
 * @param None
 * @retval ���յ����ֽ�
 */
unsigned char IICReceiveByte(iic_bus_t *bus)
{
    unsigned char i = 8;
    unsigned char cR_Byte = 0;
    SDA_Input_Mode(bus);
    while (i--)
    {
        cR_Byte += cR_Byte;
        SCL_Output(bus, 0);
        delay_us(2);
        SCL_Output(bus, 1);
        delay_us(1);
        cR_Byte |= SDA_Input(bus);
    }
    SCL_Output(bus, 0);
    SDA_Output_Mode(bus);
    return cR_Byte;
}

/**
 * @brief IICд��һ���ֽ�
 * @param daddr �豸��ַ
 * @param reg �Ĵ�����ַ
 * @param data д�������
 * @retval ����0��ʾ�ɹ���1��ʾʧ��
 */
uint8_t IIC_Write_One_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t data)
{
    IICStart(bus);
    IICSendByte(bus, daddr << 1);

    if (IICWaitAck(bus))
    {
        IICStop(bus);
        return 1;
    }

    IICSendByte(bus, reg);
    IICWaitAck(bus);
    IICSendByte(bus, data);
    IICWaitAck(bus);
    IICStop(bus);
    delay_us(1);

    return 0;
}

/**
 * @brief IICд�����ֽ�
 * @param daddr �豸��ַ
 * @param reg �Ĵ�����ַ
 * @param length д����ֽ���
 * @param buff д������ݻ�����
 * @retval ����0��ʾ�ɹ���1��ʾʧ��
 */
uint8_t IIC_Write_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[])
{
    unsigned char i;
    IICStart(bus);

    IICSendByte(bus, daddr << 1);
    if (IICWaitAck(bus))
    {
        IICStop(bus);
        return 1;
    }
    IICSendByte(bus, reg);
    IICWaitAck(bus);
    for (i = 0; i < length; i++)
    {
        IICSendByte(bus, buff[i]);
        IICWaitAck(bus);
    }
    IICStop(bus);
    delay_us(1);
    return 0;
}

/**
 * @brief IIC��ȡһ���ֽ�
 * @param daddr �豸��ַ
 * @param reg �Ĵ�����ַ
 * @retval ���ض�ȡ�����ֽ�
 */
unsigned char IIC_Read_One_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg)
{
    unsigned char dat;
    IICStart(bus);
    IICSendByte(bus, daddr << 1);
    IICWaitAck(bus);
    IICSendByte(bus, reg);
    IICWaitAck(bus);

    IICStart(bus);
    IICSendByte(bus, (daddr << 1) + 1);
    IICWaitAck(bus);
    dat = IICReceiveByte(bus);
    IICSendNotAck(bus);
    IICStop(bus);
    return dat;
}

/**
 * @brief IIC��ȡ����ֽ�
 * @param daddr �豸��ַ
 * @param reg �Ĵ�����ַ
 * @param length ��ȡ���ֽ���
 * @param buff �洢��ȡ���ݵĻ�����
 * @retval ����0��ʾ�ɹ���1��ʾʧ��
 */
uint8_t IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[])
{
    unsigned char i;
    IICStart(bus);
    IICSendByte(bus, daddr << 1);
    if (IICWaitAck(bus))
    {
        IICStop(bus);
        return 1;
    }
    IICSendByte(bus, reg);
    IICWaitAck(bus);

    IICStart(bus);
    IICSendByte(bus, (daddr << 1) + 1);
    IICWaitAck(bus);
    for (i = 0; i < length; i++)
    {
        buff[i] = IICReceiveByte(bus);
        if (i < length - 1)
        {
            IICSendAck(bus);
        }
    }
    IICSendNotAck(bus);
    IICStop(bus);
    return 0;
}

/**
 * @brief IIC���߳�ʼ��
 * @param bus IIC���߽ṹ��ָ��
 * @retval None
 */
uint8_t IICInit(iic_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // bus->CLK_ENABLE();

    GPIO_InitStructure.Pin = bus->IIC_SDA_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->IIC_SDA_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = bus->IIC_SCL_PIN;
    HAL_GPIO_Init(bus->IIC_SCL_PORT, &GPIO_InitStructure);
    SDA_Output_Mode(bus);
    SCL_Output_Mode(bus);
	return 0;
}

/**
 * @brief IIC�����ͷ�
 * @param bus IIC���߽ṹ��ָ��
 * @retval None
 */
uint8_t IICDeinit(iic_bus_t *bus)
{
    HAL_GPIO_DeInit(bus->IIC_SDA_PORT, bus->IIC_SDA_PIN);
    HAL_GPIO_DeInit(bus->IIC_SCL_PORT, bus->IIC_SCL_PIN);
	return 0;
}
/**
 * @brief AS5600 driver iic init
 * @param None
 * @retval 0 success, 1 fail
 */
uint8_t AS5600_IIC_Init(void)
{
    if (IICInit(&AS5600_iic_bus) == 0){
        return 0;
    }else{
        return 1;
    }
}

/**
 * @brief AS5600 driver iic deinit
 * @param None
 * @retval None
 */
uint8_t AS5600_IIC_Deinit(void)
{
   if (IICDeinit(&AS5600_iic_bus) == 0){
        return 0;
   }else{
        return 1;
   }
}

uint8_t AS5600_IIC_Read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if(IIC_Read_Multi_Byte(&AS5600_iic_bus, addr, reg, len, data) == 0){
        return 0;
    }else{
        return 1;
    }
}

uint8_t AS5600_IIC_Write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if(IIC_Write_Multi_Byte(&AS5600_iic_bus, addr, reg, len, data) == 0){
        return 0;
    }else{
        return 1;
    }
}

extern UART_HandleTypeDef huart5;
/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void as5600_debug_print(const char *const fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    if(len > 0 && len < sizeof(buf))
    {
        HAL_UART_Transmit(&huart5, (uint8_t *)buf, len, 1000);
    }
}

