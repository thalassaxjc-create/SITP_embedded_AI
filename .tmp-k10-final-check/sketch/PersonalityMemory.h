#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\PersonalityMemory.h"
#ifndef PERSONALITY_MEMORY_H
#define PERSONALITY_MEMORY_H

#include <Arduino.h>
#include "PersonalityState.h"

bool loadPersonalityFromStorage(
  PersonalityState& personality
);

void savePersonalityToStorage(
  const PersonalityState& personality
);

void maybeSavePersonalityToStorage(
  const PersonalityState& personality
);

void resetPersonalityStorage(
  PersonalityState& personality
);

#endif