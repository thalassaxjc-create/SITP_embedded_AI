"""Hold weather fixed and measure the full model's response to personality."""

from __future__ import annotations

import csv

import numpy as np
import tensorflow as tf

from common_v2 import (
    BASELINE_PERSONALITY,
    BASE_DIR,
    FULL_MODEL_FILE,
    MOOD_NAMES,
    PERSONALITY_PROFILES,
    PROFILE_TO_MOOD,
    SCENARIOS,
)

CSV_FILE = BASE_DIR / "sensitivity_results_v2.csv"
REPORT_FILE = BASE_DIR / "sensitivity_report_v2.md"
MIN_PROBABILITY_INCREASE = 0.05


def main():
    if not FULL_MODEL_FILE.exists():
        raise FileNotFoundError("请先运行 python train_selector_v2.py")
    model = tf.keras.models.load_model(FULL_MODEL_FILE)
    rows = []
    scenario_changed = {}
    deltas = []

    for scenario_name, weather in SCENARIOS.items():
        baseline = model.predict(np.asarray([weather + BASELINE_PERSONALITY], dtype=np.float32), verbose=0)[0]
        predictions = []
        for profile_name, personality in PERSONALITY_PROFILES.items():
            probabilities = model.predict(np.asarray([weather + personality], dtype=np.float32), verbose=0)[0]
            predicted = int(np.argmax(probabilities))
            target = PROFILE_TO_MOOD[profile_name]
            delta = float(probabilities[target] - baseline[target])
            passed = delta >= MIN_PROBABILITY_INCREASE
            predictions.append(predicted)
            deltas.append(delta)
            rows.append(
                {
                    "scenario": scenario_name,
                    "personality_profile": profile_name,
                    "quiet_prob": float(probabilities[0]),
                    "warm_prob": float(probabilities[1]),
                    "strange_prob": float(probabilities[2]),
                    "lonely_prob": float(probabilities[3]),
                    "predicted_mood": MOOD_NAMES[predicted],
                    "expected_personality_effect": f"{MOOD_NAMES[target]}_prob +{delta:.4f} vs baseline",
                    "pass/fail": "pass" if passed else "fail",
                }
            )
        scenario_changed[scenario_name] = len(set(predictions)) > 1

    fields = list(rows[0])
    with CSV_FILE.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)

    passed = sum(row["pass/fail"] == "pass" for row in rows)
    changed_count = sum(scenario_changed.values())
    neutral_changed = scenario_changed["neutral_weather"]
    table = [
        "| scenario | profile | quiet | warm | strange | lonely | predicted | delta | result |",
        "|---|---|---:|---:|---:|---:|---|---:|---|",
    ]
    for row in rows:
        delta = row["expected_personality_effect"].split("+")[1].split()[0]
        table.append(
            f"| {row['scenario']} | {row['personality_profile']} | {row['quiet_prob']:.4f} | "
            f"{row['warm_prob']:.4f} | {row['strange_prob']:.4f} | {row['lonely_prob']:.4f} | "
            f"{row['predicted_mood']} | {delta} | {row['pass/fail']} |"
        )
    conclusion = "通过" if passed == len(rows) and neutral_changed else "未完全通过"
    table_text = "\n".join(table)
    REPORT_FILE.write_text(
        f"""# Personality sensitivity test v2

测试固定五组 weather，只把 baseline personality `[0.20, 0.20, 0.20, 0.20]` 中的一个维度提高到 `0.90`。判定目标类别概率相对 baseline 至少增加 `{MIN_PROBABILITY_INCREASE:.2f}`。

- 概率提升通过：{passed}/{len(rows)}
- personality 能改变最终 predicted mood 的场景：{changed_count}/{len(SCENARIOS)}
- neutral weather 是否随 personality 改变 predicted mood：{'是' if neutral_changed else '否'}
- 平均目标概率增量（personality sensitivity score）：{np.mean(deltas):.4f}
- 综合结论：**{conclusion}**

{table_text}

## 结论

固定天气时，分别提高 calm/weird/warm/lonely 后，目标 quiet/strange/warm/lonely 概率的变化如表所示。概率增量及 neutral/ambiguous weather 的类别切换用于判断 personality 是否被模型实际使用，而不只检查 feature vector 是否含有这些字段。
""",
        encoding="utf-8",
    )
    print(f"Sensitivity pass: {passed}/{len(rows)}; changed scenarios: {changed_count}/{len(SCENARIOS)}")
    print(f"Saved {CSV_FILE.name} and {REPORT_FILE.name}")


if __name__ == "__main__":
    main()
