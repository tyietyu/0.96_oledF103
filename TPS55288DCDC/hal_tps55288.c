#include "hal_tps55288.h"
#include "hal_iic.h"
#include <math.h>

TPS55288_t BuckBoost; 

iic_bus_t iic_bus = {
    .IIC_SCL_PORT = GPIOD,
    .IIC_SDA_PORT = GPIOC,
    .IIC_SCL_PIN = GPIO_PIN_2,
    .IIC_SDA_PIN = GPIO_PIN_12,
};

/**
 * @brief д��TPS55288��һ���Ĵ���
 * @param bus I2C����
 * @param SlaveAdr I2C�ӻ���ַ
 * @param RegAdr �Ĵ�����ַ
 * @param RegVal �Ĵ���ֵ
 */
static void TPS55288_write_Reg(iic_bus_t *bus, uint8_t slave_addr, uint8_t Reg_addr, uint8_t Reg_val)
{
    IIC_Write_One_Byte(bus, slave_addr, Reg_addr, Reg_val);
}

/**
 * @brief ��ȡTPS55288��һ���Ĵ���
 * @param bus I2C����
 * @param SlaveAdr I2C�ӻ���ַ
 * @param RegAdr �Ĵ�����ַ
 * @return �Ĵ���ֵ
 */
static uint8_t TPS55288_read_Reg(iic_bus_t *bus, uint8_t slave_addr, uint8_t Reg_addr)
{
    return IIC_Read_One_Byte(bus, slave_addr, Reg_addr);
}

/**
 * @brief д��TPS55288��VREF�Ĵ���
 * @param bus I2C����
 * @param slave_addr I2C�ӻ���ַ
 * @param Vref VREF�Ĵ���ֵ
 */
static void TPS55288_write_Vref(iic_bus_t *bus, uint8_t slave_addr, uint16_t Vref)
{
    uint8_t high_bit, low_bit;

    Vref = Vref & 0x3FF;
    high_bit = (uint8_t)(Vref >> 8);
    low_bit = (uint8_t)(Vref & 0xFF);

    TPS55288_write_Reg(bus, slave_addr, REG_REF_LSB, low_bit);
    TPS55288_write_Reg(bus, slave_addr, REG_REF_MSB, high_bit);
}

void TPS55288_read_status(iic_bus_t *bus, uint8_t slave_addr)
{
    BuckBoost.STATUS = TPS55288_read_Reg(bus, slave_addr, REG_STATUS);
}

void TPS55288_read_all_Reg(iic_bus_t *bus, uint8_t slave_addr)
{
    // ��ȡ10λVREF�Ĵ���
    uint8_t lsb = TPS55288_read_Reg(bus, slave_addr, REG_REF_LSB);
    uint8_t msb = TPS55288_read_Reg(bus, slave_addr, REG_REF_MSB);
    BuckBoost.VREF = (uint16_t)(msb << 8) | lsb;

    // ��ȡ����8λ�Ĵ���
    BuckBoost.I_OUT_LIMIT = TPS55288_read_Reg(bus, slave_addr, REG_IOUT_LIMIT);
    BuckBoost.VOUT_SR = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_SR);
    BuckBoost.VOUT_FS = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_FS);
    BuckBoost.CDC = TPS55288_read_Reg(bus, slave_addr, REG_CDC);
    BuckBoost.MODE = TPS55288_read_Reg(bus, slave_addr, REG_MODE);
    BuckBoost.STATUS = TPS55288_read_Reg(bus, slave_addr, REG_STATUS);

    // ����״̬λ
    BuckBoost.OCP = (BuckBoost.STATUS >> 6) & 0x01;
    BuckBoost.SCP = (BuckBoost.STATUS >> 7) & 0x01;
    BuckBoost.OE = (BuckBoost.MODE >> 7) & 0x01;
}

void TPS55288_set_Vout(iic_bus_t *bus, uint8_t slave_addr, float Vout)
{
    uint16_t target_Vout_mv;
    uint16_t target_Vref_mv;
    uint16_t ref_value_dac;

    if (Vout < VOUT_MIN_V || Vout > VOUT_MAX_V){
        return;
    }

    // ������������ת��Ϊ���������������뵽�����20mV����,ȷ�������ѹ��оƬ֧�ֵ���С����20mV����������
    target_Vout_mv = (uint16_t)(((Vout * 1000.0f) / VOUT_STEP_MV) + 0.5f) * VOUT_STEP_MV;

    // ����������ڲ��ο���ѹVREF������),VREF = VOUT * ��������,ʹ��INTFB=11b��Ӧ��Ĭ�Ϸ�������0.0564f��
    // (�����ֲṫʽ: VOUT = VREF * (1 / INTFB_RATIO) => VREF = VOUT * INTFB_RATIO)
    target_Vref_mv = (uint16_t)(target_Vout_mv * INTFB_RATIO);

    // ���ο���ѹVREFת��Ϊ10λDAC�Ĵ���ֵ;ref_value = (VREF_mV - VREF_MIN_MV) / REF_RESOLUTION_MV
    if (target_Vref_mv < VREF_MIN_MV){
        ref_value_dac = 0;
    }else{
        ref_value_dac = (uint16_t)(((float)target_Vref_mv - VREF_MIN_MV) / REF_RESOLUTION_MV);
    }

    // ȷ��DACֵ������10λ�Ĵ��������ֵ��1023, ��0x3FF��
    if (ref_value_dac > 1023){
        ref_value_dac = 1023;
    }
    TPS55288_write_Vref(bus, slave_addr, ref_value_dac);
}

void TPS55288_init(iic_bus_t *bus, uint8_t slave_addr)
{
    uint8_t cdc_reg = 0;
    // ���CDC�Ĵ����е�OCP_MASK��SCP_MASK��Ϊ����������׼��
    cdc_reg = TPS55288_read_Reg(bus, slave_addr, REG_CDC);
    cdc_reg &= 0x1F; // �������λ (SC, OCP, OVP)
    TPS55288_write_Reg(bus, slave_addr, REG_CDC, cdc_reg);

    // ���õ������ƣ����õ������Ʋ�����ֵΪ1.5A
    uint8_t iout_limit_val = TPS_IOUT_LIMIT_1_5A | TPS_IOUT_LIMIT_ENABLE;
    TPS55288_write_Reg(bus, slave_addr, REG_IOUT_LIMIT, iout_limit_val);

    // �������������REG_MODE�Ĵ����е�OEλ����7λ��
    uint8_t mode_reg = TPS55288_read_Reg(bus, slave_addr, REG_MODE);
    mode_reg |= TPS_OE_ON; // �������ʹ�� (OE) λ
    TPS55288_write_Reg(bus, slave_addr, REG_MODE, mode_reg);

    // ���÷�����slew rate,�����ڲ���������Ϊ0.0564����Ӧ��INTFB[1:0] = 11b��
    // ����Ҫ�ȶ�ȡ�Ĵ������޸�INTFBλ����д�ء�
    uint8_t vout_fs_reg = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_FS);
    vout_fs_reg &= ~0x03;
    vout_fs_reg |= TPS_FBR_3;
    TPS55288_write_Reg(bus, slave_addr, REG_VOUT_FS, vout_fs_reg);

    // ���������ѹ�仯slew rateΪ10mV/?s����Ӧ��SR[1:0] = 11b��
    uint8_t vout_sr_reg = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_SR);
    vout_sr_reg &= ~0x03;
    vout_sr_reg |= TPS_VOUT_SR_3;
    TPS55288_write_Reg(bus, slave_addr, REG_VOUT_SR, vout_sr_reg);

    // ���ù���ָʾ���룺��OE��Current_Limit_EN���ú�����OCP_MASK��SCP_MASK,����������λ������Ϊ1��
    cdc_reg = TPS55288_read_Reg(bus, slave_addr, REG_CDC);
    cdc_reg |= TPS_MASK_ON;
    TPS55288_write_Reg(bus, slave_addr, REG_CDC, cdc_reg);

    TPS55288_read_all_Reg(bus, slave_addr);
}


