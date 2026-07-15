"""Train and report the 14D full model and 10D weather-only ablation model."""

from __future__ import annotations

import json

import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.metrics import classification_report, confusion_matrix

from common_v2 import (
    DATA_FILE,
    FULL_FEATURES,
    FULL_MODEL_FILE,
    MOOD_NAMES,
    SEED,
    WEATHER_FEATURES,
    WEATHER_MODEL_FILE,
    score_moods,
    set_reproducible_seed,
    split_indices,
)

REPORT_FILE = DATA_FILE.parent / "training_report_v2.md"
METRICS_FILE = DATA_FILE.parent / "training_metrics_v2.json"


def build_model(input_size):
    model = tf.keras.Sequential(
        [
            tf.keras.layers.Input(shape=(input_size,)),
            tf.keras.layers.Dense(16, activation="relu"),
            tf.keras.layers.Dense(8, activation="relu"),
            tf.keras.layers.Dense(4, activation="softmax"),
        ]
    )
    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.004),
        loss="categorical_crossentropy",
        metrics=["categorical_accuracy"],
    )
    return model


def make_soft_targets(frame, temperature=0.25):
    """Preserve argmax labels while teaching output probabilities to track scores."""
    scores = np.asarray(
        [score_moods(row) for row in frame[FULL_FEATURES].to_numpy(dtype=np.float32)],
        dtype=np.float32,
    )
    scaled = scores / temperature
    scaled -= scaled.max(axis=1, keepdims=True)
    exponentials = np.exp(scaled)
    return exponentials / exponentials.sum(axis=1, keepdims=True)


def train_one(name, feature_columns, model_file, frame, soft_targets, train_idx, test_idx):
    x = frame[feature_columns].to_numpy(dtype=np.float32)
    y = frame["label"].to_numpy(dtype=np.int64)
    model = build_model(len(feature_columns))
    callbacks = [
        tf.keras.callbacks.EarlyStopping(
            monitor="val_loss", patience=15, restore_best_weights=True
        )
    ]
    history = model.fit(
        x[train_idx],
        soft_targets[train_idx],
        validation_split=0.15,
        epochs=160,
        batch_size=64,
        callbacks=callbacks,
        verbose=0,
    )
    train_prob = model.predict(x[train_idx], verbose=0)
    test_prob = model.predict(x[test_idx], verbose=0)
    train_loss = float(tf.keras.losses.categorical_crossentropy(soft_targets[train_idx], train_prob).numpy().mean())
    test_loss = float(tf.keras.losses.categorical_crossentropy(soft_targets[test_idx], test_prob).numpy().mean())
    train_acc = float(np.mean(np.argmax(train_prob, axis=1) == y[train_idx]))
    test_acc = float(np.mean(np.argmax(test_prob, axis=1) == y[test_idx]))
    predicted = np.argmax(test_prob, axis=1)
    matrix = confusion_matrix(y[test_idx], predicted, labels=range(4))
    report = classification_report(
        y[test_idx],
        predicted,
        labels=range(4),
        target_names=MOOD_NAMES,
        digits=4,
        zero_division=0,
    )
    model.save(model_file)
    result = {
        "name": name,
        "input_size": len(feature_columns),
        "epochs": len(history.history["loss"]),
        "train_loss": float(train_loss),
        "train_accuracy": float(train_acc),
        "test_loss": float(test_loss),
        "test_accuracy": float(test_acc),
        "confusion_matrix": matrix.tolist(),
        "classification_report": report,
    }
    print(f"{name}: train={train_acc:.4f}, test={test_acc:.4f}, epochs={result['epochs']}")
    print(matrix)
    print(report)
    return result


def matrix_markdown(matrix):
    lines = ["| actual \\ predicted | quiet | warm | strange | lonely |", "|---|---:|---:|---:|---:|"]
    for name, row in zip(MOOD_NAMES, matrix):
        lines.append(f"| {name} | " + " | ".join(str(value) for value in row) + " |")
    return "\n".join(lines)


def main():
    if not DATA_FILE.exists():
        raise FileNotFoundError("请先运行 python generate_training_data_v2.py")
    set_reproducible_seed(tf)
    frame = pd.read_csv(DATA_FILE)
    labels = frame["label"].to_numpy(dtype=np.int64)
    train_idx, test_idx = split_indices(labels)
    soft_targets = make_soft_targets(frame)
    distribution = frame.groupby(["label", "label_name"]).size().to_dict()
    print("Label distribution:")
    for mood_id, mood_name in enumerate(MOOD_NAMES):
        print(f"  {mood_id} {mood_name}: {int(distribution.get((mood_id, mood_name), 0))}")

    full = train_one("14D full personality model", FULL_FEATURES, FULL_MODEL_FILE, frame, soft_targets, train_idx, test_idx)
    weather = train_one("10D weather-only model", WEATHER_FEATURES, WEATHER_MODEL_FILE, frame, soft_targets, train_idx, test_idx)
    metrics = {
        "seed": SEED,
        "rows": len(frame),
        "train_rows": len(train_idx),
        "test_rows": len(test_idx),
        "label_distribution": {name: int(distribution.get((i, name), 0)) for i, name in enumerate(MOOD_NAMES)},
        "full": full,
        "weather_only": weather,
    }
    METRICS_FILE.write_text(json.dumps(metrics, ensure_ascii=False, indent=2), encoding="utf-8")

    sections = []
    for result in (full, weather):
        sections.append(
            f"""## {result['name']}

- 结构：`Input({result['input_size']}) -> Dense(16, relu) -> Dense(8, relu) -> Dense(4, softmax)`
- 实际训练轮数：{result['epochs']}
- train accuracy：{result['train_accuracy']:.4f}
- test accuracy：{result['test_accuracy']:.4f}
- test loss：{result['test_loss']:.4f}

### Confusion matrix

{matrix_markdown(result['confusion_matrix'])}

### Classification report

```text
{result['classification_report'].rstrip()}
```
"""
        )
    distribution_lines = "\n".join(f"- `{name}`：{metrics['label_distribution'][name]}" for name in MOOD_NAMES)
    REPORT_FILE.write_text(
        f"""# v2 模型训练报告

## 数据与标签

共 {len(frame)} 条样本，训练集 {len(train_idx)} 条、测试集 {len(test_idx)} 条，固定随机种子 `{SEED}`。标签不是仅由天气决定，而是按四个可解释 score 联合使用天气特征和 `personality_calm/weird/warm/lonely`；每个对应 personality 的权重为 `0.70`，并只加入标准差约 `0.012` 的轻微 score jitter。CSV 的离散 label 始终是最高 score 的 mood；训练时另把四个 score 用 temperature `0.25` 转成 soft targets，使输出概率保留可解释的相对强度，避免 one-hot 训练造成概率饱和。

Label distribution：

{distribution_lines}

数据还包含同一天气配四种 personality 的 `controlled_contrast` 对照组，用于直接检验 personality-dependent cases。

{''.join(sections)}
## 运行端说明

ESP32 仍只运行 TFLite inference，不进行 on-device training。模型替换与数组命名见 `README_training_v2.md`。
""",
        encoding="utf-8",
    )
    print(f"Saved report -> {REPORT_FILE.name}")


if __name__ == "__main__":
    main()
