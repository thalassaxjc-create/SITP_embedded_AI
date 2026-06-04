#include <Arduino.h>
#include "StoryGenerator.h"
#include "StoryVocab.h"

static String lastGeneratedStory = "";

static int getEffectiveCount(
  int count,
  float randomness
) {
  if (count <= 0) {
    return 0;
  }

  // randomness 低：只从前几个稳定词里选
  if (randomness < 0.40f && count > 3) {
    return 3;
  }

  // randomness 中等：从较大但不是全部的词库里选
  if (randomness < 0.65f && count > 6) {
    return 6;
  }

  // randomness 高：使用完整词库
  return count;
}

static String pickVocabText(
  const char* const options[],
  int count,
  float randomness
) {
  int effectiveCount = getEffectiveCount(count, randomness);

  if (effectiveCount <= 0) {
    return "";
  }

  int index = random(effectiveCount);
  return String(options[index]);
}

static String pickSubject(
  const VocabSet& vocab,
  float randomness
) {
  return pickVocabText(
    vocab.subjects,
    vocab.subjectCount,
    randomness
  );
}

static String pickObject(
  const VocabSet& vocab,
  float randomness
) {
  return pickVocabText(
    vocab.objects,
    vocab.objectCount,
    randomness
  );
}

static String pickVerb(
  const VocabSet& vocab,
  float randomness
) {
  return pickVocabText(
    vocab.verbs,
    vocab.verbCount,
    randomness
  );
}

static String pickPlace(
  const VocabSet& vocab,
  float randomness
) {
  return pickVocabText(
    vocab.places,
    vocab.placeCount,
    randomness
  );
}

static String pickAdjective(
  const VocabSet& vocab,
  float randomness
) {
  return pickVocabText(
    vocab.adjectives,
    vocab.adjectiveCount,
    randomness
  );
}

static String pickEnding(
  const VocabSet& vocab,
  float randomness
) {
  return pickVocabText(
    vocab.endings,
    vocab.endingCount,
    randomness
  );
}

String generateWaitingStory(
  const StoryControl& control
) {
  const VocabSet& vocab = getVocabSet(control.vocab_bank_id);

  String subject = pickSubject(vocab, control.randomness);
  String object = pickObject(vocab, control.randomness);
  String verb = pickVerb(vocab, control.randomness);
  String place = pickPlace(vocab, control.randomness);
  String adjective = pickAdjective(vocab, control.randomness);
  String ending = pickEnding(vocab, control.randomness);

  String story = "";

  story += "A ";
  story += adjective;
  story += " ";
  story += object;
  story += " waited ";
  story += place;
  story += ".";
  story += "\n";

  story += subject;
  story += " ";
  story += verb;
  story += " the weather until everything became quiet.";

  if (control.length_id == LENGTH_MEDIUM) {
    story += "\n";
    story += ending;
  }

  return story;
}

String generateDiscoveryStory(
  const StoryControl& control
) {
  const VocabSet& vocab = getVocabSet(control.vocab_bank_id);

  String subject = pickSubject(vocab, control.randomness);
  String object = pickObject(vocab, control.randomness);
  String verb = pickVerb(vocab, control.randomness);
  String place = pickPlace(vocab, control.randomness);
  String adjective = pickAdjective(vocab, control.randomness);
  String ending = pickEnding(vocab, control.randomness);

  String story = "";

  story += subject;
  story += " ";
  story += verb;
  story += " a ";
  story += adjective;
  story += " ";
  story += object;
  story += " ";
  story += place;
  story += ".";
  story += "\n";

  story += "For a moment, the day looked almost new.";

  if (control.length_id == LENGTH_MEDIUM) {
    story += "\n";
    story += ending;
  }

  return story;
}

String generateChangeStory(
  const StoryControl& control
) {
  const VocabSet& vocab = getVocabSet(control.vocab_bank_id);

  String subject = pickSubject(vocab, control.randomness);
  String object = pickObject(vocab, control.randomness);
  String verb = pickVerb(vocab, control.randomness);
  String place = pickPlace(vocab, control.randomness);
  String adjective = pickAdjective(vocab, control.randomness);
  String ending = pickEnding(vocab, control.randomness);

  String story = "";

  story += "At noon, ";
  story += subject;
  story += " ";
  story += verb;
  story += " a ";
  story += adjective;
  story += " ";
  story += object;
  story += " ";
  story += place;
  story += ".";
  story += "\n";

  story += "No one noticed, except the weather.";

  if (control.length_id == LENGTH_MEDIUM) {
    story += "\n";
    story += ending;
  }

  return story;
}

String generateReturnStory(
  const StoryControl& control
) {
  const VocabSet& vocab = getVocabSet(control.vocab_bank_id);

  String subject = pickSubject(vocab, control.randomness);
  String object = pickObject(vocab, control.randomness);
  String verb = pickVerb(vocab, control.randomness);
  String place = pickPlace(vocab, control.randomness);
  String adjective = pickAdjective(vocab, control.randomness);
  String ending = pickEnding(vocab, control.randomness);

  String story = "";

  story += subject;
  story += " ";
  story += verb;
  story += " a ";
  story += adjective;
  story += " ";
  story += object;
  story += " ";
  story += place;
  story += ".";
  story += "\n";

  story += ending;

  if (control.length_id == LENGTH_MEDIUM) {
    story += "\n";
    story += "The city kept the little secret safe.";
  }

  return story;
}

String generateStory(
  const WeatherData& weather,
  const WeatherTags& tags,
  const StoryControl& control
) {
  // 现在 weather / tags 暂时用得不多，但保留在参数里。
  // 之后可以用它们微调句子，例如加入温度、雨、昼夜等细节。
  (void)weather;
  (void)tags;

  String story = "";

  if (control.template_id == TEMPLATE_WAITING) {
    story = generateWaitingStory(control);
  }
  else if (control.template_id == TEMPLATE_DISCOVERY) {
    story = generateDiscoveryStory(control);
  }
  else if (control.template_id == TEMPLATE_CHANGE) {
    story = generateChangeStory(control);
  }
  else {
    story = generateReturnStory(control);
  }

  /*
    避免连续两次完全相同。
    如果刚好生成了同一个故事，就再生成一次。
  */
  if (story == lastGeneratedStory) {
    if (control.template_id == TEMPLATE_WAITING) {
      story = generateWaitingStory(control);
    }
    else if (control.template_id == TEMPLATE_DISCOVERY) {
      story = generateDiscoveryStory(control);
    }
    else if (control.template_id == TEMPLATE_CHANGE) {
      story = generateChangeStory(control);
    }
    else {
      story = generateReturnStory(control);
    }
  }

  lastGeneratedStory = story;

  return story;
}