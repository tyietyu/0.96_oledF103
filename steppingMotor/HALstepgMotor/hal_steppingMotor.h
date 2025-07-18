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
  MOTOR_DIR_CW  = 0, // ˳ʱ��
  MOTOR_DIR_CCW = 1  // ��ʱ��
} Motor_Direction_t;

typedef enum
{
  MOTOR_STATE_STOP = 0,
  MOTOR_STATE_RUNNING  = 1
} Motor_State_t;

typedef struct
{
  Motor_Direction_t dir; // �������
  uint16_t speed;            // �ٶ� (��/��)
  uint32_t steps;            // ʣ�ಽ��
  Motor_State_t state;         // ���״̬
  uint8_t step_phase;          // ��ǰ����
  uint16_t step_counter;       // ����������
  PID_Controller_t pid;      // PID������
  float current_speed;       // ��ǰ�ٶ� (SPS)
  float target_speed;        // Ŀ���ٶ� (SPS)
} SteppingMotor_t;

void hal_steppingMotor_init(void);
void hal_steppingMotor_run(Motor_Direction_t dir, uint32_t steps, uint16_t sps);
void hal_steppingMotor_stop(void);
void hal_steppingMotor_set_target_speed(float speed);
Motor_State_t hal_steppingMotor_get_state(void);





#endif

