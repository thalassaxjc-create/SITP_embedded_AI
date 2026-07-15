#include <Arduino.h>
#include "StorySelector.h"

StoryControl selectStoryControl(
  const float features[FEATURE_COUNT]
) {
  StoryControl control;

  bool isDay = features[F_IS_DAY] > 0.5f;
  bool isHot = features[F_IS_HOT] > 0.5f;
  bool isHumid = features[F_IS_HUMID] > 0.5f;
  bool isRainy = features[F_IS_RAINY] > 0.5f;
  bool isStormy = features[F_IS_STORMY] > 0.5f;

  float calm = features[F_PERSONALITY_CALM];
  float weird = features[F_PERSONALITY_WEIRD];
  float warm = features[F_PERSONALITY_WARM];
  float lonely = features[F_PERSONALITY_LONELY];

  if ((isRainy || isHumid) && !isDay && calm >= weird) {
    control.mood_id = MOOD_QUIET;
    control.template_id = TEMPLATE_WAITING;
    control.vocab_bank_id = VOCAB_RAIN_ROOM;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.35;
  }
  else if (isHot && isDay && warm >= weird) {
    control.mood_id = MOOD_WARM;
    control.template_id = TEMPLATE_DISCOVERY;
    control.vocab_bank_id = VOCAB_SUN_ANIMAL;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.45;
  }
  else if (weird > 0.60f || isStormy) {
    control.mood_id = MOOD_STRANGE;
    control.template_id = TEMPLATE_CHANGE;
    control.vocab_bank_id = VOCAB_STRANGE_OBJECTS;
    control.length_id = LENGTH_MEDIUM;
    control.randomness = 0.75;
  }
  else if (!isDay || lonely > 0.55f) {
    control.mood_id = MOOD_LONELY;
    control.template_id = TEMPLATE_RETURN;
    control.vocab_bank_id = VOCAB_NIGHT_CITY;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.50;
  }
  else {
    control.mood_id = MOOD_QUIET;
    control.template_id = TEMPLATE_WAITING;
    control.vocab_bank_id = VOCAB_RAIN_ROOM;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.40;
  }

  return control;
}

StoryControl makeStoryControlFromMood(
  int moodId
) {
  StoryControl control;

  if (moodId == MOOD_QUIET) {
    control.mood_id = MOOD_QUIET;
    control.template_id = TEMPLATE_WAITING;
    control.vocab_bank_id = VOCAB_RAIN_ROOM;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.35;
  }
  else if (moodId == MOOD_WARM) {
    control.mood_id = MOOD_WARM;
    control.template_id = TEMPLATE_DISCOVERY;
    control.vocab_bank_id = VOCAB_SUN_ANIMAL;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.45;
  }
  else if (moodId == MOOD_STRANGE) {
    control.mood_id = MOOD_STRANGE;
    control.template_id = TEMPLATE_CHANGE;
    control.vocab_bank_id = VOCAB_STRANGE_OBJECTS;
    control.length_id = LENGTH_MEDIUM;
    control.randomness = 0.75;
  }
  else if (moodId == MOOD_LONELY) {
    control.mood_id = MOOD_LONELY;
    control.template_id = TEMPLATE_RETURN;
    control.vocab_bank_id = VOCAB_NIGHT_CITY;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.50;
  }
  else {
    control.mood_id = MOOD_QUIET;
    control.template_id = TEMPLATE_WAITING;
    control.vocab_bank_id = VOCAB_RAIN_ROOM;
    control.length_id = LENGTH_SHORT;
    control.randomness = 0.40;
  }

  return control;
}