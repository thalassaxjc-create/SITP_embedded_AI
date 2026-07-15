#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StoryVocab.cpp"
#include "StoryVocab.h"

#define COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))

// --------------------------------------------------
// QUIET / RAIN ROOM
// --------------------------------------------------

const char* const quietSubjects[] = {
  "the room",
  "the rain",
  "a cup",
  "the window",
  "a soft lamp",
  "the floor",
  "a small table",
  "the curtain",
  "a sleeping book",
  "the quiet wall"
};

const char* const quietObjects[] = {
  "a drop of water",
  "a folded note",
  "a quiet sound",
  "a sleeping book",
  "a silver line",
  "a small reflection",
  "a warm cup",
  "a slow shadow",
  "a thin trace of light",
  "a quiet breath"
};

const char* const quietVerbs[] = {
  "listened to",
  "held",
  "covered",
  "waited beside",
  "softened",
  "followed",
  "counted",
  "kept",
  "touched",
  "remembered"
};

const char* const quietPlaces[] = {
  "beside the window",
  "inside the rain",
  "under the soft light",
  "near the quiet wall",
  "on the wooden floor",
  "behind the curtain",
  "at the edge of the table",
  "inside the small room"
};

const char* const quietAdjectives[] = {
  "soft",
  "still",
  "dim",
  "silver",
  "slow",
  "gentle",
  "small",
  "clear",
  "pale",
  "quiet"
};

const char* const quietEndings[] = {
  "Nothing hurried.",
  "The room stayed awake for a little longer.",
  "Even the rain seemed to speak softly.",
  "No one needed to answer.",
  "The small silence remained.",
  "Everything became a little lighter."
};


// --------------------------------------------------
// WARM / SUN ANIMAL
// --------------------------------------------------

const char* const warmSubjects[] = {
  "a small animal",
  "the sun",
  "a childlike cloud",
  "a bright leaf",
  "the garden",
  "a yellow bird",
  "a warm stone",
  "the morning",
  "a little path",
  "a sleepy cat"
};

const char* const warmObjects[] = {
  "a seed",
  "a round shadow",
  "a golden sound",
  "a tiny map",
  "a warm secret",
  "a piece of bread",
  "a bright feather",
  "a sleeping flower",
  "a soft footprint",
  "a small bell"
};

const char* const warmVerbs[] = {
  "found",
  "carried",
  "followed",
  "woke",
  "shared",
  "opened",
  "warmed",
  "protected",
  "noticed",
  "welcomed"
};

const char* const warmPlaces[] = {
  "under the morning sun",
  "beside the garden path",
  "near the bright grass",
  "inside a warm breeze",
  "at the edge of the yard",
  "under a yellow tree",
  "beside a sleeping flower",
  "near the open door"
};

const char* const warmAdjectives[] = {
  "warm",
  "golden",
  "bright",
  "round",
  "gentle",
  "soft",
  "small",
  "sleepy",
  "kind",
  "green"
};

const char* const warmEndings[] = {
  "The day became easier to enter.",
  "Something small was glad to be found.",
  "The light stayed close.",
  "No one was alone for long.",
  "The garden kept the secret kindly.",
  "The morning answered with warmth."
};


// --------------------------------------------------
// STRANGE / STRANGE OBJECTS
// --------------------------------------------------

const char* const strangeSubjects[] = {
  "the mirror",
  "a clock",
  "a mushroom",
  "the blue chair",
  "a paper fish",
  "the hallway",
  "a glass spoon",
  "the old radio",
  "a door without a room",
  "a quiet machine"
};

const char* const strangeObjects[] = {
  "its own name",
  "a backwards shadow",
  "a missing minute",
  "a folded moon",
  "a small impossible sound",
  "a map of nowhere",
  "a wooden star",
  "a sleeping question",
  "a second sky",
  "a secret alphabet"
};

const char* const strangeVerbs[] = {
  "changed",
  "misplaced",
  "invented",
  "swallowed",
  "returned",
  "translated",
  "hid",
  "borrowed",
  "grew",
  "forgot"
};

const char* const strangePlaces[] = {
  "inside the mirror",
  "behind the clock",
  "under the table",
  "in the wrong afternoon",
  "beside the blue door",
  "between two shadows",
  "inside a glass room",
  "near the sleeping machine"
};

const char* const strangeAdjectives[] = {
  "blue",
  "crooked",
  "impossible",
  "backwards",
  "hollow",
  "silent",
  "glass",
  "wrong",
  "tiny",
  "unwritten"
};

const char* const strangeEndings[] = {
  "After that, the room refused to be ordinary.",
  "No one could remember which way time had gone.",
  "The object pretended this was normal.",
  "A second silence appeared.",
  "The air changed its mind.",
  "Something small learned a new shape."
};


// --------------------------------------------------
// LONELY / NIGHT CITY
// --------------------------------------------------

const char* const lonelySubjects[] = {
  "the city",
  "a window",
  "a quiet street",
  "a small light",
  "the station",
  "a distant room",
  "the night",
  "a forgotten house",
  "a late train",
  "a blue sign"
};

const char* const lonelyObjects[] = {
  "a shadow",
  "a letter",
  "a closed door",
  "a quiet song",
  "a forgotten sound",
  "a small memory",
  "a paper bird",
  "an empty chair",
  "a distant voice",
  "a name in the rain"
};

const char* const lonelyVerbs[] = {
  "remembered",
  "waited for",
  "carried",
  "hid",
  "followed",
  "called back",
  "lost",
  "kept",
  "looked for",
  "returned to"
};

const char* const lonelyPlaces[] = {
  "under the night sky",
  "near the wet glass",
  "beside the empty station",
  "at the end of the street",
  "inside the blue dark",
  "behind a quiet window",
  "under a distant lamp",
  "near the last train"
};

const char* const lonelyAdjectives[] = {
  "distant",
  "pale",
  "quiet",
  "blue",
  "thin",
  "forgotten",
  "slow",
  "silent",
  "empty",
  "late"
};

const char* const lonelyEndings[] = {
  "By night, it returned as a tiny story.",
  "No one saw it leave.",
  "The city kept listening.",
  "The window stayed bright for no reason.",
  "Something far away almost answered.",
  "The street remembered a softer time."
};


// --------------------------------------------------
// VOCAB SETS
// --------------------------------------------------

const VocabSet quietSet = {
  quietSubjects, COUNT_OF(quietSubjects),
  quietObjects, COUNT_OF(quietObjects),
  quietVerbs, COUNT_OF(quietVerbs),
  quietPlaces, COUNT_OF(quietPlaces),
  quietAdjectives, COUNT_OF(quietAdjectives),
  quietEndings, COUNT_OF(quietEndings)
};

const VocabSet warmSet = {
  warmSubjects, COUNT_OF(warmSubjects),
  warmObjects, COUNT_OF(warmObjects),
  warmVerbs, COUNT_OF(warmVerbs),
  warmPlaces, COUNT_OF(warmPlaces),
  warmAdjectives, COUNT_OF(warmAdjectives),
  warmEndings, COUNT_OF(warmEndings)
};

const VocabSet strangeSet = {
  strangeSubjects, COUNT_OF(strangeSubjects),
  strangeObjects, COUNT_OF(strangeObjects),
  strangeVerbs, COUNT_OF(strangeVerbs),
  strangePlaces, COUNT_OF(strangePlaces),
  strangeAdjectives, COUNT_OF(strangeAdjectives),
  strangeEndings, COUNT_OF(strangeEndings)
};

const VocabSet lonelySet = {
  lonelySubjects, COUNT_OF(lonelySubjects),
  lonelyObjects, COUNT_OF(lonelyObjects),
  lonelyVerbs, COUNT_OF(lonelyVerbs),
  lonelyPlaces, COUNT_OF(lonelyPlaces),
  lonelyAdjectives, COUNT_OF(lonelyAdjectives),
  lonelyEndings, COUNT_OF(lonelyEndings)
};


// --------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------

const VocabSet& getVocabSet(
  int vocabBankId
) {
  switch (vocabBankId) {
    case VOCAB_RAIN_ROOM:
      return quietSet;

    case VOCAB_SUN_ANIMAL:
      return warmSet;

    case VOCAB_STRANGE_OBJECTS:
      return strangeSet;

    case VOCAB_NIGHT_CITY:
      return lonelySet;

    default:
      return quietSet;
  }
}

const char* pickWord(
  const char* const words[],
  int count
) {
  if (count <= 0) {
    return "";
  }

  int index = random(count);
  return words[index];
}