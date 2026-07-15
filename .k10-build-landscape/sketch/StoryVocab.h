#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StoryVocab.h"
#ifndef STORY_VOCAB_H
#define STORY_VOCAB_H

#include <Arduino.h>
#include "StoryControl.h"

struct LetterVocabSet {
  const char* const* openings;
  int openingCount;

  const char* const* observations;
  int observationCount;

  const char* const* connections;
  int connectionCount;

  const char* const* endings;
  int endingCount;
};

const LetterVocabSet& getVocabSet(
  int vocabBankId
);

const char* pickFragment(
  const char* const words[],
  int count
);

#endif
