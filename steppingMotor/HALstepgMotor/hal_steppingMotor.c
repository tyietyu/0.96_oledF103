#include "hal_steppingMotor.h"
#include "pid_controller.h"
#include "tim.h"

extern TIM_HandleTypeDef htim2;

// 2-phase 8-step (half-step) sequence
const uint8_t step_sequence[8] = {0x09, 0x08, 0x0C, 0x04, 0x06, 0x02, 0x03, 0x01};

SteppingMotor_t motor;
/**
  * @brief  初始化步进电机
  * @retval None
  */
void hal_steppingMotor_init(void)
{
  motor.state = MOTOR_STATE_STOP;
  motor.step_phase = 0;
  motor.steps = 0;
  motor.step_counter = 0;
  motor.current_speed = 0;
  motor.target_speed = 0;
  HAL_MOTORA_OUT_LOW;
  HAL_MOTORB_OUT_LOW;
  HAL_MOTORC_OUT_LOW;
  HAL_MOTORD_OUT_LOW;
  // Initialize PID controller
  // Kp, Ki, Kd, output_min, output_max
  HAL_TIM_Base_Start_IT(&htim2);
  pid_controller_init(&motor.pid, 0.1f, 0.01f, 0.005f, 50, 2000);
}

/**
  * @brief  Sets the target speed for the motor (used by PID).
  * @param  speed The target speed in steps per second.
  * @retval None
  */
void hal_steppingMotor_set_target_speed(float speed)
{
  motor.target_speed = speed;
}

static void hal_steppingMotor_set_speed(uint16_t sps)
{
  if (sps > 0){
    motor.speed = 1000 / sps; // Calculate timer counts per step
  }else{
    motor.speed = 0;
  }
}

/**
  * @brief  运行步进电机
  * @param  dir   电机方向 (MOTOR_DIR_CW or MOTOR_DIR_CCW)
  * @param  steps 运行步数
  * @param  sps   运行速度 (步/秒)
  * @retval None
  */
void hal_steppingMotor_run(Motor_Direction_t dir, uint32_t steps, uint16_t sps)
{
  if (motor.state == MOTOR_STATE_RUNNING) return;
  motor.dir = dir;
  motor.steps = steps;
  motor.state = MOTOR_STATE_RUNNING;
  hal_steppingMotor_set_speed(sps);
  HAL_TIM_Base_Start_IT(&htim2);
}

/**
  * @brief  停止步进电机
  * @retval None
  */
void hal_steppingMotor_stop(void)
{
  HAL_TIM_Base_Stop_IT(&htim2);
  motor.state = MOTOR_STATE_STOP;
  HAL_MOTORA_OUT_LOW;
  HAL_MOTORB_OUT_LOW;
  HAL_MOTORC_OUT_LOW;
  HAL_MOTORD_OUT_LOW;
}
/**
  * @brief  获取当前电机状态
  * @retval Motor_State_t 当前状态
  */
Motor_State_t hal_steppingMotor_get_state(void)
{
  return motor.state;
}

/**
  * @brief  根据步相设置电机引脚电平
  * @param  phase 步相值
  * @retval None
  */
static void motor_step(uint8_t phase)
{
  if (phase & 0x08) HAL_MOTORA_OUT_HIGH; else HAL_MOTORA_OUT_LOW;
  if (phase & 0x04) HAL_MOTORB_OUT_HIGH; else HAL_MOTORB_OUT_LOW;
  if (phase & 0x02) HAL_MOTORC_OUT_HIGH; else HAL_MOTORC_OUT_LOW;
  if (phase & 0x01) HAL_MOTORD_OUT_HIGH; else HAL_MOTORD_OUT_LOW;
}

/**
  * @brief  定时器中断回调函数，用于驱动步进电机
  * @param  htim 定时器句柄
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    // Update current speed reading periodically (e.g., every 10ms)
    // This part needs a real speed feedback mechanism, like an encoder.
    // For now, we simulate it.
    static uint32_t speed_update_counter = 0;
    speed_update_counter++;
    if(speed_update_counter >= 10)
    {
      speed_update_counter = 0;
      // In a real application, you would get the actual speed from a sensor.
      // Here we just use the last commanded speed for simulation.
      float pid_output = pid_controller_update(&motor.pid, motor.target_speed, motor.current_speed);
      hal_steppingMotor_set_speed(pid_output);
      motor.current_speed = pid_output; // Simulate current speed equals target speed
    }
    if (motor.state == MOTOR_STATE_RUNNING && motor.steps > 0 && motor.speed > 0)
    {
      motor.step_counter++;
      if (motor.step_counter >= motor.speed)
      {
        motor.step_counter = 0;
        if (motor.target_speed >= 0)
        {
          motor.dir = MOTOR_DIR_CW;
          motor.step_phase++;
          if (motor.step_phase >= 8) motor.step_phase = 0;
        }
        else // CCW
        {
          motor.dir = MOTOR_DIR_CCW;
          if (motor.step_phase == 0) motor.step_phase = 8;
          motor.step_phase--;
        }
        motor_step(step_sequence[motor.step_phase]);
        motor.steps--;
        if (motor.steps == 0)
        {
          hal_steppingMotor_stop();
        }
      }
    }
  }
}


