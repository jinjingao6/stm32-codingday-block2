# STM32 Coding Day — Block 2

本仓库用于成员交付、接口约定和最终工程集成。任务依据：课程 Block2.5-CodingDay_v3.pptx。下列目录与交付流程为小组协作建议，不属于老师额外布置的要求。

## 成员目录

| 成员 | 职责 | 交付目录 |
| --- | --- | --- |
| Member 1 | 完整应用与数据展示 | [member1-application](deliveries/member1-application/) |
| Member 2 | 两个按钮及中断 | [member2-buttons](deliveries/member2-buttons/) |
| Member 3 | TIM4 PWM | [member3-pwm](deliveries/member3-pwm/) |
| Member 4 | TIM3 输入捕获及中断 | [member4-input-capture](deliveries/member4-input-capture/) |
| Member 5 | ADC 及中断 | [member5-adc](deliveries/member5-adc/) |
| Member 6 | 编码器计算与测量下限 | [member6-encoder](deliveries/member6-encoder/) |

GitHub 用户名与职责的对应关系由小组确认。仓库拥有者需在 Settings → Collaborators 中邀请组员；组员接受邀请后才能向私有仓库提交文件。

## 如何上传自己的交付

1. 打开自己的 deliveries 子目录，选择 Add file → Upload files。
2. 上传压缩包与填写后的 [交付说明模板](docs/delivery-template.md)。建议命名为 member3_pwm_v01.zip 和 member3_pwm_v01.md。
3. 填写有意义的提交说明，例如“Member 3：20 kHz PWM 初版，已用逻辑分析仪验证”，然后提交。若仓库规则要求 Pull Request，则按页面提示创建。
4. 更新时使用新版本号，说明修复内容。不要覆盖其他成员的文件。

网页上传单个文件上限为 25 MiB。压缩包应保留源码、.ioc、工程配置和需要的依赖，移除可重新生成的编译输出。不要上传密码、令牌、IDE 工作区缓存或软件安装包。课程课件和第三方材料的分享应遵守课程规定。

最终可审阅的源码存放在 [integrated-project](integrated-project/)，仅有 ZIP 不便查看代码差异。建议由 Member 1 维护集成工程，其他成员通过分支和 Pull Request 修改集成源码；上传个人交付与修改集成工程是两种不同操作。

## 课程验收要求

- 使用课程提供的 STM32 HAL 代码进行整合和改编。
- 两个按钮以中断方式控制 MC520 正转和反转。
- TIM4_CH1 / PB6 输出 20 kHz 硬件 PWM，通过 AT8236 驱动电机。
- TIM3_CH2 / PA7 以中断方式捕获编码器边沿对应的定时器计数。
- ADC1_IN0 / PA0 以中断方式读取电位器，并用于调整 PWM 占空比。
- 计算编码器脉冲周期（s）、频率（Hz）、信号占空比（%）、减速后输出轴转速（RPM）和最低可测输出轴转速（RPM）。
- 使用 SWV printf 或 STM32CubeMonitor 展示计算参数，OLED 可选。
- 通过功能测试、运行监测、逻辑分析仪和调试验证。PWM 频率应为 20 kHz，周期应为 50 μs。
- 每位成员参与演示并回答自己负责部分的问题。整组不完整则所有成员不得分；正式考核期间不能修改代码或调参。

工具：课程指定 STM32CubeIDE 1.17.0；STM32CubeMonitor 使用课程要求的最新版本，组内记录实际版本。

最迟 2026 年 10 月 22 日（周四）下午开始集成。具体交付时间由小组确认，建议周四上午前提供可集成版本。

参阅 [接口约定](docs/integration-agreement.md) 与 [验收清单](docs/validation-checklist.md)。
