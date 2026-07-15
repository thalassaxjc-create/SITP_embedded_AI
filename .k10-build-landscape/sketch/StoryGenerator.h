#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StoryGenerator.h"
#ifndef STORY_GENERATOR_H
#define STORY_GENERATOR_H

#include <Arduino.h>
#include "WeatherData.h"
#include "StoryControl.h"

// StoryGenerator = 真正写故事的部分。
// 它不再直接决定“今天应该是什么风格”，而是服从 StoryControl。
String generateStory(
  const char* fromCity,
  const WeatherData& fromWeather,
  const WeatherTags& fromTags,
  const char* toCity,
  const WeatherData& toWeather,
  const WeatherTags& toTags,
  const StoryControl& control
);

#endif
