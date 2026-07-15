"""Export the validated full TFLite model under new and firmware-compatible names."""

from pathlib import Path

from common_v2 import BASE_DIR

TFLITE_FILE = BASE_DIR / "story_mood_model_v2.tflite"


def write_header(path, array_name, guard):
    data = TFLITE_FILE.read_bytes()
    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <Arduino.h>",
        "",
        f"alignas(8) const unsigned char {array_name}[] = {{",
    ]
    for offset in range(0, len(data), 12):
        chunk = ", ".join(f"0x{value:02x}" for value in data[offset : offset + 12])
        suffix = "," if offset + 12 < len(data) else ""
        lines.append(f"  {chunk}{suffix}")
    lines.extend(
        [
            "};",
            "",
            f"const unsigned int {array_name}_len = {len(data)};",
            "",
            f"#endif  // {guard}",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")
    print(f"Exported {path.name}: {array_name}, {len(data)} bytes")


def main():
    if not TFLITE_FILE.exists():
        raise FileNotFoundError("Missing story_mood_model_v2.tflite; run convert_to_tflite_v2.py first")
    write_header(BASE_DIR / "model_v2.h", "story_mood_model_v2_tflite", "STORY_MOOD_MODEL_V2_DATA_H")
    write_header(BASE_DIR / "model_v2_compatible.h", "story_mood_model_tflite", "STORY_MOOD_MODEL_V2_COMPATIBLE_DATA_H")


if __name__ == "__main__":
    main()
