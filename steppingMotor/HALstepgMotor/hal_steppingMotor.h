#ifndef __HAL_STEPPINGMOTOR_H
#define __HAL_STEPPINGMOTOR_H

#include "main.h"
#include "pid_controller.h"

#define STEPS_PER_REVOLUTION_HALF_STEP    400
#define DEFAULT_MOTOR_SPEED_SPS           500 // Steps per second

#define HAL_MOTORA_OUT_LOW            HAL_GPIO_WritePin(g_M1_GPIO_Port, g_M1_Pin, GPIO_PIN_RESET)
#define HAL_MOTORA_OUT_HIGH           HAL_GPIO_WritePin(g_M1_GPIO_Port, g_M1_Pin, GPIO_PIN_SET)

#define HAL_MOTORB_OUT_LOW            HAL_GPIO_WritePin(g_M2_GPIO_Port, g_M2_Pin, GPIO_PIN_RESET)
#define HAL_MOTORB_OUT_HIGH           HAL_GPIO_WritePin(g_M2_GPIO_Port, g_M2_Pin, GPIO_PIN_SET)

#define HAL_MOTORC_OUT_LOW            HAL_GPIO_WritePin(g_M3_GPIO_Port, g_M3_Pin, GPIO_PIN_RESET)
#define HAL_MOTORC_OUT_HIGH           HAL_GPIO_WritePin(g_M3_GPIO_Port, g_M3_Pin, GPIO_PIN_SET)

#define HAL_MOTORD_OUT_LOW            HAL_GPIO_WritePin(g_M4_GPIO_Port, g_M4_Pin, GPIO_PIN_RESET)
#define HAL_MOTORD_OUT_HIGH           HAL_GPIO_WritePin(g_M4_GPIO_Port, g_M4_Pin, GPIO_PIN_SET)


typedef enum
{
  MOTOR_DIR_CW  = 0, // 顺时针
  MOTOR_DIR_CCW = 1  // 逆时针
} Motor_Direction_t;

typedef enum
{
  MOTOR_STATE_STOP = 0,
  MOTOR_STATE_RUNNING  = 1
} Motor_State_t;

typedef struct
{
  Motor_Direction_t dir; // 电机方向
  uint16_t speed;            // 速度 (步/秒)
  uint32_t steps;            // 剩余步数
  Motor_State_t state;         // 电机状态
  uint8_t step_phase;          // 当前步相
  uint16_t step_counter;       // 步进计数器
  PID_Controller_t pid;      // PID控制器
  float current_speed;       // 当前速度 (SPS)
  float target_speed;        // 目标速度 (SPS)
} SteppingMotor_t;

void hal_steppingMotor_init(void);
void hal_steppingMotor_run(Motor_Direction_t dir, uint32_t steps, uint16_t sps);
void hal_steppingMotor_stop(void);
void hal_steppingMotor_set_target_speed(float speed);
Motor_State_t hal_steppingMotor_get_state(void);





#endif

