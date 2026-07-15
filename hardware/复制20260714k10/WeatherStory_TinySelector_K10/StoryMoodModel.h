#ifndef STORY_MOOD_MODEL_H
#define STORY_MOOD_MODEL_H

#include <Arduino.h>
#include "FeatureExtractor.h"

bool initStoryMoodModel();

int runStoryMoodModel(
  const float features[FEATURE_COUNT],
  float outputProbabilities[4]
);

const char* modelMoodName(int moodId);
unsigned int getStoryMoodModelSize();

#endif