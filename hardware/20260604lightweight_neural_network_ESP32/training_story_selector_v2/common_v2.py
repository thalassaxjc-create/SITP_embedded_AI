"""Weather Story selector v2 shared definitions and helpers."""

from __future__ import annotations

import os
import random
from pathlib import Path

SEED = 20260604
BASE_DIR = Path(__file__).resolve().parent
DATA_FILE = BASE_DIR / "story_training_data_v2.csv"
FULL_MODEL_FILE = BASE_DIR / "story_mood_model_v2.keras"
WEATHER_MODEL_FILE = BASE_DIR / "story_mood_model_weather_only.keras"

WEATHER_FEATURES = [
    "temp_norm",
    "humidity_norm",
    "rain_norm",
    "wind_norm",
    "is_day",
    "is_hot",
    "is_humid",
    "is_rainy",
    "is_cloudy",
    "is_stormy",
]
PERSONALITY_FEATURES = [
    "personality_calm",
    "personality_weird",
    "personality_warm",
    "personality_lonely",
]
FULL_FEATURES = WEATHER_FEATURES + PERSONALITY_FEATURES
MOOD_NAMES = ["quiet", "warm", "strange", "lonely"]
PROFILE_TO_MOOD = {
    "calm_high": 0,
    "warm_high": 1,
    "weird_high": 2,
    "lonely_high": 3,
}
PERSONALITY_PROFILES = {
    "calm_high": [0.90, 0.20, 0.20, 0.20],
    "weird_high": [0.20, 0.90, 0.20, 0.20],
    "warm_high": [0.20, 0.20, 0.90, 0.20],
    "lonely_high": [0.20, 0.20, 0.20, 0.90],
}
BASELINE_PERSONALITY = [0.20, 0.20, 0.20, 0.20]

# Values follow the firmware's normalized feature order exactly.
SCENARIOS = {
    "neutral_weather": [0.50, 0.25, 0.05, 0.20, 1, 0, 0, 0, 0, 0],
    "humid_night": [0.48, 0.82, 0.08, 0.12, 0, 0, 1, 0, 1, 0],
    "clear_day": [0.72, 0.25, 0.00, 0.15, 1, 0, 0, 0, 0, 0],
    "rainy_room": [0.42, 0.78, 0.45, 0.18, 0, 0, 1, 1, 1, 0],
    "stormy_weather": [0.45, 0.80, 0.72, 0.72, 0, 0, 1, 1, 1, 1],
}


def set_reproducible_seed(tf=None) -> None:
    os.environ.setdefault("PYTHONHASHSEED", str(SEED))
    random.seed(SEED)
    if tf is not None:
        tf.keras.utils.set_random_seed(SEED)
        try:
            tf.config.experimental.enable_op_determinism()
        except Exception:
            pass


def score_moods(features, noise=None):
    """Return [quiet, warm, strange, lonely] scores for one 14D row."""
    _, humidity, rain, wind, is_day, is_hot, is_humid, is_rainy, is_cloudy, is_stormy, calm, weird, warm, lonely = features
    scores = [
        0.35 * is_rainy + 0.25 * is_humid + 0.15 * (1.0 - wind) + 0.70 * calm,
        0.30 * is_day + 0.25 * is_hot + 0.20 * (1.0 - rain) + 0.70 * warm,
        0.55 * is_stormy + 0.15 * wind + 0.70 * weird,
        0.35 * (1.0 - is_day) + 0.25 * humidity + 0.20 * is_cloudy + 0.70 * lonely,
    ]
    if noise is not None:
        scores = [score + noise() for score in scores]
    return scores


def split_indices(labels, test_size=0.20):
    from sklearn.model_selection import train_test_split

    indices = list(range(len(labels)))
    return train_test_split(
        indices, test_size=test_size, random_state=SEED, stratify=labels
    )


def predict_probabilities(model, values):
    return model.predict(values, verbose=0)

