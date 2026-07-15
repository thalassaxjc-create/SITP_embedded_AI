#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\PersonalityLearning.h"
#ifndef PERSONALITY_LEARNING_H
#define PERSONALITY_LEARNING_H

#include <Arduino.h>
#include "PersonalityState.h"
#include "FeatureExtractor.h"

void updatePersonalityFromWeatherFeatures(
  PersonalityState& personality,
  const float features[FEATURE_COUNT]
);

void printPersonalityState(
  const PersonalityState& personality
);

#endif