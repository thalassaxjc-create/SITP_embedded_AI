"""Generate score-labelled data in which weather and personality both matter."""

from __future__ import annotations

import csv
import random
from collections import Counter

from common_v2 import (
    DATA_FILE,
    FULL_FEATURES,
    MOOD_NAMES,
    PERSONALITY_PROFILES,
    PROFILE_TO_MOOD,
    SCENARIOS,
    SEED,
    score_moods,
)

RANDOM_SAMPLES_PER_CLASS = 1500
CONTRAST_REPEATS = 80


def clipped(value):
    return max(0.0, min(1.0, value))


def make_random_weather(rng):
    temp = rng.random()
    humidity = rng.random()
    rain = rng.random() ** 2.2
    wind = rng.random() ** 1.5
    is_day = float(rng.random() < 0.58)
    return [
        temp,
        humidity,
        rain,
        wind,
        is_day,
        float(temp > 0.75),
        float(humidity > 0.75),
        float(rain > 0.20),
        float((rain > 0.08) or (humidity > 0.68 and rng.random() < 0.75)),
        float(rain > 0.65 and wind > 0.45),
    ]


def make_random_personality(rng):
    # Half the samples emphasize one trait; the rest cover the continuous space.
    values = [rng.uniform(0.05, 0.75) for _ in range(4)]
    if rng.random() < 0.50:
        values[rng.randrange(4)] = rng.uniform(0.72, 1.0)
    return values


def label_for(features, rng):
    scores = score_moods(features, noise=lambda: rng.gauss(0.0, 0.012))
    return max(range(4), key=scores.__getitem__)


def random_balanced_rows(rng):
    rows = []
    counts = Counter()
    attempts = 0
    while min((counts[i] for i in range(4)), default=0) < RANDOM_SAMPLES_PER_CLASS:
        attempts += 1
        if attempts > 500_000:
            raise RuntimeError("Unable to balance generated labels")
        features = make_random_weather(rng) + make_random_personality(rng)
        label = label_for(features, rng)
        if counts[label] >= RANDOM_SAMPLES_PER_CLASS:
            continue
        counts[label] += 1
        rows.append(features + [label, MOOD_NAMES[label], "random", "", "", ""])
    return rows


def controlled_contrast_rows(rng):
    rows = []
    for scenario_name, base_weather in SCENARIOS.items():
        for repeat in range(CONTRAST_REPEATS):
            # All four profiles in a contrast group share exactly the same weather.
            weather = list(base_weather)
            for i in range(4):
                if i < 4:
                    weather[i] = clipped(weather[i] + rng.gauss(0.0, 0.025))
            group = f"{scenario_name}_{repeat:03d}"
            for profile_name, base_personality in PERSONALITY_PROFILES.items():
                personality = [clipped(v + rng.gauss(0.0, 0.012)) for v in base_personality]
                features = weather + personality
                label = label_for(features, rng)
                rows.append(
                    features
                    + [
                        label,
                        MOOD_NAMES[label],
                        "controlled_contrast",
                        scenario_name,
                        group,
                        profile_name,
                    ]
                )
    return rows


def main():
    rng = random.Random(SEED)
    rows = random_balanced_rows(rng) + controlled_contrast_rows(rng)
    rng.shuffle(rows)
    header = FULL_FEATURES + [
        "label",
        "label_name",
        "sample_type",
        "scenario",
        "contrast_group",
        "personality_profile",
    ]
    with DATA_FILE.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(header)
        for row in rows:
            writer.writerow([round(v, 6) if isinstance(v, float) else v for v in row])

    counts = Counter(row[14] for row in rows)
    controlled = sum(row[16] == "controlled_contrast" for row in rows)
    print(f"Generated {len(rows)} samples -> {DATA_FILE.name}")
    print("Label distribution:", {MOOD_NAMES[i]: counts[i] for i in range(4)})
    print(f"Controlled contrast samples: {controlled}")


if __name__ == "__main__":
    main()

