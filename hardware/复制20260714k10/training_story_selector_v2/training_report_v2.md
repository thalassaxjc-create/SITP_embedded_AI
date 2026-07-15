# v2 模型训练报告

## 数据与标签

共 7600 条样本，训练集 6080 条、测试集 1520 条，固定随机种子 `20260604`。标签不是仅由天气决定，而是按四个可解释 score 联合使用天气特征和 `personality_calm/weird/warm/lonely`；每个对应 personality 的权重为 `0.70`，并只加入标准差约 `0.012` 的轻微 score jitter。CSV 的离散 label 始终是最高 score 的 mood；训练时另把四个 score 用 temperature `0.25` 转成 soft targets，使输出概率保留可解释的相对强度，避免 one-hot 训练造成概率饱和。

Label distribution：

- `quiet`：1921
- `warm`：1696
- `strange`：1705
- `lonely`：2278

数据还包含同一天气配四种 personality 的 `controlled_contrast` 对照组，用于直接检验 personality-dependent cases。

## 14D full personality model

- 结构：`Input(14) -> Dense(16, relu) -> Dense(8, relu) -> Dense(4, softmax)`
- 实际训练轮数：59
- train accuracy：0.9743
- test accuracy：0.9697
- test loss：0.9969

### Confusion matrix

| actual \ predicted | quiet | warm | strange | lonely |
|---|---:|---:|---:|---:|
| quiet | 367 | 5 | 1 | 11 |
| warm | 2 | 329 | 8 | 0 |
| strange | 0 | 4 | 330 | 7 |
| lonely | 4 | 2 | 2 | 448 |

### Classification report

```text
              precision    recall  f1-score   support

       quiet     0.9839    0.9557    0.9696       384
        warm     0.9676    0.9705    0.9691       339
     strange     0.9677    0.9677    0.9677       341
      lonely     0.9614    0.9825    0.9718       456

    accuracy                         0.9697      1520
   macro avg     0.9702    0.9691    0.9696      1520
weighted avg     0.9699    0.9697    0.9697      1520
```
## 10D weather-only model

- 结构：`Input(10) -> Dense(16, relu) -> Dense(8, relu) -> Dense(4, softmax)`
- 实际训练轮数：67
- train accuracy：0.6377
- test accuracy：0.6559
- test loss：1.1664

### Confusion matrix

| actual \ predicted | quiet | warm | strange | lonely |
|---|---:|---:|---:|---:|
| quiet | 146 | 88 | 11 | 139 |
| warm | 29 | 284 | 8 | 18 |
| strange | 11 | 62 | 215 | 53 |
| lonely | 20 | 70 | 14 | 352 |

### Classification report

```text
              precision    recall  f1-score   support

       quiet     0.7087    0.3802    0.4949       384
        warm     0.5635    0.8378    0.6738       339
     strange     0.8669    0.6305    0.7301       341
      lonely     0.6263    0.7719    0.6916       456

    accuracy                         0.6559      1520
   macro avg     0.6914    0.6551    0.6476      1520
weighted avg     0.6871    0.6559    0.6465      1520
```

## 运行端说明

ESP32 仍只运行 TFLite inference，不进行 on-device training。模型替换与数组命名见 `README_training_v2.md`。
