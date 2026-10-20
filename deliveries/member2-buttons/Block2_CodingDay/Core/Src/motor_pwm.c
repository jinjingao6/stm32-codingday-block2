#include "motor_pwm.h"

#define MOTOR_PWM_PERIOD_COUNTS 4200U
#define MOTOR_DIRECTION_DEADTIME_MS 50U

static uint8_t requested_speed;
static MotorDirection_t active_direction;
static MotorDirection_t pending_direction;
static uint32_t direction_change_tick;
static uint8_t direction_change_pending;

static void Motor_ApplyOutputs(void)
{
    uint32_t compare = ((uint32_t)requested_speed * MOTOR_PWM_PERIOD_COUNTS) / 100U;

    if (active_direction == MOTOR_FORWARD)
    {
        TIM4->CCR1 = compare;
        TIM4->CCR2 = 0U;
    }
    else if (active_direction == MOTOR_REVERSE)
    {
        TIM4->CCR1 = 0U;
        TIM4->CCR2 = compare;
    }
    else
    {
        TIM4->CCR1 = 0U;
        TIM4->CCR2 = 0U;
    }
}

void Motor_PWM_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* PB6 = TIM4_CH1 (AIN1), PB7 = TIM4_CH2 (AIN2). */
    GPIOB->MODER &= ~((3U << (6U * 2U)) | (3U << (7U * 2U)));
    GPIOB->MODER |= (2U << (6U * 2U)) | (2U << (7U * 2U));
    GPIOB->OTYPER &= ~((1U << 6U) | (1U << 7U));
    GPIOB->OSPEEDR |= (3U << (6U * 2U)) | (3U << (7U * 2U));
    GPIOB->PUPDR &= ~((3U << (6U * 2U)) | (3U << (7U * 2U)));
    GPIOB->AFR[0] &= ~((0xFU << (6U * 4U)) | (0xFU << (7U * 4U)));
    GPIOB->AFR[0] |= (2U << (6U * 4U)) | (2U << (7U * 4U));

    TIM4->CR1 = 0U;
    TIM4->PSC = 0U;
    TIM4->ARR = MOTOR_PWM_PERIOD_COUNTS - 1U;
    TIM4->CCR1 = 0U;
    TIM4->CCR2 = 0U;

    /* CH1 and CH2: PWM mode 1 with preload. */
    TIM4->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE |
                  (6U << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
    TIM4->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM4->EGR = TIM_EGR_UG;
    TIM4->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;

    requested_speed = 0U;
    active_direction = MOTOR_STOP;
    pending_direction = MOTOR_STOP;
    direction_change_pending = 0U;
}

void Motor_SetSpeed(uint8_t speed)
{
    requested_speed = (speed > 100U) ? 100U : speed;
    if (direction_change_pending == 0U)
    {
        Motor_ApplyOutputs();
    }
}

void Motor_RequestDirection(MotorDirection_t direction)
{
    if ((direction != MOTOR_STOP) &&
        (direction != MOTOR_FORWARD) &&
        (direction != MOTOR_REVERSE))
    {
        return;
    }

    if (direction == MOTOR_STOP)
    {
        active_direction = MOTOR_STOP;
        pending_direction = MOTOR_STOP;
        direction_change_pending = 0U;
        Motor_ApplyOutputs();
        return;
    }

    if ((direction == active_direction) && (direction_change_pending == 0U))
    {
        return;
    }

    /* Coast before changing polarity to avoid an abrupt bridge reversal. */
    TIM4->CCR1 = 0U;
    TIM4->CCR2 = 0U;
    pending_direction = direction;
    direction_change_tick = HAL_GetTick();
    direction_change_pending = 1U;
}

void Motor_Task(void)
{
    if ((direction_change_pending != 0U) &&
        ((HAL_GetTick() - direction_change_tick) >= MOTOR_DIRECTION_DEADTIME_MS))
    {
        active_direction = pending_direction;
        direction_change_pending = 0U;
        Motor_ApplyOutputs();
    }
}

MotorDirection_t Motor_GetDirection(void)
{
    return active_direction;
}
