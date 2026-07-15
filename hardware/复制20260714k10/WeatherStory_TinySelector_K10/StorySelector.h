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