#include <Arduino.h>
#include "FeatureExtractor.h"

static float clamp01(float value) {
  if (value < 0.0f) return 0.0f;
  if (value > 1.0f) return 1.0f;
  return value;
}

static float normalizeRange(float value, float minValue, float maxValue) {
  if (maxValue == minValue) return 0.0f;
  return clamp01((value - minValue) / (maxValue - minValue));
}

void makeFeatures(
  const WeatherData& weather,
  const WeatherTags& tags,
  const PersonalityState& personality,
  float features[FEATURE_COUNT]
) {
  features[F_TEMP_NORM] = normalizeRange(weather.temperature, -10.0f, 40.0f);
  features[F_HUMIDITY_NORM] = normalizeRange((float)weather.humidity, 0.0f, 100.0f);
  features[F_RAIN_NORM] = normalizeRange(weather.rain, 0.0f, 20.0f);
  features[F_WIND_NORM] = normalizeRange(weather.windSpeed, 0.0f, 60.0f);

  features[F_IS_DAY] = tags.daytime ? 1.0f : 0.0f;
  features[F_IS_HOT] = tags.hot ? 1.0f : 0.0f;
  features[F_IS_HUMID] = tags.humid ? 1.0f : 0.0f;
  features[F_IS_RAINY] = tags.rainy ? 1.0f : 0.0f;
  features[F_IS_CLOUDY] = tags.cloudy ? 1.0f : 0.0f;
  features[F_IS_STORMY] = tags.stormy ? 1.0f : 0.0f;

  features[F_PERSONALITY_CALM] = clamp01(personality.calm);
  features[F_PERSONALITY_WEIRD] = clamp01(personality.weird);
  features[F_PERSONALITY_WARM] = clamp01(personality.warm);
  features[F_PERSONALITY_LONELY] = clamp01(personality.lonely);
}

const char* featureName(int index) {
  switch (index) {
    case F_TEMP_NORM: return "temp_norm";
    case F_HUMIDITY_NORM: return "humidity_norm";
    case F_RAIN_NORM: return "rain_norm";
    case F_WIND_NORM: return "wind_norm";

    case F_IS_DAY: return "is_day";
    case F_IS_HOT: return "is_hot";
    case F_IS_HUMID: return "is_humid";
    case F_IS_RAINY: return "is_rainy";
    case F_IS_CLOUDY: return "is_cloudy";
    case F_IS_STORMY: return "is_stormy";

    case F_PERSONALITY_CALM: return "personality_calm";
    case F_PERSONALITY_WEIRD: return "personality_weird";
    case F_PERSONALITY_WARM: return "personality_warm";
    case F_PERSONALITY_LONELY: return "personality_lonely";

    default: return "unknown";
  }
}