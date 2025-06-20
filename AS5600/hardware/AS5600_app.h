#ifndef AS5600_APP_H
#define AS5600_APP_H

#include "AS5600_driver.h"
#include "hal_iic.h"
#include "main.h"
#include <stdint.h>

#define AS5600_DEVICE_COUNT 3  //AS5600�豸����

typedef enum {
    AS5600_DEVICE_1 = 0,
    AS5600_DEVICE_2 = 1,
    AS5600_DEVICE_3 = 2,
} AS5600_Device_Index_t;

#pragma pack(1)
typedef struct{
    float current_angle;        //��ǰ�Ƕ�
    float last_angle;           //�ϴνǶ�
    float relative_angle;       //��ԽǶ�
    float angular_velocity;     // ���ٶ�(��/��)
    float rpm;                  // ת��(ת/��)
    int32_t total_turns;          //��ת��
    uint16_t zero_position;     //��λλ��
    uint16_t stop_position;     //ֹͣλ��
    uint8_t angle_flag;         //��ȡ�Ƕȱ�־λ
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
 * @brief ��ʼ������AS5600�豸
 */
void as5600_init_all(void);
/**
 * @brief ��ָ����AS5600�豸��ȡ������Ƕȡ��ٶȵ���Ϣ
 * @param device_index �豸����
 */
void as5600_update_angle_data(AS5600_Device_Index_t device_index);
/**
 * @brief ��ȡָ���豸��ת�� (RPM)
 * @param device_index �豸����
 * @return float ת��ֵ
 */
float as5600_get_rpm(AS5600_Device_Index_t device_index);
/**
 * @brief ��ȡָ���豸����Ȧ��
 * @param device_index �豸����
 * @return int32_t ��Ȧ��
 */
int32_t as5600_get_total_turns(AS5600_Device_Index_t device_index);
/**
 * @brief ��ȡָ���豸����ԽǶ�
 * @param device_index �豸����
 * @return float ��ԽǶ�ֵ
 */
float as5600_get_relative_angle(AS5600_Device_Index_t device_index);

void as5600_debug_print(const char *const fmt, ...);

#endif










