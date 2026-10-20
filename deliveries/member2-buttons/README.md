# Member 2：按钮与电机方向

本目录基于 Member 3 的 20 kHz PWM 工程，增加两个按键中断与 AT8236 正反转控制。

## 接线

| 模块信号 | NUCLEO-F446RE | 用途 |
| --- | --- | --- |
| 按键板 GND | GND | 共地 |
| 按键板 K1 | PB12 | 正转，下降沿 EXTI12 |
| 按键板 K2 | PB14 | 反转，下降沿 EXTI14 |
| AT8236 GND | GND | 必须与 Nucleo 共地 |
| AT8236 AIN1 | PB6 / TIM4_CH1 | 正转 20 kHz PWM |
| AT8236 AIN2 | PB7 / TIM4_CH2 | 反转 20 kHz PWM |

电机接 AT8236 的 AO1/AO2。若实际转向与“正转/反转”定义相反，交换 AO1/AO2 即可，不要改控制逻辑。

## 按键逻辑

- 上电保持停止，预设测试速度为 30%。
- 按 K1 后持续正转；按 K2 后持续反转，不需要长按。
- 按键中断采用 40 ms 软件消抖。
- 换向时两路 PWM 先归零 50 ms，再启动相反方向，避免 H 桥直接反接。
- `Motor_SetSpeed(0..100)` 接口保持不变，Member 5 可直接用 ADC 结果更新速度。

## 主要接口

- `ButtonControl_EXTI_Callback(pin)`：由 HAL EXTI 回调调用，只记录事件。
- `ButtonControl_Task()`：在主循环处理按键命令。
- `Motor_RequestDirection(MOTOR_FORWARD/MOTOR_REVERSE/MOTOR_STOP)`：请求方向。
- `Motor_Task()`：执行非阻塞换向死区。
- `Motor_SetSpeed(0..100)`：设置占空比。

工程路径：`Block2_CodingDay/`。STM32CubeIDE 导入后可直接编译。
