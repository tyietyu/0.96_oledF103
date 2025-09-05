#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

#define MOTOR_A1(state)   HAL_GPIO_WritePin(A1_GPIO_Port, A1_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define MOTOR_A2(state)   HAL_GPIO_WritePin(A2_GPIO_Port, A2_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define MOTOR_B1(state)   HAL_GPIO_WritePin(B1_GPIO_Port, B1_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define MOTOR_B2(state)   HAL_GPIO_WritePin(B2_GPIO_Port, B2_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET)

/**
 * @brief  Motor direction definition
 */
typedef enum
{
  DIR_CW = 0, // Clockwise
  DIR_CCW = 1 // Counter-clockwise
} motor_dir_t;

/**
 * @brief  Motor state definition for the state machine
 */
typedef enum
{
  MOTOR_STOPPED,
  MOTOR_ACCELERATING,
  MOTOR_RUNNING_AT_MAX_SPEED,
  MOTOR_DECELERATING
} motor_state_t;

void motor_init(void);
void motor_run(motor_dir_t dir);
void motor_stop(void);

#endif /* __MOTOR_H */
