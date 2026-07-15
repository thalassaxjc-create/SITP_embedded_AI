# Ablation comparison v2

比较完全相同的数据划分与网络宽度；唯一关键差异是 full model 读取全部 14 维，而 weather-only model 只能读取前 10 维。

| metric | 14D full | 10D weather-only | difference |
|---|---:|---:|---:|
| 普通 test set accuracy | 0.9697 | 0.6559 | +0.3138 |
| controlled contrast accuracy | 0.9700 | 0.5006 | +0.4694 |
| personality sensitivity score | 0.3229 | 0.0000 | +0.3229 |
| 随 personality 改变预测类别的场景数（共 5） | 5 | 0 | +5 |

Weather-only model 对同一组固定 weather 的四种 personality 收到的是完全相同的 10D 输入，因此 sensitivity score 理论上为 0，也无法按 personality 改变输出。Controlled contrast accuracy 直接衡量这一信息缺失。

## 验收问题

- **Does personality input affect mood selection?** 是。14D 模型的平均目标概率增量为 0.3229。
- **Is the effect visible in controlled sensitivity tests?** 是。14D 模型在 5/5 个固定天气场景中随 personality 改变最终类别。
- **Does the 14D full model outperform the 10D weather-only model on personality-dependent samples?** 是。Controlled contrast accuracy 差值为 +0.4694。

## 结论

当前 v2 训练数据明确把 personality 纳入 label 生成逻辑。Sensitivity test 检查固定天气下各 personality 对对应 mood 概率的定向作用；ablation 则通过移除后四维验证性能变化。

Sensitivity 与 ablation 三项判据均通过。因此可以确认：personality state 不只被写入 feature vector，而是被 14D 模型实际用于 mood selection。

ESP32 仍然只运行 inference，不进行 on-device training。新模型的兼容头文件及替换步骤见 `README_training_v2.md`。
