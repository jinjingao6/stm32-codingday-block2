#include "motor_pwm.h"

#define MOTOR_PWM_PERIOD_COUNTS 4200U
#define MOTOR_DIRECTION_DEADTIME_MS 150U

static uint8_t requested_speed;
static MotorDirection_t active_direction;
static MotorDirection_t pending_direction;
static uint32_t direction_change_tick;
static uint8_t direction_change_pending;

static void Motor_ApplyOutputs(void)
{
    uint32_t compare = ((uint32_t)requested_speed * MOTOR_PWM_PERIOD_COUNTS) / 100U;

    if ((active_direction == MOTOR_STOP) || (requested_speed == 0U))
    {
        TIM4->CCR1 = 0U;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    }
    else if (active_direction == MOTOR_FORWARD)
    {
        /* AIN1 = PWM, AIN2 = 0: forward, fast decay. */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
        TIM4->CCR1 = compare;
    }
    else if (active_direction == MOTOR_REVERSE)
    {
        /* AIN1 = PWM, AIN2 = 1: reverse during PWM-low time. */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
        TIM4->CCR1 = MOTOR_PWM_PERIOD_COUNTS - compare;
    }
}

void Motor_PWM_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* Final wiring: PB6/D10 = TIM4_CH1 (AIN1), PA8/D7 = AIN2. */
    GPIOB->MODER &= ~(3U << (6U * 2U));
    GPIOB->MODER |= (2U << (6U * 2U));
    GPIOB->OTYPER &= ~(1U << 6U);
    GPIOB->OSPEEDR |= (3U << (6U * 2U));
    GPIOB->PUPDR &= ~(3U << (6U * 2U));
    GPIOB->AFR[0] &= ~(0xFU << (6U * 4U));
    GPIOB->AFR[0] |= (2U << (6U * 4U));

    GPIOA->MODER &= ~(3U << (8U * 2U));
    GPIOA->MODER |= (1U << (8U * 2U));
    GPIOA->OTYPER &= ~(1U << 8U);
    GPIOA->PUPDR &= ~(3U << (8U * 2U));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

    TIM4->CR1 = 0U;
    TIM4->PSC = 0U;
    TIM4->ARR = MOTOR_PWM_PERIOD_COUNTS - 1U;
    TIM4->CCR1 = 0U;

    /* CH1: PWM mode 1 with preload. */
    TIM4->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM4->CCER = TIM_CCER_CC1E;
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

    /* Coast before every start/restart. Repeated K1/K2 presses can therefore
       recover from a driver protection event instead of being ignored. */
    TIM4->CCR1 = 0U;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
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
