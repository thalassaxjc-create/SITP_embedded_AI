#ifndef STORY_GENERATOR_H
#define STORY_GENERATOR_H

#include <Arduino.h>
#include "WeatherData.h"
#include "StoryControl.h"

// StoryGenerator = 真正写故事的部分。
// 它不再直接决定“今天应该是什么风格”，而是服从 StoryControl。
String generateStory(
  const WeatherData& weather,
  const WeatherTags& tags,
  const StoryControl& control
);

#endif
