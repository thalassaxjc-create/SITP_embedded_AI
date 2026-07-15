"""Convert both trained Keras models to float32 TFLite models."""

from pathlib import Path
import tempfile

import tensorflow as tf

from common_v2 import FULL_MODEL_FILE, WEATHER_MODEL_FILE

OUTPUTS = {
    FULL_MODEL_FILE: Path("story_mood_model_v2.tflite"),
    WEATHER_MODEL_FILE: Path("story_mood_model_weather_only.tflite"),
}


def main():
    for keras_path, relative_output in OUTPUTS.items():
        if not keras_path.exists():
            raise FileNotFoundError(f"Missing {keras_path.name}; run train_selector_v2.py first")
        model = tf.keras.models.load_model(keras_path)
        # SavedModel is more robust than direct Keras conversion across
        # TensorFlow/Keras version combinations (including TF 2.21 on Windows).
        # Keep the temporary path beside the model: some Windows TensorFlow
        # builds cannot handle a non-ASCII user TEMP path.
        with tempfile.TemporaryDirectory(prefix=".story_selector_v2_", dir=keras_path.parent) as temp_dir:
            saved_model_dir = str(Path(temp_dir) / "saved_model")
            model.export(saved_model_dir, format="tf_saved_model")
            converter = tf.lite.TFLiteConverter.from_saved_model(saved_model_dir)
            converted = converter.convert()
        output = keras_path.parent / relative_output
        output.write_bytes(converted)
        print(f"Converted {keras_path.name} -> {output.name} ({len(converted)} bytes)")


if __name__ == "__main__":
    main()
