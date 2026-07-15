#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StorySelector.h"
#ifndef STORY_SELECTOR_H
#define STORY_SELECTOR_H

#include "StoryControl.h"
#include "FeatureExtractor.h"

StoryControl selectStoryControl(
  const float features[FEATURE_COUNT]
);

StoryControl makeStoryControlFromMood(
  int moodId
);

#endif