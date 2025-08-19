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
 * @brief 写入TPS55288的一个寄存器
 * @param bus I2C总线
 * @param SlaveAdr I2C从机地址
 * @param RegAdr 寄存器地址
 * @param RegVal 寄存器值
 */
static void TPS55288_write_Reg(iic_bus_t *bus, uint8_t slave_addr, uint8_t Reg_addr, uint8_t Reg_val)
{
    IIC_Write_One_Byte(bus, slave_addr, Reg_addr, Reg_val);
}

/**
 * @brief 读取TPS55288的一个寄存器
 * @param bus I2C总线
 * @param SlaveAdr I2C从机地址
 * @param RegAdr 寄存器地址
 * @return 寄存器值
 */
static uint8_t TPS55288_read_Reg(iic_bus_t *bus, uint8_t slave_addr, uint8_t Reg_addr)
{
    return IIC_Read_One_Byte(bus, slave_addr, Reg_addr);
}

/**
 * @brief 写入TPS55288的VREF寄存器
 * @param bus I2C总线
 * @param slave_addr I2C从机地址
 * @param Vref VREF寄存器值
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
    // 读取10位VREF寄存器
    uint8_t lsb = TPS55288_read_Reg(bus, slave_addr, REG_REF_LSB);
    uint8_t msb = TPS55288_read_Reg(bus, slave_addr, REG_REF_MSB);
    BuckBoost.VREF = (uint16_t)(msb << 8) | lsb;

    // 读取其他8位寄存器
    BuckBoost.I_OUT_LIMIT = TPS55288_read_Reg(bus, slave_addr, REG_IOUT_LIMIT);
    BuckBoost.VOUT_SR = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_SR);
    BuckBoost.VOUT_FS = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_FS);
    BuckBoost.CDC = TPS55288_read_Reg(bus, slave_addr, REG_CDC);
    BuckBoost.MODE = TPS55288_read_Reg(bus, slave_addr, REG_MODE);
    BuckBoost.STATUS = TPS55288_read_Reg(bus, slave_addr, REG_STATUS);

    // 解析状态位
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

    // 将浮点数伏特转换为毫伏，并四舍五入到最近的20mV步进,确保输出电压是芯片支持的最小步长20mV的整数倍。
    target_Vout_mv = (uint16_t)(((Vout * 1000.0f) / VOUT_STEP_MV) + 0.5f) * VOUT_STEP_MV;

    // 计算所需的内部参考电压VREF（毫伏),VREF = VOUT * 反馈比率,使用INTFB=11b对应的默认反馈比率0.0564f。
    // (数据手册公式: VOUT = VREF * (1 / INTFB_RATIO) => VREF = VOUT * INTFB_RATIO)
    target_Vref_mv = (uint16_t)(target_Vout_mv * INTFB_RATIO);

    // 将参考电压VREF转换为10位DAC寄存器值;ref_value = (VREF_mV - VREF_MIN_MV) / REF_RESOLUTION_MV
    if (target_Vref_mv < VREF_MIN_MV){
        ref_value_dac = 0;
    }else{
        ref_value_dac = (uint16_t)(((float)target_Vref_mv - VREF_MIN_MV) / REF_RESOLUTION_MV);
    }

    // 确保DAC值不超过10位寄存器的最大值（1023, 即0x3FF）
    if (ref_value_dac > 1023){
        ref_value_dac = 1023;
    }
    TPS55288_write_Vref(bus, slave_addr, ref_value_dac);
}

void TPS55288_init(iic_bus_t *bus, uint8_t slave_addr)
{
    uint8_t cdc_reg = 0;
    // 清除CDC寄存器中的OCP_MASK和SCP_MASK，为后续配置做准备
    cdc_reg = TPS55288_read_Reg(bus, slave_addr, REG_CDC);
    cdc_reg &= 0x1F; // 清除掩码位 (SC, OCP, OVP)
    TPS55288_write_Reg(bus, slave_addr, REG_CDC, cdc_reg);

    // 配置电流限制：启用电流限制并设置值为1.5A
    uint8_t iout_limit_val = TPS_IOUT_LIMIT_1_5A | TPS_IOUT_LIMIT_ENABLE;
    TPS55288_write_Reg(bus, slave_addr, REG_IOUT_LIMIT, iout_limit_val);

    // 激活输出：设置REG_MODE寄存器中的OE位（第7位）
    uint8_t mode_reg = TPS55288_read_Reg(bus, slave_addr, REG_MODE);
    mode_reg |= TPS_OE_ON; // 设置输出使能 (OE) 位
    TPS55288_write_Reg(bus, slave_addr, REG_MODE, mode_reg);

    // 配置反馈和slew rate,设置内部反馈比率为0.0564，对应于INTFB[1:0] = 11b。
    // 这需要先读取寄存器，修改INTFB位，再写回。
    uint8_t vout_fs_reg = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_FS);
    vout_fs_reg &= ~0x03;
    vout_fs_reg |= TPS_FBR_3;
    TPS55288_write_Reg(bus, slave_addr, REG_VOUT_FS, vout_fs_reg);

    // 设置输出电压变化slew rate为10mV/?s，对应于SR[1:0] = 11b。
    uint8_t vout_sr_reg = TPS55288_read_Reg(bus, slave_addr, REG_VOUT_SR);
    vout_sr_reg &= ~0x03;
    vout_sr_reg |= TPS_VOUT_SR_3;
    TPS55288_write_Reg(bus, slave_addr, REG_VOUT_SR, vout_sr_reg);

    // 设置故障指示掩码：在OE和Current_Limit_EN设置后，设置OCP_MASK和SCP_MASK,将所有掩码位都设置为1。
    cdc_reg = TPS55288_read_Reg(bus, slave_addr, REG_CDC);
    cdc_reg |= TPS_MASK_ON;
    TPS55288_write_Reg(bus, slave_addr, REG_CDC, cdc_reg);

    TPS55288_read_all_Reg(bus, slave_addr);
}


