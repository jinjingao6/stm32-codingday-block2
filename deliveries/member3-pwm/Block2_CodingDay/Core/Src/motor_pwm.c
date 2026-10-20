#include "motor_pwm.h"

// 纯寄存器初始化，完全不需要 HAL 库
void Motor_PWM_Init(void) {
    // 1. 开启 GPIOB 和 TIM4 的时钟 (RCC 寄存器)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // 2. 配置 PB6 为复用功能 AF2 (TIM4_CH1)
    // 清除 PB6 的模式位 (MODER)
    GPIOB->MODER &= ~(3U << (6 * 2));
    // 设置 PB6 为复用模式 (10)
    GPIOB->MODER |= (2U << (6 * 2));
    // 清除 PB6 的复用功能寄存器高位 (AFR[0] 负责 0-7 引脚)
    GPIOB->AFR[0] &= ~(0xFU << (6 * 4));
    // 设置 PB6 为 AF2 (TIM4_CH1)
    GPIOB->AFR[0] |= (2U << (6 * 4));

    // 3. 配置 TIM4 的时基
    // 时钟频率 84MHz，预分频 PSC = 0，自动重装载 ARR = 4199
    TIM4->PSC = 0;
    TIM4->ARR = 4199;

    // 4. 配置通道 1 为 PWM 模式 1
    // 清除通道 1 配置位
    TIM4->CCMR1 &= ~(0xFFU);
    // 设置 PWM 模式 1 (OC1M = 110)，使能预装载 (OC1PE = 1)
    TIM4->CCMR1 |= (6U << 4) | (1U << 3);
    // 使能通道 1 输出 (CC1E = 1)，高电平有效 (CC1P = 0)
    TIM4->CCER |= (1U << 0);

    // 5. 设置初始占空比 50% (CCR1 = 2100)
    TIM4->CCR1 = 2100;

    // 6. 使能定时器
    TIM4->CR1 |= (1U << 0);
}

// 调速函数：传入 0-100
void Motor_SetSpeed(uint8_t speed) {
    if (speed > 100) speed = 100;
    // 直接修改 CCR1 寄存器
    TIM4->CCR1 = (speed * 4200) / 100;
}
