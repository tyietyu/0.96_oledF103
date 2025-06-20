#ifndef AS5600_APP_H
#define AS5600_APP_H

#include "AS5600_driver.h"
#include "hal_iic.h"
#include "main.h"
#include <stdint.h>

#define AS5600_DEVICE_COUNT 3  //AS5600设备数量

typedef enum {
    AS5600_DEVICE_1 = 0,
    AS5600_DEVICE_2 = 1,
    AS5600_DEVICE_3 = 2,
} AS5600_Device_Index_t;

#pragma pack(1)
typedef struct{
    float current_angle;        //当前角度
    float last_angle;           //上次角度
    float relative_angle;       //相对角度
    float angular_velocity;     // 角速度(度/秒)
    float rpm;                  // 转速(转/分)
    int32_t total_turns;          //总转数
    uint16_t zero_position;     //零位位置
    uint16_t stop_position;     //停止位置
    uint8_t angle_flag;         //获取角度标志位
}AS5600_angle_t;
#pragma pack()

typedef struct {
    as5600_power_mode_t             power_mode;
    as5600_hysteresis_t             hysteresis;
    as5600_output_stage_t           output_stage;
    as5600_slow_filter_t            slow_filter;
    as5600_fast_filter_threshold_t  fast_filter_threshold;
    as5600_bool_t                   watchdog_enable;
} as5600_config_t;

typedef struct {
    as5600_handle_t handle;       
    AS5600_angle_t  angle_data;   
    iic_bus_t       iic_bus;   
    as5600_config_t  config;   
    uint8_t         is_initialized; 
} as5600_device_t;
extern as5600_device_t g_as5600_devices[AS5600_DEVICE_COUNT];

/*public app function*/
/**
 * @brief 初始化所有AS5600设备
 */
void as5600_init_all(void);
/**
 * @brief 从指定的AS5600设备读取并计算角度、速度等信息
 * @param device_index 设备索引
 */
void as5600_update_angle_data(AS5600_Device_Index_t device_index);
/**
 * @brief 获取指定设备的转速 (RPM)
 * @param device_index 设备索引
 * @return float 转速值
 */
float as5600_get_rpm(AS5600_Device_Index_t device_index);
/**
 * @brief 获取指定设备的总圈数
 * @param device_index 设备索引
 * @return int32_t 总圈数
 */
int32_t as5600_get_total_turns(AS5600_Device_Index_t device_index);
/**
 * @brief 获取指定设备的相对角度
 * @param device_index 设备索引
 * @return float 相对角度值
 */
float as5600_get_relative_angle(AS5600_Device_Index_t device_index);

void as5600_debug_print(const char *const fmt, ...);

#endif










