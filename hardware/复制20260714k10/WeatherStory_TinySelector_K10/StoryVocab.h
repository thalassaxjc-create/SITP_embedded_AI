#ifndef STORY_VOCAB_H
#define STORY_VOCAB_H

#include <Arduino.h>
#include "StoryControl.h"

struct VocabSet {
  const char* const* subjects;
  int subjectCount;

  const char* const* objects;
  int objectCount;

  const char* const* verbs;
  int verbCount;

  const char* const* places;
  int placeCount;

  const char* const* adjectives;
  int adjectiveCount;

  const char* const* endings;
  int endingCount;
};

const VocabSet& getVocabSet(
  int vocabBankId
);

const char* pickWord(
  const char* const words[],
  int count
);

#endif