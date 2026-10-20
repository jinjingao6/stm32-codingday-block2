#ifndef INC_BUTTON_CONTROL_H_
#define INC_BUTTON_CONTROL_H_

#include "main.h"

void ButtonControl_Init(void);
void ButtonControl_EXTI_Callback(uint16_t gpio_pin);
void ButtonControl_Task(void);

#endif
