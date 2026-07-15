#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StoryVocab.cpp"
#include "StoryVocab.h"

#define COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))

// The first entries in each group are the safest demo sentences. Higher
// randomness can reach the stranger, more opinionated fragments later on.

const char* const quietOpenings[] = {
  "Hello from {FROM_CITY}.",
  "{TIME_OF_DAY} in {FROM_CITY} has gone quiet.",
  "I am writing this beside a window in {FROM_CITY}.",
  "The room is almost empty now.",
  "A small weather note arrived from {FROM_CITY}.",
  "The library table is free for once.",
  "I waited a minute before sending this.",
  "No hurry to answer this."
};

const char* const quietObservations[] = {
  "{FROM_CITY} is {FROM_WEATHER} at {FROM_TEMP}C.",
  "The screen says {FROM_WEATHER}, and the window agrees.",
  "The weather here is {FROM_WEATHER}; it makes people lower their voices.",
  "My notes are beside the forecast: {FROM_TEMP}C.",
  "The campus path looks slower in this {FROM_WEATHER} weather.",
  "Someone left a wet umbrella near the door.",
  "The bicycle seats outside still look suspicious.",
  "The library windows have turned the color of the sky.",
  "Even the vending machine sounds quieter today.",
  "A sensor on this side has been watching the air.",
  "The forecast changed, but the room did not.",
  "The small machine reports the weather more calmly than I do."
};

const char* const quietConnections[] = {
  "{TO_CITY} is {TO_WEATHER} at {TO_TEMP}C.",
  "Your city is {TEMP_DIFF}C away from mine today.",
  "The two forecasts fit on one small screen.",
  "Your weather stayed open on the display after class.",
  "Maybe your sidewalk is drier than mine.",
  "If your side is also quiet, we can call it a tie.",
  "The API brought your temperature here before dinner.",
  "Our weather is close enough to share one sentence.",
  "The robot carried this note like it was careful with it.",
  "I keep checking whether your sky changed too."
};

const char* const quietEndings[] = {
  "No reply is needed.",
  "I only wanted to send the weather.",
  "I will leave the letter here.",
  "The screen can keep it for a while.",
  "That is all for now.",
  "The room stayed awake a little longer.",
  "Please read it when your side slows down.",
  "The last line is just the temperature."
};

const char* const warmOpenings[] = {
  "Good afternoon from {FROM_CITY}.",
  "{FROM_CITY} feels easier to walk through today.",
  "I am sending a small bright report.",
  "Lunch ended and the weather behaved for once.",
  "This is a decent day for a cold drink.",
  "The campus finally looks less tired.",
  "Someone laughed near the convenience store.",
  "I have almost forgiven the forecast."
};

const char* const warmObservations[] = {
  "{FROM_CITY} is {FROM_WEATHER} at {FROM_TEMP}C.",
  "The air here is {FROM_WEATHER}, which feels almost useful.",
  "The shade outside the classroom is doing real work.",
  "A drink shop sign looks more persuasive than usual.",
  "The sun came out after lunch and surprised everyone.",
  "My bag is warm from sitting near the window.",
  "The campus path smells a little like rain and food.",
  "The forecast says {FROM_TEMP}C; my face believes it.",
  "The device is warm too, but that may be its personality.",
  "People are walking slower, not unhappily.",
  "Even the model seemed to choose a softer mood.",
  "A small breeze crossed the table at the right time."
};

const char* const warmConnections[] = {
  "{TO_CITY} is {TO_WEATHER} at {TO_TEMP}C.",
  "Your city is only {TEMP_DIFF}C different today.",
  "If your side is warm too, buy the cold drink.",
  "The two temperatures look like they could share lunch.",
  "Your forecast arrived while I was choosing dinner.",
  "The robot moved as if it had good news.",
  "A tiny model decided this was worth sending.",
  "Maybe both campuses have the same slow afternoon.",
  "Your weather makes this note feel less random.",
  "I hope your shade is better than ours."
};

const char* const warmEndings[] = {
  "Save something cold for later.",
  "This seems worth reporting.",
  "That is the whole cheerful part.",
  "I am taking this as a good sign.",
  "Please accept this very small update.",
  "The screen looked pleased with itself.",
  "I will send another note when the sky changes.",
  "For now, the weather is behaving."
};

const char* const strangeOpenings[] = {
  "The model chose strange again.",
  "Something odd happened in {FROM_CITY}.",
  "The sensor has an opinion today.",
  "I checked the forecast twice.",
  "This machine is trying to sound calm.",
  "The sky and the API may need to talk.",
  "I am reporting a minor weather disagreement.",
  "Please inspect the following cloud when convenient."
};

const char* const strangeObservations[] = {
  "{FROM_CITY} is listed as {FROM_WEATHER} at {FROM_TEMP}C.",
  "The sensor says {FROM_WEATHER}; the window is considering it.",
  "The forecast changed while I was still reading it.",
  "A number on the screen blinked like it knew too much.",
  "The motor started before I finished judging the sky.",
  "The air here feels like a bug report.",
  "The device checked the weather and became involved.",
  "A cloud may have left before this message did.",
  "The classroom fan is making its own prediction.",
  "My notes now contain both data and suspicion.",
  "The weather code looks normal, which is suspicious.",
  "The small screen is too confident for its size."
};

const char* const strangeConnections[] = {
  "{TO_CITY} reports {TO_WEATHER} at {TO_TEMP}C.",
  "Your side differs by {TEMP_DIFF}C, which feels intentional.",
  "If your sky is clearer, I may have received the wrong cloud.",
  "The two cities disagree in a very organized way.",
  "The API placed both forecasts in the same tiny room.",
  "Please return any spare sunshine when convenient.",
  "The robot has already treated this as urgent.",
  "Your temperature made the model change its mind.",
  "Maybe your side can verify whether this is weather.",
  "I am choosing not to argue with the data yet."
};

const char* const strangeEndings[] = {
  "Please advise the sky.",
  "I will file this under weather.",
  "Do not trust the cloud completely.",
  "The machine appears satisfied.",
  "This is probably normal.",
  "I am not promising that it makes sense.",
  "The next reading may deny everything.",
  "End of report, unfortunately."
};

const char* const lonelyOpenings[] = {
  "{TIME_OF_DAY} in {FROM_CITY} is still awake.",
  "The last class ended a while ago.",
  "I am sending this after the room emptied.",
  "The station lights are still on.",
  "A late note from {FROM_CITY}.",
  "The lab is quiet except for the fan.",
  "I missed the easy time to write.",
  "The screen lit up before anyone answered."
};

const char* const lonelyObservations[] = {
  "{FROM_CITY} is {FROM_WEATHER} at {FROM_TEMP}C.",
  "The weather here is {FROM_WEATHER}, and the glass looks tired.",
  "The forecast is the only new thing on my desk.",
  "The corridor feels longer in this weather.",
  "A bus passed with almost nobody inside.",
  "The unfinished work stayed exactly where it was.",
  "The air has cooled around the window frame.",
  "The vending machine light is doing its best.",
  "The sensor keeps measuring things after everyone leaves.",
  "The campus gate looked farther away than usual.",
  "The rain mark on my shoe has not dried yet.",
  "The small robot moved, then the room went quiet again."
};

const char* const lonelyConnections[] = {
  "{TO_CITY} is {TO_WEATHER} at {TO_TEMP}C.",
  "Your temperature changed by {TEMP_DIFF}C from mine.",
  "Your weather is still glowing on this screen.",
  "The two cities are both awake, apparently.",
  "I do not know if your side is quiet too.",
  "The API brought your sky here without making noise.",
  "A reply can wait until morning.",
  "The distance is just numbers tonight.",
  "The robot delivered the note and stopped.",
  "Your forecast stayed after the browser closed."
};

const char* const lonelyEndings[] = {
  "I will leave the letter here.",
  "No need to answer tonight.",
  "The screen can keep watch.",
  "That is enough for this hour.",
  "The last train has probably gone.",
  "I am going to pack up slowly.",
  "Maybe the next reading will be warmer.",
  "The light is still on."
};

const LetterVocabSet quietSet = {
  quietOpenings, COUNT_OF(quietOpenings),
  quietObservations, COUNT_OF(quietObservations),
  quietConnections, COUNT_OF(quietConnections),
  quietEndings, COUNT_OF(quietEndings)
};

const LetterVocabSet warmSet = {
  warmOpenings, COUNT_OF(warmOpenings),
  warmObservations, COUNT_OF(warmObservations),
  warmConnections, COUNT_OF(warmConnections),
  warmEndings, COUNT_OF(warmEndings)
};

const LetterVocabSet strangeSet = {
  strangeOpenings, COUNT_OF(strangeOpenings),
  strangeObservations, COUNT_OF(strangeObservations),
  strangeConnections, COUNT_OF(strangeConnections),
  strangeEndings, COUNT_OF(strangeEndings)
};

const LetterVocabSet lonelySet = {
  lonelyOpenings, COUNT_OF(lonelyOpenings),
  lonelyObservations, COUNT_OF(lonelyObservations),
  lonelyConnections, COUNT_OF(lonelyConnections),
  lonelyEndings, COUNT_OF(lonelyEndings)
};

const LetterVocabSet& getVocabSet(
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

const char* pickFragment(
  const char* const words[],
  int count
) {
  if (count <= 0) {
    return "";
  }

  int index = random(count);
  return words[index];
}
