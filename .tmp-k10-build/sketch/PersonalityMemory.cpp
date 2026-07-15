#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\PersonalityMemory.cpp"
#include "PersonalityMemory.h"
#include <Preferences.h>

namespace {
  Preferences prefs;

  const char* STORAGE_NAMESPACE = "story_ai";

  const uint32_t STORAGE_MAGIC = 0x53544F52; // "STOR"
  const uint32_t STORAGE_VERSION = 1;

  const float SAVE_EPSILON = 0.003f;

  bool hasLastSaved = false;
  PersonalityState lastSavedPersonality;

  float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
  }

  bool personalityChangedEnough(
    const PersonalityState& a,
    const PersonalityState& b
  ) {
    if (abs(a.calm - b.calm) > SAVE_EPSILON) return true;
    if (abs(a.weird - b.weird) > SAVE_EPSILON) return true;
    if (abs(a.warm - b.warm) > SAVE_EPSILON) return true;
    if (abs(a.lonely - b.lonely) > SAVE_EPSILON) return true;
    return false;
  }

  void rememberLastSaved(
    const PersonalityState& personality
  ) {
    lastSavedPersonality = personality;
    hasLastSaved = true;
  }
}

bool loadPersonalityFromStorage(
  PersonalityState& personality
) {
  prefs.begin(STORAGE_NAMESPACE, true);

  uint32_t magic = prefs.getUInt("magic", 0);
  uint32_t version = prefs.getUInt("version", 0);

  if (magic != STORAGE_MAGIC || version != STORAGE_VERSION) {
    prefs.end();
    return false;
  }

  personality.calm = clamp01(
    prefs.getFloat("calm", personality.calm)
  );

  personality.weird = clamp01(
    prefs.getFloat("weird", personality.weird)
  );

  personality.warm = clamp01(
    prefs.getFloat("warm", personality.warm)
  );

  personality.lonely = clamp01(
    prefs.getFloat("lonely", personality.lonely)
  );

  prefs.end();

  rememberLastSaved(personality);
  return true;
}

void savePersonalityToStorage(
  const PersonalityState& personality
) {
  prefs.begin(STORAGE_NAMESPACE, false);

  prefs.putUInt("magic", STORAGE_MAGIC);
  prefs.putUInt("version", STORAGE_VERSION);

  prefs.putFloat("calm", clamp01(personality.calm));
  prefs.putFloat("weird", clamp01(personality.weird));
  prefs.putFloat("warm", clamp01(personality.warm));
  prefs.putFloat("lonely", clamp01(personality.lonely));

  prefs.end();

  rememberLastSaved(personality);

  Serial.println("Personality saved to storage.");
}

void maybeSavePersonalityToStorage(
  const PersonalityState& personality
) {
  if (!hasLastSaved) {
    savePersonalityToStorage(personality);
    return;
  }

  if (personalityChangedEnough(personality, lastSavedPersonality)) {
    savePersonalityToStorage(personality);
  }
}

void resetPersonalityStorage(
  PersonalityState& personality
) {
  personality = makeDefaultPersonalityState();

  prefs.begin(STORAGE_NAMESPACE, false);
  prefs.clear();
  prefs.end();

  savePersonalityToStorage(personality);

  Serial.println("Personality storage reset to default.");
}