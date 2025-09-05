#include "motor.h"
#include "tim.h"

// Delay in ms for the fastest speed (shorter delay = faster)
#define MAX_SPEED_DELAY_MS      2
// Delay in ms for the slowest speed (at the start/end of the ramp)
#define MIN_SPEED_DELAY_MS      10
// How many steps to take for accelerating or decelerating
#define RAMP_STEPS              200

static volatile motor_state_t motor_state = MOTOR_STOPPED;

// 用于电机控制的状态变量
static volatile motor_dir_t current_motor_dir = DIR_CW;
static volatile uint8_t motor_step_index = 0;
static volatile uint16_t current_step_delay_ms = MIN_SPEED_DELAY_MS;
static volatile uint32_t ramp_step_counter = 0;

static volatile uint16_t timer_ms_count = 0;
static volatile uint32_t led_count = 0;

/**
 * @brief 根据方向和步进索引执行八拍序列中的一步。
 * @param dir：旋转方向（DIR_CW 或 DIR_CCW）。
 * @param step_index：序列中的当前步（0-7）。
 */
static void motor_execute_step(motor_dir_t dir, uint8_t step_index)
{
    if (dir == DIR_CW) 
    {
        switch (step_index) 
        {
            case 0: MOTOR_B2(0); MOTOR_A2(0); MOTOR_B1(0); MOTOR_A1(1); break;
            case 1: MOTOR_B2(0); MOTOR_A2(0); MOTOR_B1(1); MOTOR_A1(1); break;
            case 2: MOTOR_B2(0); MOTOR_A2(0); MOTOR_B1(1); MOTOR_A1(0); break;
            case 3: MOTOR_B2(0); MOTOR_A2(1); MOTOR_B1(1); MOTOR_A1(0); break;
            case 4: MOTOR_B2(0); MOTOR_A2(1); MOTOR_B1(0); MOTOR_A1(0); break;
            case 5: MOTOR_B2(1); MOTOR_A2(1); MOTOR_B1(0); MOTOR_A1(0); break;
            case 6: MOTOR_B2(1); MOTOR_A2(0); MOTOR_B1(0); MOTOR_A1(0); break;
            case 7: MOTOR_B2(1); MOTOR_A2(0); MOTOR_B1(0); MOTOR_A1(1); break;
        }
    }else{ 
        switch (step_index) 
        {
            case 0: MOTOR_B2(1); MOTOR_A2(0); MOTOR_B1(0); MOTOR_A1(1); break;
            case 1: MOTOR_B2(1); MOTOR_A2(0); MOTOR_B1(0); MOTOR_A1(0); break;
            case 2: MOTOR_B2(1); MOTOR_A2(1); MOTOR_B1(0); MOTOR_A1(0); break;
            case 3: MOTOR_B2(0); MOTOR_A2(1); MOTOR_B1(0); MOTOR_A1(0); break;
            case 4: MOTOR_B2(0); MOTOR_A2(1); MOTOR_B1(1); MOTOR_A1(0); break;
            case 5: MOTOR_B2(0); MOTOR_A2(0); MOTOR_B1(1); MOTOR_A1(0); break;
            case 6: MOTOR_B2(0); MOTOR_A2(0); MOTOR_B1(1); MOTOR_A1(1); break;
            case 7: MOTOR_B2(0); MOTOR_A2(0); MOTOR_B1(0); MOTOR_A1(1); break;
        }
    }
}

/**
 * @brief  Initializes the motor and stops it.
 * @retval None
 */
void motor_init(void)
{
    motor_state = MOTOR_STOPPED;
    MOTOR_A1(0);
    MOTOR_A2(0);
    MOTOR_B1(0);
    MOTOR_B2(0);
    HAL_TIM_Base_Start_IT(&htim2);
}

/**
 * @brief  Initiates the acceleration and run sequence.
 * @param  dir: The direction of rotation.
 * @retval None
 */
void motor_run(motor_dir_t dir)
{
    // Can only start from a stopped state
    if (motor_state == MOTOR_STOPPED)
    {
        current_motor_dir = dir;
        motor_step_index = 0;
        ramp_step_counter = 0;
        current_step_delay_ms = MIN_SPEED_DELAY_MS;
        motor_state = MOTOR_ACCELERATING;
    }
}

/**
 * @brief  Initiates the deceleration ramp to stop the motor smoothly.
 * @retval None
 */
void motor_stop(void)
{
    if (motor_state == MOTOR_RUNNING_AT_MAX_SPEED)
    {
        ramp_step_counter = RAMP_STEPS;
        motor_state = MOTOR_DECELERATING;
    }
    else if (motor_state == MOTOR_ACCELERATING)
    {
        motor_state = MOTOR_DECELERATING;
    }
}
/**
 * @brief  TIM period elapsed callback in non-blocking mode.
 * @param  htim: TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (motor_state == MOTOR_STOPPED) {
            // Blink LED even when stopped, as a system heartbeat
            led_count++;
            if(led_count >= 500) { // Slower blink when stopped
                led_count = 0;
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
            return;
        }

        led_count++;
        if(led_count >= 100)
        {
            led_count = 0;
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        }

        timer_ms_count++;
        if (timer_ms_count >= current_step_delay_ms)
        {
            timer_ms_count = 0; 
            motor_execute_step(current_motor_dir, motor_step_index);
            motor_step_index = (motor_step_index + 1) % 8;

            switch (motor_state)
            {
                case MOTOR_ACCELERATING:
                    ramp_step_counter++;
                    if (ramp_step_counter % 2 == 0) 
                    { 
                        if (current_step_delay_ms > MAX_SPEED_DELAY_MS)
                        {
                            current_step_delay_ms--;
                        }
                    }
                    if (ramp_step_counter >= RAMP_STEPS)
                    {
                        motor_state = MOTOR_RUNNING_AT_MAX_SPEED;
                    }
                    break;

                case MOTOR_DECELERATING:
                    ramp_step_counter--;
                    // A simple linear ramp: increase delay every few steps
                    if (ramp_step_counter % 2 == 0) { 
                        if (current_step_delay_ms < MIN_SPEED_DELAY_MS) {
                            current_step_delay_ms++;
                        }
                    }
                    if (ramp_step_counter == 0 ) {
                        motor_init();
                    }
                    break;

                case MOTOR_RUNNING_AT_MAX_SPEED:
                    break;
                    
                case MOTOR_STOPPED:
                    motor_init();
                    break;
            }
        }
    }
}

