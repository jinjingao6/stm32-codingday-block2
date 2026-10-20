#ifndef INC_MOTOR_PWM_H_
#define INC_MOTOR_PWM_H_

#include "main.h"
#include "stm32f4xx_hal.h"   // 必须加这个，否则找不到 TIM 相关的定义

void Motor_PWM_Init(void);
void Motor_SetSpeed(uint8_t speed);

#endif
