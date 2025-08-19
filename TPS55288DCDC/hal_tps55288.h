#ifndef _HAL_TPS55288_H
#define _HAL_TPS55288_H

#include <stdint.h>

// I2C 从机地址宏定义,根据原理图中MODE引脚的电阻（24.9kΩ），I2C地址被配置为75h。
#define ADDR_TPS 0x75

// 数据手册寄存器地址宏定义。
#define REG_REF_LSB      0x00  // REF寄存器低字节地址
#define REG_REF_MSB      0x01  // REF寄存器高字节地址
#define REG_IOUT_LIMIT   0x02  // 输出电流限制寄存器地址
#define REG_VOUT_SR      0x03  // 输出电压变化斜率（Slew Rate）寄存器地址
#define REG_VOUT_FS      0x04  // 输出反馈选择寄存器地址
#define REG_CDC          0x05  // 线缆压降补偿寄存器地址
#define REG_MODE         0x06  // 模式控制寄存器地址
#define REG_STATUS       0x07  // 状态寄存器地址
    
// 配置位宏定义
// REG_MODE (地址6) 的第7位，用于启用/禁用输出   
#define TPS_OE_ON           0x80

// REG_VOUT_FS (地址4) 的INTFB[1:0]位，用于设置内部反馈比率
// 根据数据手册表7-7   
#define TPS_FBR_0           0x00 // INTFB=00b, 反馈比率 0.2256   
#define TPS_FBR_1           0x01 // INTFB=01b, 反馈比率 0.1128   
#define TPS_FBR_2           0x02 // INTFB=10b, 反馈比率 0.0752   
#define TPS_FBR_3           0x03 // INTFB=11b, 反馈比率 0.0564   

// REG_IOUT_LIMIT (地址2) 的第7位，用于启用/禁用输出电流限制   
#define TPS_IOUT_LIMIT_ENABLE 0x80

// REG_VOUT_SR (地址3) 的SR[1:0]位，用于设置输出电压斜率   
// 根据数据手册表7-6   
#define TPS_VOUT_SR_0       0x00 // SR=00b, 1.25mV/us   
#define TPS_VOUT_SR_1       0x01 // SR=01b, 2.5mV/us (默认)   
#define TPS_VOUT_SR_2       0x02 // SR=10b, 5mV/us   
#define TPS_VOUT_SR_3       0x03 // SR=11b, 10mV/us   

// REG_CDC (地址5) 的掩码，用于控制故障指示
// 根据数据手册表7-9   
// 启用所有故障指示（SC, OCP, OVP）
#define TPS_MASK_ON         0xE0 
// 关闭所有故障指示
#define TPS_MASK_OFF        0x3F

// 电流限制宏定义，根据数据手册IOUT_LIMIT寄存器设置
#define TPS_IOUT_LIMIT_1_0A 0x14
#define TPS_IOUT_LIMIT_1_2A 0x18
#define TPS_IOUT_LIMIT_1_5A 0x1E
#define TPS_IOUT_LIMIT_2A   0x28
#define TPS_IOUT_LIMIT_MAX  0x7F

// 芯片状态
#define TPS_STATE_OFF   0
#define TPS_STATE_ON    1 

// 芯片参数定义，用于计算VREF
#define REF_RESOLUTION_MV   1.129f // REF寄存器每LSB代表的电压，单位：毫伏   
#define VOUT_MIN_V          0.8f   // 芯片允许的最小输出电压，单位：伏特   
#define VOUT_MAX_V          22.0f  // 芯片允许的最大输出电压，单位：伏特   
#define VOUT_STEP_MV        20.0f  // 输出电压最小步长，单位：毫伏   
#define INTFB_RATIO         0.0564f // 内部反馈比率，对应INTFB=11b设置   
#define VREF_MIN_MV         45.0f  // 芯片允许的最小参考电压，单位：毫伏   


typedef struct 
{
    uint16_t VREF;       // 10位参考电压值
    uint8_t I_OUT_LIMIT; // 输出电流限制
    uint8_t VOUT_SR;     // 输出电压斜率
    uint8_t VOUT_FS;     // 输出反馈选择
    uint8_t CDC;         // 线缆压降补偿
    uint8_t MODE;        // 模式控制
    uint8_t STATUS;      // 状态寄存器
    // 从寄存器中解析出的常用状态
    uint8_t OE;          // 输出使能状态
    uint8_t OCP;         // 过流保护状态
    uint8_t SCP;         // 短路保护状态
}TPS55288_t;
extern TPS55288_t BuckBoost;

/**
 * @brief 初始化TPS55288
 * @param bus I2C总线
 * @param slave_addr I2C从机地址
 */
void TPS55288_init(iic_bus_t *bus, uint8_t slave_addr);

/**
 * @brief 设置输出电压
 * @param bus I2C总线
 * @param slave_addr I2C从机地址
 * @param Vout 输出电压，单位：伏特
 */
void TPS55288_set_Vout(iic_bus_t *bus, uint8_t slave_addr, float Vout);

/**
 * @brief 读取TPS55288的状态寄存器
 * @param bus I2C总线
 * @param slave_addr I2C从机地址
 * @return 状态寄存器值
 */
void TPS55288_read_status(iic_bus_t *bus, uint8_t slave_addr);

/**
 * @brief 读取TPS55288的所有寄存器
 * @param bus I2C总线
 * @param slave_addr I2C从机地址
 * @param BuckBoost 存器数据的结构体
 */
void TPS55288_read_all_Reg(iic_bus_t *bus, uint8_t slave_addr);


#endif /* _HAL_TPS55288_H */
