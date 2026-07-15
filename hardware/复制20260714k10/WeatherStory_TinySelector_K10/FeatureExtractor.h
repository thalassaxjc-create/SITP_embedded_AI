#ifndef FEATURE_EXTRACTOR_H
#define FEATURE_EXTRACTOR_H

#include <Arduino.h>
#include "WeatherData.h"
#include "PersonalityState.h"

enum FeatureIndex {
  F_TEMP_NORM = 0,
  F_HUMIDITY_NORM,
  F_RAIN_NORM,
  F_WIND_NORM,

  F_IS_DAY,
  F_IS_HOT,
  F_IS_HUMID,
  F_IS_RAINY,
  F_IS_CLOUDY,
  F_IS_STORMY,

  F_PERSONALITY_CALM,
  F_PERSONALITY_WEIRD,
  F_PERSONALITY_WARM,
  F_PERSONALITY_LONELY,

  FEATURE_COUNT
};

void makeFeatures(
  const WeatherData& weather,
  const WeatherTags& tags,
  const PersonalityState& personality,
  float features[FEATURE_COUNT]
);

const char* featureName(int index);

#endif