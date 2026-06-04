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