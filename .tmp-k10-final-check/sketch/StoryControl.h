#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StoryControl.h"
#ifndef STORY_CONTROL_H
#define STORY_CONTROL_H

#include <Arduino.h>

// mood_id = 故事气质
enum MoodId {
  MOOD_QUIET = 0,
  MOOD_WARM = 1,
  MOOD_STRANGE = 2,
  MOOD_LONELY = 3
};

// template_id = 故事结构
enum TemplateId {
  TEMPLATE_WAITING = 0,
  TEMPLATE_DISCOVERY = 1,
  TEMPLATE_CHANGE = 2,
  TEMPLATE_RETURN = 3
};

// vocab_bank_id = 词库组
enum VocabBankId {
  VOCAB_RAIN_ROOM = 0,
  VOCAB_SUN_ANIMAL = 1,
  VOCAB_STRANGE_OBJECTS = 2,
  VOCAB_NIGHT_CITY = 3
};

// length_id = 故事长度
enum LengthId {
  LENGTH_SHORT = 0,
  LENGTH_MEDIUM = 1
};

// StoryControl = 故事生成器的“指令卡”。
// 未来 TinyML 模型主要就是输出这张指令卡，或者先输出 mood_id。
struct StoryControl {
  int mood_id;
  int template_id;
  int vocab_bank_id;
  int length_id;
  float randomness;
};

inline const char* moodName(int id) {
  switch (id) {
    case MOOD_QUIET: return "quiet";
    case MOOD_WARM: return "warm";
    case MOOD_STRANGE: return "strange";
    case MOOD_LONELY: return "lonely";
    default: return "unknown";
  }
}

inline const char* templateName(int id) {
  switch (id) {
    case TEMPLATE_WAITING: return "waiting";
    case TEMPLATE_DISCOVERY: return "discovery";
    case TEMPLATE_CHANGE: return "change";
    case TEMPLATE_RETURN: return "return";
    default: return "unknown";
  }
}

inline const char* vocabBankName(int id) {
  switch (id) {
    case VOCAB_RAIN_ROOM: return "rain_room";
    case VOCAB_SUN_ANIMAL: return "sun_animal";
    case VOCAB_STRANGE_OBJECTS: return "strange_objects";
    case VOCAB_NIGHT_CITY: return "night_city";
    default: return "unknown";
  }
}

inline const char* lengthName(int id) {
  switch (id) {
    case LENGTH_SHORT: return "short";
    case LENGTH_MEDIUM: return "medium";
    default: return "unknown";
  }
}

#endif
