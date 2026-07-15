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