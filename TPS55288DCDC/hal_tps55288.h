#ifndef _HAL_TPS55288_H
#define _HAL_TPS55288_H

#include <stdint.h>

// I2C �ӻ���ַ�궨��,����ԭ��ͼ��MODE���ŵĵ��裨24.9k������I2C��ַ������Ϊ75h��
#define ADDR_TPS 0x75

// �����ֲ�Ĵ�����ַ�궨�塣
#define REG_REF_LSB      0x00  // REF�Ĵ������ֽڵ�ַ
#define REG_REF_MSB      0x01  // REF�Ĵ������ֽڵ�ַ
#define REG_IOUT_LIMIT   0x02  // ����������ƼĴ�����ַ
#define REG_VOUT_SR      0x03  // �����ѹ�仯б�ʣ�Slew Rate���Ĵ�����ַ
#define REG_VOUT_FS      0x04  // �������ѡ��Ĵ�����ַ
#define REG_CDC          0x05  // ����ѹ�������Ĵ�����ַ
#define REG_MODE         0x06  // ģʽ���ƼĴ�����ַ
#define REG_STATUS       0x07  // ״̬�Ĵ�����ַ
    
// ����λ�궨��
// REG_MODE (��ַ6) �ĵ�7λ����������/�������   
#define TPS_OE_ON           0x80

// REG_VOUT_FS (��ַ4) ��INTFB[1:0]λ�����������ڲ���������
// ���������ֲ��7-7   
#define TPS_FBR_0           0x00 // INTFB=00b, �������� 0.2256   
#define TPS_FBR_1           0x01 // INTFB=01b, �������� 0.1128   
#define TPS_FBR_2           0x02 // INTFB=10b, �������� 0.0752   
#define TPS_FBR_3           0x03 // INTFB=11b, �������� 0.0564   

// REG_IOUT_LIMIT (��ַ2) �ĵ�7λ����������/���������������   
#define TPS_IOUT_LIMIT_ENABLE 0x80

// REG_VOUT_SR (��ַ3) ��SR[1:0]λ���������������ѹб��   
// ���������ֲ��7-6   
#define TPS_VOUT_SR_0       0x00 // SR=00b, 1.25mV/us   
#define TPS_VOUT_SR_1       0x01 // SR=01b, 2.5mV/us (Ĭ��)   
#define TPS_VOUT_SR_2       0x02 // SR=10b, 5mV/us   
#define TPS_VOUT_SR_3       0x03 // SR=11b, 10mV/us   

// REG_CDC (��ַ5) �����룬���ڿ��ƹ���ָʾ
// ���������ֲ��7-9   
// �������й���ָʾ��SC, OCP, OVP��
#define TPS_MASK_ON         0xE0 
// �ر����й���ָʾ
#define TPS_MASK_OFF        0x3F

// �������ƺ궨�壬���������ֲ�IOUT_LIMIT�Ĵ�������
#define TPS_IOUT_LIMIT_1_0A 0x14
#define TPS_IOUT_LIMIT_1_2A 0x18
#define TPS_IOUT_LIMIT_1_5A 0x1E
#define TPS_IOUT_LIMIT_2A   0x28
#define TPS_IOUT_LIMIT_MAX  0x7F

// оƬ״̬
#define TPS_STATE_OFF   0
#define TPS_STATE_ON    1 

// оƬ�������壬���ڼ���VREF
#define REF_RESOLUTION_MV   1.129f // REF�Ĵ���ÿLSB����ĵ�ѹ����λ������   
#define VOUT_MIN_V          0.8f   // оƬ�������С�����ѹ����λ������   
#define VOUT_MAX_V          22.0f  // оƬ�������������ѹ����λ������   
#define VOUT_STEP_MV        20.0f  // �����ѹ��С��������λ������   
#define INTFB_RATIO         0.0564f // �ڲ��������ʣ���ӦINTFB=11b����   
#define VREF_MIN_MV         45.0f  // оƬ�������С�ο���ѹ����λ������   


typedef struct 
{
    uint16_t VREF;       // 10λ�ο���ѹֵ
    uint8_t I_OUT_LIMIT; // �����������
    uint8_t VOUT_SR;     // �����ѹб��
    uint8_t VOUT_FS;     // �������ѡ��
    uint8_t CDC;         // ����ѹ������
    uint8_t MODE;        // ģʽ����
    uint8_t STATUS;      // ״̬�Ĵ���
    // �ӼĴ����н������ĳ���״̬
    uint8_t OE;          // ���ʹ��״̬
    uint8_t OCP;         // ��������״̬
    uint8_t SCP;         // ��·����״̬
}TPS55288_t;
extern TPS55288_t BuckBoost;

/**
 * @brief ��ʼ��TPS55288
 * @param bus I2C����
 * @param slave_addr I2C�ӻ���ַ
 */
void TPS55288_init(iic_bus_t *bus, uint8_t slave_addr);

/**
 * @brief ���������ѹ
 * @param bus I2C����
 * @param slave_addr I2C�ӻ���ַ
 * @param Vout �����ѹ����λ������
 */
void TPS55288_set_Vout(iic_bus_t *bus, uint8_t slave_addr, float Vout);

/**
 * @brief ��ȡTPS55288��״̬�Ĵ���
 * @param bus I2C����
 * @param slave_addr I2C�ӻ���ַ
 * @return ״̬�Ĵ���ֵ
 */
void TPS55288_read_status(iic_bus_t *bus, uint8_t slave_addr);

/**
 * @brief ��ȡTPS55288�����мĴ���
 * @param bus I2C����
 * @param slave_addr I2C�ӻ���ַ
 * @param BuckBoost �������ݵĽṹ��
 */
void TPS55288_read_all_Reg(iic_bus_t *bus, uint8_t slave_addr);


#endif /* _HAL_TPS55288_H */
