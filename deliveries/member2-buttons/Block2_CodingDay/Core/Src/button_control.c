#include "button_control.h"
#include "motor_pwm.h"

#define BUTTON_DEBOUNCE_MS 40U

static volatile MotorDirection_t pending_command;
static volatile uint8_t command_pending;
static uint32_t forward_last_tick;
static uint32_t reverse_last_tick;
static uint32_t stop_last_tick;

void ButtonControl_Init(void)
{
    pending_command = MOTOR_STOP;
    command_pending = 0U;
    forward_last_tick = HAL_GetTick() - BUTTON_DEBOUNCE_MS;
    reverse_last_tick = HAL_GetTick() - BUTTON_DEBOUNCE_MS;
    stop_last_tick = HAL_GetTick() - BUTTON_DEBOUNCE_MS;
}

void ButtonControl_EXTI_Callback(uint16_t gpio_pin)
{
    uint32_t now = HAL_GetTick();

    if ((gpio_pin == FORWARD_BTN_Pin) &&
        ((now - forward_last_tick) >= BUTTON_DEBOUNCE_MS))
    {
        forward_last_tick = now;
        pending_command = MOTOR_FORWARD;
        command_pending = 1U;
    }
    else if ((gpio_pin == REVERSE_BTN_Pin) &&
             ((now - reverse_last_tick) >= BUTTON_DEBOUNCE_MS))
    {
        reverse_last_tick = now;
        pending_command = MOTOR_REVERSE;
        command_pending = 1U;
    }
    else if ((gpio_pin == STOP_BTN_Pin) &&
             ((now - stop_last_tick) >= BUTTON_DEBOUNCE_MS))
    {
        stop_last_tick = now;
        pending_command = MOTOR_STOP;
        command_pending = 1U;
    }
}

void ButtonControl_Task(void)
{
    MotorDirection_t command;

    if (command_pending == 0U)
    {
        return;
    }

    __disable_irq();
    command = pending_command;
    command_pending = 0U;
    __enable_irq();

    Motor_RequestDirection(command);
}
