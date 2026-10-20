# Member 2：按钮与电机方向

本目录基于 Member 3 的 20 kHz PWM 工程，增加两个按键中断与 AT8236 正反转控制。

## 接线

| 模块信号 | NUCLEO-F446RE | 用途 |
| --- | --- | --- |
| 按键板 VCC | CN7-16 / 3V3 | 按键板供电 |
| 按键板 GND | CN7-20 / GND | 共地 |
| 按键板 K1（上箭头） | CN10-16 / PB12 | 小车前进，下降沿 EXTI12 |
| 按键板 K2（下箭头） | CN10-28 / PB14 | 小车后退，下降沿 EXTI14 |
| 按键板 K3（#） | CN10-26 / PB15 | 停止，下降沿 EXTI15 |
| AT8236 GND | GND | 必须与 Nucleo 共地 |
| AT8236 AIN1（黑线） | CN10-17 / PB6 / D10 / TIM4_CH1 | 20 kHz PWM |
| AT8236 AIN2（白线） | CN10-23 / PA8 / D7 | 方向控制 |
| AT8236 GND（灰线） | CN10-20 / GND | 共地 |

电机接 AT8236 的 AO1/AO2，现有电机线保持不动。若实车前进/后退方向与按键定义相反，应在 `Core/Src/button_control.c` 中交换 K1、K2 对应的 `MOTOR_FORWARD` / `MOTOR_REVERSE` 命令，不需要改动 AO1/AO2 接线。

## 按键逻辑

- 上电保持停止，预设测试速度为 30%。
- 按 K1 后小车持续前进；按 K2 后小车持续后退；按 K3 停止，均不需要长按。由于电机安装方向与电机轴正反转定义相反，K1 映射到 `MOTOR_REVERSE`，K2 映射到 `MOTOR_FORWARD`。
- 按键中断采用 40 ms 软件消抖。
- 启动或换向时先停止输出 150 ms，再启动目标方向，降低 H 桥换向电流；重复按 K1/K2 也会重新启动目标方向。
- `Motor_SetSpeed(0..100)` 接口保持不变，Member 5 可直接用 ADC 结果更新速度。

## 主要接口

- `ButtonControl_EXTI_Callback(pin)`：由 HAL EXTI 回调调用，只记录事件。
- `ButtonControl_Task()`：在主循环处理按键命令。
- `Motor_RequestDirection(MOTOR_FORWARD/MOTOR_REVERSE/MOTOR_STOP)`：请求方向。
- `Motor_Task()`：执行非阻塞换向死区。
- `Motor_SetSpeed(0..100)`：设置占空比。

工程路径：`Block2_CodingDay/`。STM32CubeIDE 导入后可直接编译。

成员5接入 ADC 调速前请先阅读 [`Block2_CodingDay/MEMBER2_HANDOFF.md`](Block2_CodingDay/MEMBER2_HANDOFF.md)。
