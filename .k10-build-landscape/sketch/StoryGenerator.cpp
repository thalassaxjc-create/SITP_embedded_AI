#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\StoryGenerator.cpp"
#include <Arduino.h>
#include "StoryGenerator.h"
#include "StoryVocab.h"

static String lastGeneratedStory = "";

struct LetterContext {
  String fromCity;
  String toCity;
  String fromTemp;
  String toTemp;
  String tempDiff;
  String fromWeather;
  String toWeather;
  String timeOfDay;
};

static const char* const rainDetails[] = {
  "The rain reached the windows before I did.",
  "My shoes are making a small wet sound.",
  "Someone forgot an umbrella beside the door.",
  "The bicycle seats outside are not trustworthy.",
  "The awning keeps dripping after the rain slows."
};

static const char* const windDetails[] = {
  "The wind keeps testing the classroom door.",
  "Tree leaves are moving faster than the people.",
  "A sign near the street has started complaining.",
  "Riding a bike today would require negotiation.",
  "My hair has accepted defeat."
};

static const char* const hotDetails[] = {
  "The fan is doing its best and losing.",
  "A cold drink sounds like a serious plan.",
  "The shade outside the classroom is crowded.",
  "The device feels warm enough to have opinions.",
  "I have decided not to move unless necessary."
};

static const char* const coldDetails[] = {
  "The glass has a thin fog on it.",
  "I keep pulling my sleeves over my hands.",
  "A hot drink would solve at least one problem.",
  "The ride back will feel longer tonight.",
  "The air near the window is sharper than expected."
};

static const char* const nightDetails[] = {
  "The dorm lights are scattered across the dark.",
  "The lab is quiet except for the fan.",
  "A late bus passed with almost nobody inside.",
  "The unfinished work is still looking at me.",
  "The night market lights would be useful right now."
};

static const char* const humidDetails[] = {
  "Even my notes are beginning to curl.",
  "The air feels like it is holding its breath.",
  "The hallway floor looks slightly reflective.",
  "My sketchbook has bent at the corners.",
  "Everything dries slowly today."
};

static const char* const cloudyDetails[] = {
  "The sky is bright but undecided.",
  "The windows have lost their shadows.",
  "The afternoon looks paused.",
  "Clouds are making the campus quieter.",
  "The light is soft enough for nobody to trust it."
};

static const char* const clearDetails[] = {
  "The sky is clear enough to make the data look polite.",
  "Sunlight has reached the table.",
  "The shadows outside are unusually confident.",
  "The weather is behaving better than my schedule.",
  "People have started walking as if they planned it."
};

static const char* const bigDifferenceConnections[] = {
  "The {TEMP_DIFF}C gap between us feels unfair.",
  "You have different weather today. I am noticing.",
  "Please keep your forecast; mine is busy being difficult.",
  "If we could trade weather for ten minutes, I would.",
  "Your side sounds like a different afternoon."
};

static const char* const closeWeatherConnections[] = {
  "Our forecasts are close enough to share one table.",
  "Both cities seem to be having the same kind of day.",
  "The two screens almost agree, which is rare.",
  "Your weather and mine could pass for classmates.",
  "Maybe both sidewalks sound similar right now."
};

static int getEffectiveCount(
  int count,
  float randomness
) {
  if (count <= 0) {
    return 0;
  }

  if (randomness < 0.40f && count > 4) {
    return 4;
  }

  if (randomness < 0.65f && count > 7) {
    return 7;
  }

  return count;
}

static String pickText(
  const char* const options[],
  int count,
  float randomness
) {
  int effectiveCount = getEffectiveCount(count, randomness);
  if (effectiveCount <= 0) {
    return "";
  }

  return String(options[random(effectiveCount)]);
}

static String weatherName(
  const WeatherTags& tags
) {
  if (tags.stormy) return "stormy";
  if (tags.snowy) return "snowy";
  if (tags.rainy) return "rainy";
  if (tags.windy) return "windy";
  if (tags.cloudy) return "cloudy";
  if (tags.hot) return "hot";
  if (tags.cold) return "cold";
  if (tags.humid) return "humid";
  return "clear";
}

static String dayName(
  const WeatherTags& tags
) {
  return tags.night ? "Tonight" : "Today";
}

static LetterContext makeContext(
  const char* fromCity,
  const WeatherData& fromWeather,
  const WeatherTags& fromTags,
  const char* toCity,
  const WeatherData& toWeather,
  const WeatherTags& toTags
) {
  LetterContext ctx;
  ctx.fromCity = fromCity;
  ctx.toCity = toCity;
  ctx.fromTemp = String(fromWeather.temperature, 1);
  ctx.toTemp = String(toWeather.temperature, 1);
  ctx.tempDiff = String(fabsf(fromWeather.temperature - toWeather.temperature), 1);
  ctx.fromWeather = weatherName(fromTags);
  ctx.toWeather = weatherName(toTags);
  ctx.timeOfDay = dayName(fromTags);
  return ctx;
}

static String applyContext(
  String text,
  const LetterContext& ctx
) {
  text.replace("{FROM_CITY}", ctx.fromCity);
  text.replace("{TO_CITY}", ctx.toCity);
  text.replace("{FROM_TEMP}", ctx.fromTemp);
  text.replace("{TO_TEMP}", ctx.toTemp);
  text.replace("{TEMP_DIFF}", ctx.tempDiff);
  text.replace("{FROM_WEATHER}", ctx.fromWeather);
  text.replace("{TO_WEATHER}", ctx.toWeather);
  text.replace("{TIME_OF_DAY}", ctx.timeOfDay);
  return text;
}

static String pickWeatherDetail(
  const WeatherTags& tags,
  float randomness
) {
  if (tags.night && randomness < 0.70f) {
    return pickText(nightDetails, (int)(sizeof(nightDetails) / sizeof(nightDetails[0])), randomness);
  }

  if (tags.rainy || tags.stormy) {
    return pickText(rainDetails, (int)(sizeof(rainDetails) / sizeof(rainDetails[0])), randomness);
  }

  if (tags.windy) {
    return pickText(windDetails, (int)(sizeof(windDetails) / sizeof(windDetails[0])), randomness);
  }

  if (tags.hot) {
    return pickText(hotDetails, (int)(sizeof(hotDetails) / sizeof(hotDetails[0])), randomness);
  }

  if (tags.cold) {
    return pickText(coldDetails, (int)(sizeof(coldDetails) / sizeof(coldDetails[0])), randomness);
  }

  if (tags.humid) {
    return pickText(humidDetails, (int)(sizeof(humidDetails) / sizeof(humidDetails[0])), randomness);
  }

  if (tags.cloudy) {
    return pickText(cloudyDetails, (int)(sizeof(cloudyDetails) / sizeof(cloudyDetails[0])), randomness);
  }

  return pickText(clearDetails, (int)(sizeof(clearDetails) / sizeof(clearDetails[0])), randomness);
}

static String pickConnection(
  const LetterVocabSet& vocab,
  const WeatherData& fromWeather,
  const WeatherTags& fromTags,
  const WeatherData& toWeather,
  const WeatherTags& toTags,
  float randomness,
  const LetterContext& ctx
) {
  float diff = fabsf(fromWeather.temperature - toWeather.temperature);
  if (diff >= 5.0f) {
    return applyContext(
      pickText(bigDifferenceConnections,
        (int)(sizeof(bigDifferenceConnections) / sizeof(bigDifferenceConnections[0])),
        randomness),
      ctx
    );
  }

  if (weatherName(fromTags) == weatherName(toTags)) {
    return applyContext(
      pickText(closeWeatherConnections,
        (int)(sizeof(closeWeatherConnections) / sizeof(closeWeatherConnections[0])),
        randomness),
      ctx
    );
  }

  return applyContext(
    pickText(vocab.connections, vocab.connectionCount, randomness),
    ctx
  );
}

static void appendLine(
  String& story,
  const String& line
) {
  if (line.length() == 0) return;
  if (story.length() > 0) story += "\n";
  story += line;
}

static String buildLetter(
  const LetterVocabSet& vocab,
  const WeatherData& fromWeather,
  const WeatherTags& fromTags,
  const WeatherData& toWeather,
  const WeatherTags& toTags,
  const StoryControl& control,
  const LetterContext& ctx
) {
  String story;
  story.reserve(360);

  String opening = applyContext(
    pickText(vocab.openings, vocab.openingCount, control.randomness),
    ctx
  );
  String observation = applyContext(
    pickText(vocab.observations, vocab.observationCount, control.randomness),
    ctx
  );
  String weatherDetail = pickWeatherDetail(fromTags, control.randomness);
  String connection = pickConnection(
    vocab,
    fromWeather,
    fromTags,
    toWeather,
    toTags,
    control.randomness,
    ctx
  );
  String ending = applyContext(
    pickText(vocab.endings, vocab.endingCount, control.randomness),
    ctx
  );

  appendLine(story, opening);

  if (control.template_id == TEMPLATE_WAITING) {
    appendLine(story, observation);
    appendLine(story, connection);
  } else if (control.template_id == TEMPLATE_DISCOVERY) {
    appendLine(story, weatherDetail);
    appendLine(story, connection);
  } else if (control.template_id == TEMPLATE_CHANGE) {
    appendLine(story, weatherDetail);
    appendLine(story, observation);
    appendLine(story, connection);
  } else {
    appendLine(story, observation);
    appendLine(story, connection);
  }

  if (control.length_id == LENGTH_MEDIUM || control.randomness >= 0.55f) {
    appendLine(story, ending);
  }

  return story;
}

String generateStory(
  const char* fromCity,
  const WeatherData& fromWeather,
  const WeatherTags& fromTags,
  const char* toCity,
  const WeatherData& toWeather,
  const WeatherTags& toTags,
  const StoryControl& control
) {
  const LetterVocabSet& vocab = getVocabSet(control.vocab_bank_id);
  LetterContext ctx = makeContext(
    fromCity,
    fromWeather,
    fromTags,
    toCity,
    toWeather,
    toTags
  );

  String story = buildLetter(
    vocab,
    fromWeather,
    fromTags,
    toWeather,
    toTags,
    control,
    ctx
  );

  if (story == lastGeneratedStory) {
    story = buildLetter(
      vocab,
      fromWeather,
      fromTags,
      toWeather,
      toTags,
      control,
      ctx
    );
  }

  lastGeneratedStory = story;
  return story;
}
