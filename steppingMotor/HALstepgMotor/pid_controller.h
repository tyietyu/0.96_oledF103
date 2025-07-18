#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>

// PID Controller Structure
typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float target;
    float actual;

    float error;
    float last_error;
    float integral;

    float output_min;
    float output_max;
} PID_Controller_t;
extern PID_Controller_t pid_controller;

void pid_controller_init(PID_Controller_t *pid, float Kp, float Ki, float Kd, float output_min, float output_max);
float pid_controller_update(PID_Controller_t *pid, float target, float actual);
void pid_controller_reset(PID_Controller_t *pid);

#endif // PID_CONTROLLER_H

