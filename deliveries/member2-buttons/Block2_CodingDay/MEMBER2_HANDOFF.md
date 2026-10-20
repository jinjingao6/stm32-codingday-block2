# Member 2 → Member 5 对接说明

## 1. 交接目标

成员2已经完成按键、方向、停止和换向保护。成员5只需使用 ADC1_IN0 / PA0 采集电位器，并把 ADC 数值换算成 0～100 的速度百分比后调用 `Motor_SetSpeed()`，不要重新配置 PWM 或另写电机方向控制。

## 2. 当前已验证功能

- STM32F446RE 系统时钟为 84 MHz。
- TIM4_CH1 / PB6 输出 20 kHz PWM，连接 AT8236 AIN1。
- PA8 为方向 GPIO，连接 AT8236 AIN2。
- K1 / PB12 控制小车前进，软件命令为 `MOTOR_REVERSE`。
- K2 / PB14 控制小车后退，软件命令为 `MOTOR_FORWARD`。
- K3 / PB15 调用 `MOTOR_STOP`，立即停止并取消待执行换向。
- 按键使用下降沿 EXTI 和 40 ms 软件消抖。
- 启动、重复启动及换向前均停止输出 150 ms，降低换向冲击。
- 工程已经在 STM32CubeIDE 2.2.0 下 clean build，结果为 0 errors、0 warnings，并通过实车测试。

注意：`MOTOR_FORWARD` / `MOTOR_REVERSE` 表示电机电气方向。由于小车电机安装方向，当前实车验证后的映射为 K1（前进）→ `MOTOR_REVERSE`、K2（后退）→ `MOTOR_FORWARD`。AO1/AO2 电机接线固定不改；如果换用硬件后实车方向相反，应在 `button_control.c` 中交换 K1/K2的软件命令映射。

## 3. 固定引脚和接线

| 功能 | STM32 引脚 | 实物连接 |
| --- | --- | --- |
| 20 kHz PWM | PB6 / TIM4_CH1 / CN10-17 | AT8236 AIN1 |
| 电机方向 | PA8 / CN10-23 | AT8236 AIN2 |
| 前进键 K1 | PB12 / CN10-16 | 按键板 K1 |
| 后退键 K2 | PB14 / CN10-28 | 按键板 K2 |
| 停止键 K3 | PB15 / CN10-26 | 按键板 K3 |
| ADC 电位器 | PA0 / ADC1_IN0 | 预留给成员5 |
| 编码器捕获 | PA7 / TIM3_CH2 | 预留给成员4 |

成员5不得占用 PB6、PA8、PB12、PB14、PB15 和 PA7。所有模块必须共地。

## 4. 成员5使用的接口

头文件：

```c
#include "motor_pwm.h"
```

速度接口：

```c
void Motor_SetSpeed(uint8_t speed);
```

- 参数范围：0～100。
- 大于 100 的值会自动限制为 100。
- `Motor_SetSpeed(0)` 只把速度降为零，不改变当前方向状态。
- 之后调用 `Motor_SetSpeed(nonzero)` 会按原方向恢复运行。
- 若需要明确停止并取消换向，应调用：

```c
Motor_RequestDirection(MOTOR_STOP);
```

方向相关接口由成员2和最终应用层使用：

```c
Motor_RequestDirection(MOTOR_FORWARD);
Motor_RequestDirection(MOTOR_REVERSE);
Motor_RequestDirection(MOTOR_STOP);
Motor_GetDirection();
```

不要在成员5代码中再次调用 `Motor_PWM_Init()`；主程序已经完成初始化。

## 5. 推荐的 ADC 数据接口

建议 ADC 使用 12 位分辨率，原始范围为 0～4095。ADC 中断中只保存采样值和置更新标志，不在中断中执行延时、打印或复杂计算。

```c
static volatile uint16_t adc_raw;
static volatile uint8_t adc_updated;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_raw = (uint16_t)HAL_ADC_GetValue(hadc);
        adc_updated = 1U;
    }
}
```

在主循环或 `ADC_Task()` 中完成换算并更新速度：

```c
if (adc_updated != 0U)
{
    uint16_t raw;

    __disable_irq();
    raw = adc_raw;
    adc_updated = 0U;
    __enable_irq();

    uint8_t speed_percent = (uint8_t)(((uint32_t)raw * 100U) / 4095U);
    Motor_SetSpeed(speed_percent);
}
```

如果电位器在零点附近存在抖动，建议增加死区，例如 ADC 小于 80 时输出 0：

```c
if (raw < 80U)
{
    speed_percent = 0U;
}
```

为避免 ADC 噪声导致 PWM 频繁变化，建议只在速度百分比变化至少 1～2% 时调用一次 `Motor_SetSpeed()`。

## 6. `main.c` 集成位置

当前初始化顺序：

```c
MX_GPIO_Init();
Motor_PWM_Init();
ButtonControl_Init();
Motor_SetSpeed(30);       /* 成员2的固定测试速度 */
```

成员5接入后建议改为：

```c
MX_GPIO_Init();
MX_ADC1_Init();
Motor_PWM_Init();
ButtonControl_Init();
Motor_SetSpeed(0);        /* 等待第一份有效 ADC 数据 */
HAL_ADC_Start_IT(&hadc1);
```

主循环必须保留成员2任务，并加入 ADC 任务：

```c
while (1)
{
    ButtonControl_Task();
    Motor_Task();
    ADC_Task();
}
```

`Motor_Task()` 不可删除，否则 150 ms 非阻塞换向流程无法完成。

## 7. HAL 回调合并规则

- 工程已经定义 `HAL_GPIO_EXTI_Callback()`，成员5不要重复定义同名 GPIO 回调。
- 如果成员5使用 `HAL_ADC_ConvCpltCallback()`，当前工程没有同名实现，可以新增。
- 后续多个模块需要同一 HAL 回调，必须在一个回调中按外设实例分派，不能在不同 `.c` 文件重复定义。
- 中断中只采样和置标志，业务逻辑放在主循环任务中。

## 8. 不可改动项

- 不要用 CubeMX 重新生成或覆盖 `Core/Src/motor_pwm.c`。
- 不要改变 TIM4、PB6、20 kHz PWM、PA8 方向控制和现有按键引脚。
- 不要在 ADC 中断内调用 `HAL_Delay()`。
- 不要删除 `ButtonControl_Task()` 或 `Motor_Task()`。
- 不要把 ADC 调速写成新的 PWM 初始化模块；唯一速度入口为 `Motor_SetSpeed(0..100)`。

## 9. 集成验证步骤

1. 车轮架空，上电后 ADC 最低位置应使电机保持停止。
2. 缓慢旋转电位器，确认 `Motor_SetSpeed()` 输入从 0 平滑变化到 100。
3. 按 K1，确认小车方向为前进；旋转电位器，确认速度可调。
4. 按 K2，确认停顿约 150 ms 后反向；速度仍由 ADC 控制。
5. 按 K3，确认立即停止且不会在 150 ms 后自行启动。
6. K3 后按 K1 或 K2，确认按当前 ADC 速度重新启动。
7. 用逻辑分析仪确认 PB6 频率保持 20 kHz，ADC 只改变占空比、不改变频率。
8. 完成 clean build，要求 0 errors、0 warnings。

## 10. 交付给成员1时需要说明

- ADC 原始范围、分辨率、采样频率和更新标志。
- ADC 到 0～100% 的映射公式、零点死区和滤波方法。
- 是否限制最低启动占空比。
- `Motor_SetSpeed()` 的调用频率。
- 实车验证结果及逻辑分析仪波形。

## 11. 成员2所做的具体改动

本工程以成员3的单路 20 kHz PWM 工程为基础。成员3提供了 TIM4_CH1 / PB6 的 PWM 初始化和 `Motor_SetSpeed(0..100)`；成员2没有改变 20 kHz 频率要求，在此基础上完成了以下工作：

### 11.1 硬件和引脚确定

- 核对 NUCLEO-F446RE、AT8236 和 HW-937AB 按键板的实际排针。
- 将 PB6 / CN10-17 保留为 AT8236 AIN1 的 20 kHz PWM。
- 将 PA8 / CN10-23 确定为 AT8236 AIN2 的方向控制输出。
- 将 PB12 / CN10-16、PB14 / CN10-28 和 PB15 / CN10-26 分别分配给 K1、K2、K3。
- 修正早期测试中 AIN2 使用 PC7 / D9 的临时配置，使代码、`.ioc`、README 和最终实物接线统一为 PA8 / D7。
- 保留 PA0 给成员5的 ADC1_IN0，保留 PA7 给成员4的 TIM3_CH2 输入捕获。

### 11.2 按键输入和中断

- 新增 `button_control.c/.h`，把按键处理从电机驱动中独立出来。
- 将 K1、K2、K3 配置为内部上拉、下降沿 EXTI 输入。
- 在共享的 `EXTI15_10_IRQHandler()` 中分别分派 EXTI12、EXTI14 和 EXTI15。
- 在 `HAL_GPIO_EXTI_Callback()` 中统一转交给 `ButtonControl_EXTI_Callback()`，避免在中断处理函数中执行复杂业务。
- 使用待处理命令标志，由 `ButtonControl_Task()` 在主循环执行方向请求。

### 11.3 按键消抖

- 为三个按键分别记录最近一次有效触发时间，互不干扰。
- 使用 `HAL_GetTick()` 实现 40 ms 软件消抖，防止一次机械按键抖动被识别为多次操作。
- 中断中不使用 `HAL_Delay()`，不会阻塞系统节拍和其他中断。

### 11.4 方向与停止控制

- 在 `motor_pwm.h` 中定义 `MOTOR_STOP`、`MOTOR_FORWARD` 和 `MOTOR_REVERSE` 三种状态。
- 新增 `Motor_RequestDirection()`、`Motor_Task()` 和 `Motor_GetDirection()` 接口。
- PA8 / AIN2 拉低时配合 PB6 PWM 驱动一个方向；PA8 / AIN2 拉高时使用互补 PWM 比较值驱动另一方向。
- 根据实车运动方向调整最终映射：K1 为小车前进、K2 为小车后退。
- 增加 K3 停止功能：立即关闭 PWM，并取消尚未完成的换向请求。

### 11.5 换向保护和故障修复

- 实现非阻塞换向状态机，换向时先关闭输出，再经过安全等待后启动目标方向。
- 最初保护时间为 50 ms；根据实车快速换向时偶发停转现象，将其增加到 150 ms。
- 修复“软件认为方向未改变而忽略同方向按键”的问题：现在重复按 K1 或 K2 也会执行一次安全停止和重新启动，可从偶发驱动保护状态恢复。
- 换向等待使用 `Motor_Task()` 和 `HAL_GetTick()` 完成，不使用阻塞式延时。

### 11.6 主程序和配置文件

- 在初始化阶段加入 `Motor_PWM_Init()`、`ButtonControl_Init()` 和测试速度设置。
- 在主循环持续调用 `ButtonControl_Task()` 和 `Motor_Task()`。
- 更新 `main.h`、`main.c`、`stm32f4xx_it.c` 和 CubeMX `.ioc` 中的按键、EXTI及 PA8 配置。
- 编写成员2 README，记录最终接线、按键逻辑、公开接口和安全行为。

### 11.7 验证结果

- 在 STM32CubeIDE 2.2.0 中多次执行 clean build，均为 0 errors、0 warnings。
- 完成程序烧录和实车验证：K1 前进、K2 后退、K3 停止。
- 验证快速换向会先进入约 150 ms 安全停顿，然后按目标方向重新启动。
- 验证停止后可以通过 K1 或 K2 按当前设定速度重新启动。

