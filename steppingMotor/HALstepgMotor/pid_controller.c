#include "pid_controller.h"

/**
 * @brief Initializes the PID controller.
 * @param pid Pointer to the PID_Controller_t structure.
 * @param Kp Proportional gain.
 * @param Ki Integral gain.
 * @param Kd Derivative gain.
 * @param output_min Minimum output value.
 * @param output_max Maximum output value.
 */
void pid_controller_init(PID_Controller_t *pid, float Kp, float Ki, float Kd, float output_min, float output_max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid_controller_reset(pid);
}

/**
 * @brief Updates the PID controller.
 * @param pid Pointer to the PID_Controller_t structure.
 * @param target The desired value.
 * @param actual The current value.
 * @return The PID controller output.
 */
float pid_controller_update(PID_Controller_t *pid, float target, float actual)
{
    pid->target = target;
    pid->actual = actual;
    pid->error = pid->target - pid->actual;

    pid->integral += pid->error;
    // Integral windup protection
    if (pid->integral > pid->output_max) {
        pid->integral = pid->output_max;
    } else if (pid->integral < pid->output_min) {
        pid->integral = pid->output_min;
    }

    float derivative = pid->error - pid->last_error;
    pid->last_error = pid->error;

    float output = pid->Kp * pid->error + pid->Ki * pid->integral + pid->Kd * derivative;

    // Output saturation
    if (output > pid->output_max) {
        output = pid->output_max;
    } else if (output < pid->output_min) {
        output = pid->output_min;
    }

    return output;
}

/**
 * @brief Resets the PID controller internal states.
 * @param pid Pointer to the PID_Controller_t structure.
 */
void pid_controller_reset(PID_Controller_t *pid)
{
    pid->error = 0;
    pid->last_error = 0;
    pid->integral = 0;
}


