# Member 3 交接核对

## 已实现

- MCU 为 STM32F446RE，系统时钟 84 MHz，APB1 定时器时钟 84 MHz。
- `Core/Src/motor_pwm.c` 直接配置 TIM4_CH1 / PB6 输出硬件 PWM。
- TIM4 配置为 PSC = 0、ARR = 4199，对应 20 kHz、50 us 周期。
- `Motor_PWM_Init()` 初始化 GPIOB、TIM4 和 PWM 通道。
- `Motor_SetSpeed(uint8_t speed)` 接收 0 到 100，并更新 TIM4 CCR1。
- `main.c` 启动时调用 `Motor_PWM_Init()` 和 `Motor_SetSpeed(0)`，复位后默认不输出有效驱动占空比。
- PC13 已配置为下降沿 EXTI，当前回调只切换板载 LED，用于验证中断链路。

## 尚未实现

- 第二个按钮及其 EXTI 配置。
- 两个按钮对应的正转和反转状态处理、消抖及同时按下规则。
- AT8236 的第二个方向输入和正反转真值表。
- ADC 调速、编码器输入捕获、编码器计算和数据显示。

## 集成接口

```c
#include "motor_pwm.h"

Motor_PWM_Init();
Motor_SetSpeed(speed_percent);  /* 0..100 */
```

PB6 已被 TIM4_CH1 占用。PA0 预留给 ADC1_IN0，PA7 预留给 TIM3_CH2。交接建议使用 PA8 和 PA9 作为方向控制引脚，但该建议尚未在硬件上验证。

## 验证与限制

交接说明记录了逻辑分析仪测得的 20 kHz PWM，并称 20% 和 50% 占空比正常。当前提交没有附带波形图片，因此仓库只能核对配置和源码，不能独立复核测量证据。

成员3记录的硬件问题是 AT8236 接入 12 V 后无指示灯且电机不转。PWM 波形与软件接口已完成，但电机驱动板供电、接线、跳线和方向输入仍需团队现场排查。在排除该问题前，不能把电机转动标记为已验证。
