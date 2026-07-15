"""Compare full and weather-only models on ordinary and controlled samples."""

from __future__ import annotations

import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.metrics import accuracy_score

from common_v2 import (
    BASELINE_PERSONALITY,
    BASE_DIR,
    DATA_FILE,
    FULL_FEATURES,
    FULL_MODEL_FILE,
    PERSONALITY_PROFILES,
    PROFILE_TO_MOOD,
    SCENARIOS,
    WEATHER_FEATURES,
    WEATHER_MODEL_FILE,
    split_indices,
)

REPORT_FILE = BASE_DIR / "ablation_report_v2.md"


def accuracy(model, values, labels):
    predicted = np.argmax(model.predict(values, verbose=0), axis=1)
    return float(accuracy_score(labels, predicted))


def sensitivity_score(model, include_personality):
    deltas = []
    switches = 0
    for weather in SCENARIOS.values():
        if include_personality:
            baseline_input = weather + BASELINE_PERSONALITY
        else:
            baseline_input = weather
        baseline = model.predict(np.asarray([baseline_input], dtype=np.float32), verbose=0)[0]
        predictions = []
        for profile, personality in PERSONALITY_PROFILES.items():
            values = weather + personality if include_personality else weather
            probabilities = model.predict(np.asarray([values], dtype=np.float32), verbose=0)[0]
            target = PROFILE_TO_MOOD[profile]
            deltas.append(float(probabilities[target] - baseline[target]))
            predictions.append(int(np.argmax(probabilities)))
        switches += int(len(set(predictions)) > 1)
    return float(np.mean(deltas)), switches


def main():
    missing = [path.name for path in (DATA_FILE, FULL_MODEL_FILE, WEATHER_MODEL_FILE) if not path.exists()]
    if missing:
        raise FileNotFoundError(f"Missing {', '.join(missing)}; run data generation and training first")
    frame = pd.read_csv(DATA_FILE)
    labels = frame["label"].to_numpy(dtype=np.int64)
    _, test_idx = split_indices(labels)
    test = frame.iloc[test_idx]
    controlled = frame[frame["sample_type"] == "controlled_contrast"]

    full = tf.keras.models.load_model(FULL_MODEL_FILE)
    weather = tf.keras.models.load_model(WEATHER_MODEL_FILE)
    full_test = accuracy(full, test[FULL_FEATURES].to_numpy(np.float32), test["label"])
    weather_test = accuracy(weather, test[WEATHER_FEATURES].to_numpy(np.float32), test["label"])
    full_controlled = accuracy(full, controlled[FULL_FEATURES].to_numpy(np.float32), controlled["label"])
    weather_controlled = accuracy(weather, controlled[WEATHER_FEATURES].to_numpy(np.float32), controlled["label"])
    full_sensitivity, full_switches = sensitivity_score(full, True)
    weather_sensitivity, weather_switches = sensitivity_score(weather, False)

    affects = full_sensitivity >= 0.05
    visible = full_switches >= 1
    outperforms = full_controlled > weather_controlled + 0.10
    overall = affects and visible and outperforms
    final_conclusion = (
        "Sensitivity 与 ablation 三项判据均通过。因此可以确认：personality state 不只被写入 feature vector，而是被 14D 模型实际用于 mood selection。"
        if overall
        else "当前至少一项判据未通过，尚不能确认 personality 被模型稳定用于 mood selection。"
    )
    REPORT_FILE.write_text(
        f"""# Ablation comparison v2

比较完全相同的数据划分与网络宽度；唯一关键差异是 full model 读取全部 14 维，而 weather-only model 只能读取前 10 维。

| metric | 14D full | 10D weather-only | difference |
|---|---:|---:|---:|
| 普通 test set accuracy | {full_test:.4f} | {weather_test:.4f} | {full_test - weather_test:+.4f} |
| controlled contrast accuracy | {full_controlled:.4f} | {weather_controlled:.4f} | {full_controlled - weather_controlled:+.4f} |
| personality sensitivity score | {full_sensitivity:.4f} | {weather_sensitivity:.4f} | {full_sensitivity - weather_sensitivity:+.4f} |
| 随 personality 改变预测类别的场景数（共 5） | {full_switches} | {weather_switches} | {full_switches - weather_switches:+d} |

Weather-only model 对同一组固定 weather 的四种 personality 收到的是完全相同的 10D 输入，因此 sensitivity score 理论上为 0，也无法按 personality 改变输出。Controlled contrast accuracy 直接衡量这一信息缺失。

## 验收问题

- **Does personality input affect mood selection?** {'是' if affects else '当前证据不足'}。14D 模型的平均目标概率增量为 {full_sensitivity:.4f}。
- **Is the effect visible in controlled sensitivity tests?** {'是' if visible else '否'}。14D 模型在 {full_switches}/5 个固定天气场景中随 personality 改变最终类别。
- **Does the 14D full model outperform the 10D weather-only model on personality-dependent samples?** {'是' if outperforms else '否'}。Controlled contrast accuracy 差值为 {full_controlled - weather_controlled:+.4f}。

## 结论

当前 v2 训练数据明确把 personality 纳入 label 生成逻辑。Sensitivity test 检查固定天气下各 personality 对对应 mood 概率的定向作用；ablation 则通过移除后四维验证性能变化。

{final_conclusion}

ESP32 仍然只运行 inference，不进行 on-device training。新模型的兼容头文件及替换步骤见 `README_training_v2.md`。
""",
        encoding="utf-8",
    )
    print(f"Test accuracy: full={full_test:.4f}, weather-only={weather_test:.4f}")
    print(f"Controlled accuracy: full={full_controlled:.4f}, weather-only={weather_controlled:.4f}")
    print(f"Sensitivity: full={full_sensitivity:.4f}, weather-only={weather_sensitivity:.4f}")
    print(f"Saved {REPORT_FILE.name}")


if __name__ == "__main__":
    main()
