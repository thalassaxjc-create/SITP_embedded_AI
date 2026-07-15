#!/usr/bin/env python3
"""Preview and sanity-check the K10 weather-letter corpus.

The Arduino firmware is the source of truth. This tool parses the string arrays
from StoryVocab.cpp so sentence changes can be checked without maintaining a
second corpus by hand.
"""

from __future__ import annotations

import random
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VOCAB_CPP = ROOT / "WeatherStory_TinySelector_K10" / "StoryVocab.cpp"
SAMPLE_COUNT_PER_MOOD = 200
VISIBLE_SAMPLES_PER_MOOD = 10
MAX_LETTER_CHARS = 260

MOODS = ("quiet", "warm", "strange", "lonely")
PARTS = ("Openings", "Observations", "Connections", "Endings")

FORBIDDEN_PHRASES = (
    "two sides are one family",
    "blood is thicker than water",
    "friendship bridge",
    "common vision",
    "youth power",
    "create a better future",
    "new era youth",
    "deepen exchanges",
    "technology empowers",
    "integration development",
)

SCENARIOS = (
    {
        "FROM_CITY": "Shanghai",
        "TO_CITY": "Taichung",
        "FROM_TEMP": "24.2",
        "TO_TEMP": "28.4",
        "FROM_WEATHER": "rainy",
        "TO_WEATHER": "cloudy",
        "TEMP_DIFF": "4.2",
        "TIME_OF_DAY": "Tonight",
    },
    {
        "FROM_CITY": "Shanghai",
        "TO_CITY": "Taichung",
        "FROM_TEMP": "33.1",
        "TO_TEMP": "29.0",
        "FROM_WEATHER": "hot",
        "TO_WEATHER": "clear",
        "TEMP_DIFF": "4.1",
        "TIME_OF_DAY": "Today",
    },
    {
        "FROM_CITY": "Shanghai",
        "TO_CITY": "Taichung",
        "FROM_TEMP": "18.0",
        "TO_TEMP": "25.6",
        "FROM_WEATHER": "windy",
        "TO_WEATHER": "warm",
        "TEMP_DIFF": "7.6",
        "TIME_OF_DAY": "Today",
    },
    {
        "FROM_CITY": "Shanghai",
        "TO_CITY": "Taichung",
        "FROM_TEMP": "12.5",
        "TO_TEMP": "13.4",
        "FROM_WEATHER": "cloudy",
        "TO_WEATHER": "cloudy",
        "TEMP_DIFF": "0.9",
        "TIME_OF_DAY": "Tonight",
    },
)

WEATHER_DETAILS = {
    "rainy": (
        "The rain reached the windows before I did.",
        "My shoes are making a small wet sound.",
        "Someone forgot an umbrella beside the door.",
        "The bicycle seats outside are not trustworthy.",
    ),
    "windy": (
        "The wind keeps testing the classroom door.",
        "Tree leaves are moving faster than the people.",
        "A sign near the street has started complaining.",
        "Riding a bike today would require negotiation.",
    ),
    "hot": (
        "The fan is doing its best and losing.",
        "A cold drink sounds like a serious plan.",
        "The shade outside the classroom is crowded.",
        "I have decided not to move unless necessary.",
    ),
    "cloudy": (
        "The sky is bright but undecided.",
        "The windows have lost their shadows.",
        "The afternoon looks paused.",
        "Clouds are making the campus quieter.",
    ),
    "clear": (
        "Sunlight has reached the table.",
        "The shadows outside are unusually confident.",
        "The weather is behaving better than my schedule.",
        "People have started walking as if they planned it.",
    ),
}


def parse_arrays() -> dict[str, list[str]]:
    text = VOCAB_CPP.read_text(encoding="utf-8")
    arrays: dict[str, list[str]] = {}
    pattern = re.compile(
        r"const char\* const (\w+)\[\] = \{(.*?)\};",
        re.DOTALL,
    )
    for name, body in pattern.findall(text):
        arrays[name] = re.findall(r'"((?:\\.|[^"])*)"', body)
    return arrays


def apply_context(line: str, scenario: dict[str, str]) -> str:
    for key, value in scenario.items():
        line = line.replace("{" + key + "}", value)
    return line


def pick(arrays: dict[str, list[str]], mood: str, part: str) -> str:
    return random.choice(arrays[f"{mood}{part}"])


def generate(arrays: dict[str, list[str]], mood: str, template: int) -> str:
    scenario = random.choice(SCENARIOS)
    lines = [
        apply_context(pick(arrays, mood, "Openings"), scenario),
    ]
    detail = random.choice(WEATHER_DETAILS.get(
        scenario["FROM_WEATHER"],
        WEATHER_DETAILS["clear"],
    ))

    if template == 0:
        lines.append(apply_context(pick(arrays, mood, "Observations"), scenario))
        lines.append(apply_context(pick(arrays, mood, "Connections"), scenario))
    elif template == 1:
        lines.append(detail)
        lines.append(apply_context(pick(arrays, mood, "Connections"), scenario))
    elif template == 2:
        lines.append(detail)
        lines.append(apply_context(pick(arrays, mood, "Observations"), scenario))
        lines.append(apply_context(pick(arrays, mood, "Connections"), scenario))
    else:
        lines.append(apply_context(pick(arrays, mood, "Observations"), scenario))
        lines.append(apply_context(pick(arrays, mood, "Connections"), scenario))

    lines.append(apply_context(pick(arrays, mood, "Endings"), scenario))
    return "\n".join(lines)


def check_story(story: str, errors: list[str]) -> None:
    if not story.strip():
        errors.append("empty story")
    if re.search(r"\{[A-Z_]+\}", story):
        errors.append(f"unreplaced placeholder: {story}")
    if re.search(r"\b(a|an)\s+(a|an)\b", story, re.IGNORECASE):
        errors.append(f"duplicate article: {story}")
    lowered = story.lower()
    for phrase in FORBIDDEN_PHRASES:
        if phrase in lowered:
            errors.append(f"forbidden phrase '{phrase}': {story}")
    if len(story) > MAX_LETTER_CHARS:
        errors.append(f"long story ({len(story)} chars): {story}")
    lines = story.splitlines()
    for left, right in zip(lines, lines[1:]):
        if left.strip() and left.strip() == right.strip():
            errors.append(f"repeated adjacent sentence: {story}")


def main() -> int:
    random.seed(20260715)
    arrays = parse_arrays()
    errors: list[str] = []

    for mood in MOODS:
        for part in PARTS:
            key = f"{mood}{part}"
            if key not in arrays:
                errors.append(f"missing array: {key}")
            elif not all(item.strip() for item in arrays[key]):
                errors.append(f"empty string in array: {key}")

    for mood in MOODS:
        samples = []
        for i in range(SAMPLE_COUNT_PER_MOOD):
            story = generate(arrays, mood, i % 4)
            check_story(story, errors)
            if i < VISIBLE_SAMPLES_PER_MOOD:
                samples.append(story)

        print(f"\n=== {mood.upper()} samples ===")
        for index, sample in enumerate(samples, 1):
            print(f"\n[{index}]")
            print(sample)

    if errors:
        print("\n=== CHECK FAILED ===")
        for error in errors[:80]:
            print(f"- {error}")
        if len(errors) > 80:
            print(f"- ... {len(errors) - 80} more")
        return 1

    print("\nAll corpus checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
