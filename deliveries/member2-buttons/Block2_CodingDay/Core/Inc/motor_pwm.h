#ifndef INC_MOTOR_PWM_H_
#define INC_MOTOR_PWM_H_

#include "main.h"
#include "stm32f4xx_hal.h"

typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_REVERSE
} MotorDirection_t;

void Motor_PWM_Init(void);
void Motor_SetSpeed(uint8_t speed);
void Motor_RequestDirection(MotorDirection_t direction);
void Motor_Task(void);
MotorDirection_t Motor_GetDirection(void);

#endif
