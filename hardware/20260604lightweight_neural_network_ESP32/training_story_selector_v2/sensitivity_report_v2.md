# Personality sensitivity test v2

测试固定五组 weather，只把 baseline personality `[0.20, 0.20, 0.20, 0.20]` 中的一个维度提高到 `0.90`。判定目标类别概率相对 baseline 至少增加 `0.05`。

- 概率提升通过：20/20
- personality 能改变最终 predicted mood 的场景：5/5
- neutral weather 是否随 personality 改变 predicted mood：是
- 平均目标概率增量（personality sensitivity score）：0.3229
- 综合结论：**通过**

| scenario | profile | quiet | warm | strange | lonely | predicted | delta | result |
|---|---|---:|---:|---:|---:|---|---:|---|
| neutral_weather | calm_high | 0.5465 | 0.3383 | 0.0535 | 0.0617 | quiet | 0.4012 | pass |
| neutral_weather | weird_high | 0.0898 | 0.3945 | 0.4444 | 0.0713 | strange | 0.3432 | pass |
| neutral_weather | warm_high | 0.0297 | 0.9261 | 0.0207 | 0.0235 | warm | 0.2880 | pass |
| neutral_weather | lonely_high | 0.0853 | 0.3745 | 0.0594 | 0.4807 | lonely | 0.3654 | pass |
| humid_night | calm_high | 0.5807 | 0.0370 | 0.0190 | 0.3632 | quiet | 0.4175 | pass |
| humid_night | weird_high | 0.1324 | 0.0599 | 0.2190 | 0.5887 | lonely | 0.1810 | pass |
| humid_night | warm_high | 0.1125 | 0.3617 | 0.0262 | 0.4997 | lonely | 0.2878 | pass |
| humid_night | lonely_high | 0.0301 | 0.0136 | 0.0070 | 0.9493 | lonely | 0.2243 | pass |
| clear_day | calm_high | 0.5474 | 0.3422 | 0.0505 | 0.0600 | quiet | 0.4016 | pass |
| clear_day | weird_high | 0.0921 | 0.4082 | 0.4289 | 0.0709 | strange | 0.3332 | pass |
| clear_day | warm_high | 0.0295 | 0.9285 | 0.0193 | 0.0227 | warm | 0.2822 | pass |
| clear_day | lonely_high | 0.0866 | 0.3836 | 0.0568 | 0.4730 | lonely | 0.3608 | pass |
| rainy_room | calm_high | 0.8515 | 0.0103 | 0.0074 | 0.1308 | quiet | 0.4048 | pass |
| rainy_room | weird_high | 0.3822 | 0.0329 | 0.1676 | 0.4173 | lonely | 0.1400 | pass |
| rainy_room | warm_high | 0.3618 | 0.2211 | 0.0223 | 0.3947 | lonely | 0.1827 | pass |
| rainy_room | lonely_high | 0.1124 | 0.0097 | 0.0069 | 0.8710 | lonely | 0.3837 | pass |
| stormy_weather | calm_high | 0.7245 | 0.0098 | 0.1087 | 0.1570 | quiet | 0.4543 | pass |
| stormy_weather | weird_high | 0.0980 | 0.0094 | 0.7416 | 0.1510 | strange | 0.4537 | pass |
| stormy_weather | warm_high | 0.2333 | 0.1590 | 0.2485 | 0.3592 | lonely | 0.1331 | pass |
| stormy_weather | lonely_high | 0.0764 | 0.0073 | 0.0813 | 0.8350 | lonely | 0.4190 | pass |

## 结论

固定天气时，分别提高 calm/weird/warm/lonely 后，目标 quiet/strange/warm/lonely 概率的变化如表所示。概率增量及 neutral/ambiguous weather 的类别切换用于判断 personality 是否被模型实际使用，而不只检查 feature vector 是否含有这些字段。
