#include "StoryMoodModel.h"
#include "model.h"

#include <ArduTFLite.h>

namespace {
  constexpr int kTensorArenaSize = 32 * 1024;
  byte tensorArena[kTensorArenaSize];

  bool modelReady = false;
}

bool initStoryMoodModel() {
  Serial.println("Initializing ArduTFLite story mood model...");

  bool ok = modelInit(
    story_mood_model_tflite,
    tensorArena,
    kTensorArenaSize
  );

  if (!ok) {
    Serial.println("modelInit() failed.");
    modelReady = false;
    return false;
  }

  Serial.println("Story mood model initialized with ArduTFLite.");
  modelReady = true;
  return true;
}

int runStoryMoodModel(
  const float features[FEATURE_COUNT],
  float outputProbabilities[4]
) {
  if (!modelReady) {
    Serial.println("Story mood model not ready.");
    return -1;
  }

  for (int i = 0; i < FEATURE_COUNT; i++) {
    bool ok = modelSetInput(features[i], i);

    if (!ok) {
      Serial.print("modelSetInput() failed at index ");
      Serial.println(i);
      return -1;
    }
  }

  bool invokeOk = modelRunInference();

  if (!invokeOk) {
    Serial.println("modelRunInference() failed.");
    return -1;
  }

  int bestIndex = 0;
  float bestValue = modelGetOutput(0);

  for (int i = 0; i < 4; i++) {
    float value = modelGetOutput(i);
    outputProbabilities[i] = value;

    if (value > bestValue) {
      bestValue = value;
      bestIndex = i;
    }
  }

  return bestIndex;
}

const char* modelMoodName(int moodId) {
  switch (moodId) {
    case 0:
      return "quiet";
    case 1:
      return "warm";
    case 2:
      return "strange";
    case 3:
      return "lonely";
    default:
      return "unknown";
  }
}

unsigned int getStoryMoodModelSize() {
  return story_mood_model_tflite_len;
}