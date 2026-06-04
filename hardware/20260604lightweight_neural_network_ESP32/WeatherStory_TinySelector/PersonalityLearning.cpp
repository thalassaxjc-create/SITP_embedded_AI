#include "PersonalityLearning.h"

static float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

static float lerpFloat(float current, float target, float amount) {
  return current + (target - current) * amount;
}

void updatePersonalityFromWeatherFeatures(
  PersonalityState& personality,
  const float features[FEATURE_COUNT]
) {
  float humidity = features[F_HUMIDITY_NORM];

  bool isDay = features[F_IS_DAY] > 0.5f;
  bool isHumid = features[F_IS_HUMID] > 0.5f;
  bool isRainy = features[F_IS_RAINY] > 0.5f;
  bool isCloudy = features[F_IS_CLOUDY] > 0.5f;
  bool isStormy = features[F_IS_STORMY] > 0.5f;

  float wetness = 0.0f;

  wetness += humidity * 0.55f;

  if (isHumid) {
    wetness += 0.15f;
  }

  if (isRainy) {
    wetness += 0.20f;
  }

  if (isCloudy) {
    wetness += 0.08f;
  }

  if (isStormy) {
    wetness += 0.18f;
  }

  wetness = clamp01(wetness);

  /*
    这些 baseline 是它平时的性格中心。
    天气不会瞬间改写它，只会让它缓慢靠近某种状态。
  */
  float targetCalm = 0.55f;
  float targetWeird = 0.30f;
  float targetWarm = 0.40f;
  float targetLonely = 0.25f;

  /*
    长期潮湿：
    lonely 上升
    warm 下降
    calm 略微上升
  */
  targetLonely += wetness * 0.45f;
  targetWarm -= wetness * 0.25f;
  targetCalm += wetness * 0.10f;

  /*
    夜晚也会轻微推向 lonely。
  */
  if (!isDay) {
    targetLonely += 0.12f;
    targetWarm -= 0.05f;
  }

  /*
    暴风、异常天气推向 strange。
  */
  if (isStormy) {
    targetWeird += 0.35f;
    targetCalm -= 0.15f;
  }

  targetCalm = clamp01(targetCalm);
  targetWeird = clamp01(targetWeird);
  targetWarm = clamp01(targetWarm);
  targetLonely = clamp01(targetLonely);

  /*
    learningRate 越小，性格变化越慢。
    0.03 表示每次刷新只改变一点点。
    这样“长期潮湿”才会逐渐积累，而不是一次下雨就变 lonely。
  */
  const float learningRate = 0.03f;

  personality.calm = clamp01(
    lerpFloat(personality.calm, targetCalm, learningRate)
  );

  personality.weird = clamp01(
    lerpFloat(personality.weird, targetWeird, learningRate)
  );

  personality.warm = clamp01(
    lerpFloat(personality.warm, targetWarm, learningRate)
  );

  personality.lonely = clamp01(
    lerpFloat(personality.lonely, targetLonely, learningRate)
  );
}

void printPersonalityState(
  const PersonalityState& personality
) {
  Serial.println();
  Serial.println("Personality State:");

  Serial.print("calm: ");
  Serial.println(personality.calm, 4);

  Serial.print("weird: ");
  Serial.println(personality.weird, 4);

  Serial.print("warm: ");
  Serial.println(personality.warm, 4);

  Serial.print("lonely: ");
  Serial.println(personality.lonely, 4);
}