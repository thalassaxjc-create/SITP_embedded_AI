#include "unihiker_k10.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "enter WIFI name";
const char* password = " enter the password";

UNIHIKER_K10 k10;
uint8_t screen_dir = 2;

// 你现在用的是蒙特利尔附近坐标
const char* weatherUrl =
  "https://api.open-meteo.com/v1/forecast?"
  "latitude=45.508888&longitude=-73.561668"
  "&current=temperature_2m,relative_humidity_2m,precipitation,rain,weather_code,wind_speed_10m,is_day"
  "&timezone=auto";

struct WeatherData {
  String time;
  float temperature;
  int humidity;
  float precipitation;
  float rain;
  int weatherCode;
  float windSpeed;
  int isDay;
};
struct WeatherTags {
  bool hot;
  bool cold;
  bool cool;
  bool mild;

  bool humid;
  bool dry;

  bool rainy;
  bool cloudy;
  bool stormy;
  bool windy;

  bool daytime;
  bool night;

  String tempMood;
  String airMood;
  String skyMood;
  String windMood;
  String lightMood;
  WeatherData currentWeather;

};
// 分页全局变量
WeatherData currentWeather;
WeatherTags currentTags;
String currentSentence = "";
String currentStory = "";
bool hasWeatherData = false;
int currentPage = 0;
unsigned long lastPageSwitch = 0;
const unsigned long PAGE_INTERVAL = 5000;  // 5 秒切一页
// 分页函数声明
void showWeatherPage(const WeatherData &w, const WeatherTags &t, const String &sentence);
void showStoryPage(const WeatherData &w, const WeatherTags &t, const String &story);
void showCurrentPage();


// 先声明函数，让 initDisplay() 能提前调用它
void showStatus(String line1, String line2 = "", String line3 = "");

void initDisplay() {
  k10.begin();
  k10.initScreen(screen_dir);
  k10.creatCanvas();

  showStatus("Starting...");
}
void clearPage() {
  k10.canvas->canvasClear();
  k10.setScreenBackground(0xFFFFFF);
}
void showStatus(String line1, String line2, String line3) {
  clearPage();

  k10.canvas->canvasText("Weather Story", 1, 0x000000);
  k10.canvas->canvasText(line1, 3, 0x0000FF);

  if (line2.length() > 0) {
    k10.canvas->canvasText(line2, 4, 0x000000);
  }

  if (line3.length() > 0) {
    k10.canvas->canvasText(line3, 5, 0x000000);
  }

  k10.canvas->updateCanvas();
}
int drawWrappedText(String text, int startLine, int maxCharsPerLine, int color) {
  int line = startLine;
  String currentLine = "";

  int index = 0;

  while (index < text.length()) {
    char c = text.charAt(index);

    if (c == '\n') {
      if (currentLine.length() > 0) {
        k10.canvas->canvasText(currentLine, line, color);
        line++;
        currentLine = "";
      }
      line++;
      index++;
      continue;
    }

    int nextSpace = text.indexOf(' ', index);
    int nextNewline = text.indexOf('\n', index);

    int nextBreak;

    if (nextSpace == -1 && nextNewline == -1) {
      nextBreak = text.length();
    } else if (nextSpace == -1) {
      nextBreak = nextNewline;
    } else if (nextNewline == -1) {
      nextBreak = nextSpace;
    } else if (nextSpace < nextNewline) {
      nextBreak = nextSpace;
    } else {
      nextBreak = nextNewline;
    }

    String word = text.substring(index, nextBreak);

    if (currentLine.length() == 0) {
      currentLine = word;
    } else if (currentLine.length() + 1 + word.length() <= maxCharsPerLine) {
      currentLine += " ";
      currentLine += word;
    } else {
      k10.canvas->canvasText(currentLine, line, color);
      line++;
      currentLine = word;
    }

    index = nextBreak;

    while (index < text.length() && text.charAt(index) == ' ') {
      index++;
    }
  }

  if (currentLine.length() > 0) {
    k10.canvas->canvasText(currentLine, line, color);
    line++;
  }

  return line;
}
void showStoryOnScreen(const WeatherData &w, const WeatherTags &t, const String &story) {
  k10.setScreenBackground(0xFFFFFF);

  k10.canvas->canvasText("Weather Story", 1, 0x000000);

  String tempLine = "Temp: " + String(w.temperature, 1) + " C";
  String skyLine = "Sky: " + t.skyMood;
  String windLine = "Wind: " + t.windMood;

  k10.canvas->canvasText(tempLine, 3, 0x000000);
  k10.canvas->canvasText(skyLine, 4, 0x0000FF);
  k10.canvas->canvasText(windLine, 5, 0x0000FF);

  k10.canvas->canvasText("Story:", 7, 0x000000);
  drawWrappedText(story, 8, 21, 0x333333);

  k10.canvas->updateCanvas();
}
void connectWiFi() {
  Serial.println("Connecting WiFi...");
  showStatus("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int retry = 0;

  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");

    String dots = "";
    for (int i = 0; i < retry % 10; i++) {
      dots += ".";
    }

    showStatus("Connecting WiFi", dots);

    retry++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    showStatus("WiFi connected", WiFi.localIP().toString());
    delay(1000);
  } else {
    Serial.println("WiFi failed.");
    showStatus("WiFi failed", "Check SSID/password", "or use hotspot");
    delay(3000);
  }
}


bool fetchWeather(WeatherData &w) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot fetch weather: WiFi not connected.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(8000);

  Serial.println();
  Serial.println("Requesting weather API...");

  bool ok = http.begin(client, weatherUrl);

  if (!ok) {
    Serial.println("HTTP begin failed.");
    return false;
  }

  int httpCode = http.GET();

  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  if (httpCode != 200) {
    Serial.print("HTTP failed: ");
    Serial.println(http.errorToString(httpCode));
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  Serial.println();
  Serial.println("Raw JSON received.");
  Serial.print("Payload length: ");
  Serial.println(payload.length());

  // 4096 对当前这个 JSON 足够。
  // 如果之后请求更多字段，可能需要增大。
  DynamicJsonDocument doc(4096);

  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return false;
  }

  JsonObject current = doc["current"];

  w.time = current["time"].as<String>();
  w.temperature = current["temperature_2m"] | -999.0;
  w.humidity = current["relative_humidity_2m"] | -1;
  w.precipitation = current["precipitation"] | -1.0;
  w.rain = current["rain"] | -1.0;
  w.weatherCode = current["weather_code"] | -1;
  w.windSpeed = current["wind_speed_10m"] | -1.0;
  w.isDay = current["is_day"] | -1;

  return true;
}

bool isRainCode(int code) {
  // Open-Meteo / WMO weather code:
  // 51-67: drizzle / rain
  // 80-82: rain showers
  return (code >= 51 && code <= 67) ||
         (code >= 80 && code <= 82);
}

bool isCloudCode(int code) {
  // 1-3: mainly clear / partly cloudy / overcast
  // 45, 48: fog
  return (code >= 1 && code <= 3) ||
         code == 45 ||
         code == 48;
}

bool isStormCode(int code) {
  // 95-99: thunderstorm
  return code >= 95 && code <= 99;
}

WeatherTags makeTags(const WeatherData &w) {
  WeatherTags t;
Serial.println();
  Serial.println("=== Debug makeTags input ===");
  Serial.print("temperature = ");
  Serial.println(w.temperature);
  Serial.print("humidity = ");
  Serial.println(w.humidity);
  Serial.print("precipitation = ");
  Serial.println(w.precipitation);
  Serial.print("rain = ");
  Serial.println(w.rain);
  Serial.print("weatherCode = ");
  Serial.println(w.weatherCode);
  Serial.print("windSpeed = ");
  Serial.println(w.windSpeed);
  Serial.print("isDay = ");
  Serial.println(w.isDay);
  Serial.println("============================");
  // Temperature
  t.hot = w.temperature >= 30.0;
  t.cold = w.temperature <= 5.0;
  t.cool = w.temperature > 5.0 && w.temperature <= 18.0;
  t.mild = w.temperature > 18.0 && w.temperature < 30.0;

  if (t.hot) {
    t.tempMood = "hot";
  } else if (t.cold) {
    t.tempMood = "cold";
  } else if (t.cool) {
    t.tempMood = "cool";
  } else {
    t.tempMood = "mild";
  }

  // Humidity
  t.humid = w.humidity >= 75;
  t.dry = w.humidity <= 35;

  if (t.humid) {
    t.airMood = "humid";
  } else if (t.dry) {
    t.airMood = "dry";
  } else {
    t.airMood = "normal";
  }

  // Rain / cloud / storm
  t.rainy = w.rain > 0.0 ||
            w.precipitation > 0.0 ||
            isRainCode(w.weatherCode);

  t.cloudy = isCloudCode(w.weatherCode);
  t.stormy = isStormCode(w.weatherCode);

  if (t.stormy) {
    t.skyMood = "stormy";
  } else if (t.rainy) {
    t.skyMood = "rainy";
  } else if (t.cloudy) {
    t.skyMood = "cloudy";
  } else {
    t.skyMood = "clear";
  }

  // Wind
  t.windy = w.windSpeed >= 20.0;

  if (t.windy) {
    t.windMood = "windy";
  } else {
    t.windMood = "calm";
  }

  // Light
  t.daytime = w.isDay == 1;
  t.night = w.isDay == 0;

  if (t.night) {
    t.lightMood = "night";
  } else {
    t.lightMood = "day";
  }

  return t;
}
void printTags(const WeatherTags &t) {
  Serial.println();
  Serial.println("=== Weather Tags ===");

  Serial.print("Temperature mood: ");
  Serial.println(t.tempMood);

  Serial.print("Air mood: ");
  Serial.println(t.airMood);

  Serial.print("Sky mood: ");
  Serial.println(t.skyMood);

  Serial.print("Wind mood: ");
  Serial.println(t.windMood);

  Serial.print("Light mood: ");
  Serial.println(t.lightMood);

  Serial.println();
  Serial.print("hot: ");
  Serial.println(t.hot);

  Serial.print("cold: ");
  Serial.println(t.cold);

  Serial.print("cool: ");
  Serial.println(t.cool);

  Serial.print("mild: ");
  Serial.println(t.mild);

  Serial.print("humid: ");
  Serial.println(t.humid);

  Serial.print("dry: ");
  Serial.println(t.dry);

  Serial.print("rainy: ");
  Serial.println(t.rainy);

  Serial.print("cloudy: ");
  Serial.println(t.cloudy);

  Serial.print("stormy: ");
  Serial.println(t.stormy);

  Serial.print("windy: ");
  Serial.println(t.windy);

  Serial.print("daytime: ");
  Serial.println(t.daytime);

  Serial.print("night: ");
  Serial.println(t.night);

  Serial.println("====================");
}
String pickText(const char* options[], int count) {
  int index = random(count);
  return String(options[index]);
}
const char* characters[] = {
  "A small cloud",
  "A quiet streetlamp",
  "A lost cat",
  "A tiny messenger",
  "A sleepy window"
};

const char* places[] = {
  "waited beside the window",
  "stood under the roof",
  "walked across the empty street",
  "hid near the corner",
  "listened from the balcony"
};

const char* cool_images[] = {
  "The cool air made every sound softer.",
  "The city seemed to breathe more slowly.",
  "A thin chill passed through the quiet street."
};

const char* cloudy_images[] = {
  "Clouds covered the sky like folded paper.",
  "The moon was hidden behind a grey curtain.",
  "The sky kept its thoughts to itself."
};

const char* rainy_images[] = {
  "Rain tapped gently on the ground.",
  "Small puddles opened their eyes in the street.",
  "The rain carried tiny messages from the sky."
};

const char* windy_images[] = {
  "The wind ran past and forgot where it was going.",
  "Leaves turned over like pages of a small book.",
  "The air moved as if looking for someone."
};

const char* calm_images[] = {
  "Nothing hurried.",
  "Even the shadows stayed still.",
  "The silence rested on the rooftops."
};

const char* night_endings[] = {
  "Under the night sky, the city became a tiny story.",
  "By midnight, even the clouds seemed to fall asleep.",
  "The night kept the little secret safe."
};

const char* day_endings[] = {
  "In the daylight, everything looked almost new.",
  "The day carried the story down the street.",
  "By afternoon, the sky had almost forgotten it."
};
String buildTinyStory(const WeatherTags &t) {
  String story = "";

  // Sentence 1: character + place/action
  story += pickText(characters, 5);
  story += " ";
  story += pickText(places, 5);
  story += ".";

  story += "\n";

  // Sentence 2: weather image
  if (t.rainy) {
    story += pickText(rainy_images, 3);
  } else if (t.cloudy) {
    story += pickText(cloudy_images, 3);
  } else if (t.cool) {
    story += pickText(cool_images, 3);
  } else {
    story += "The weather left a soft mark on the street.";
  }

  story += "\n";

  // Sentence 3: wind/calm + ending
  if (t.windy) {
    story += pickText(windy_images, 3);
  } else {
    story += pickText(calm_images, 3);
  }

  story += " ";

  if (t.night) {
    story += pickText(night_endings, 3);
  } else {
    story += pickText(day_endings, 3);
  }

  return story;
}
String buildWeatherSentence(const WeatherTags &t) {
  String sentence = "Today feels ";

  // Temperature
  if (t.hot) {
    sentence += "hot";
  } else if (t.cold) {
    sentence += "cold";
  } else if (t.cool) {
    sentence += "cool";
  } else {
    sentence += "mild";
  }

  // Sky
  if (t.stormy) {
    sentence += ", stormy";
  } else if (t.rainy) {
    sentence += ", rainy";
  } else if (t.cloudy) {
    sentence += ", cloudy";
  } else {
    sentence += ", clear";
  }

  // Wind
  if (t.windy) {
    sentence += ", and windy";
  } else {
    sentence += ", and calm";
  }

  // Light
  if (t.night) {
    sentence += " under the night sky.";
  } else {
    sentence += " in the daylight.";
  }

  return sentence;
}
void printWeather(const WeatherData &w) {
  Serial.println();
  Serial.println("=== Parsed Weather ===");

  Serial.print("Time: ");
  Serial.println(w.time);

  Serial.print("Temperature: ");
  Serial.print(w.temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(w.humidity);
  Serial.println(" %");

  Serial.print("Precipitation: ");
  Serial.print(w.precipitation);
  Serial.println(" mm");

  Serial.print("Rain: ");
  Serial.print(w.rain);
  Serial.println(" mm");

  Serial.print("Weather code: ");
  Serial.println(w.weatherCode);

  Serial.print("Wind speed: ");
  Serial.print(w.windSpeed);
  Serial.println(" km/h");

  Serial.print("Is day: ");
  Serial.println(w.isDay);

  Serial.println("======================");
}

void showWeatherPage(const WeatherData &w, const WeatherTags &t, const String &sentence) {
  clearPage();

  k10.canvas->canvasText("Weather Story  [1/2]", 1, 0x000000);
  k10.canvas->canvasText("Weather Summary", 2, 0x0000FF);

  String tempLine = "Temp: " + String(w.temperature, 1) + " C";
  String humidLine = "Humidity: " + String(w.humidity) + " %";
  String skyLine = "Sky: " + t.skyMood;
  String windLine = "Wind: " + t.windMood;
  String lightLine = "Light: " + t.lightMood;

  k10.canvas->canvasText(tempLine, 4, 0x000000);
  k10.canvas->canvasText(humidLine, 5, 0x000000);
  k10.canvas->canvasText(skyLine, 6, 0x0000FF);
  k10.canvas->canvasText(windLine, 7, 0x0000FF);
  k10.canvas->canvasText(lightLine, 8, 0x0000FF);

  k10.canvas->canvasText("Sentence:", 10, 0x000000);
  drawWrappedText(sentence, 11, 21, 0x333333);

  k10.canvas->updateCanvas();
}
void showStoryPage(const WeatherData &w, const WeatherTags &t, const String &story) {
  clearPage();

  k10.canvas->canvasText("Weather Story  [2/2]", 1, 0x000000);
  k10.canvas->canvasText("Tiny Story", 2, 0x0000FF);

  String moodLine = t.tempMood + ", " + t.skyMood + ", " + t.windMood;
  k10.canvas->canvasText(moodLine, 4, 0x000000);

  drawWrappedText(story, 6, 21, 0x333333);

  k10.canvas->updateCanvas();
}
void showCurrentPage() {
  if (!hasWeatherData) {
    showStatus("No weather data");
    return;
  }

  if (currentPage == 0) {
    showWeatherPage(currentWeather, currentTags, currentSentence);
  } else {
    showStoryPage(currentWeather, currentTags, currentStory);
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  randomSeed(millis());

  initDisplay();

  Serial.println();
  Serial.println("=== Weather Display Test ===");

  showStatus("Boot OK", "Starting WiFi...");
  delay(800);

  connectWiFi();

  WeatherData weather;

  bool success = fetchWeather(weather);

  if (success) {
    printWeather(weather);

    WeatherTags tags = makeTags(weather);
    printTags(tags);

    String sentence = buildWeatherSentence(tags);
String story = buildTinyStory(tags);

Serial.println();
Serial.println("=== Weather Sentence ===");
Serial.println(sentence);
Serial.println("========================");

Serial.println();
Serial.println("=== Tiny Weather Story ===");
Serial.println(story);
Serial.println("==========================");

currentWeather = weather;
currentTags = tags;
currentSentence = sentence;
currentStory = story;

hasWeatherData = true;
currentPage = 0;
lastPageSwitch = millis();

showStatus("Displaying pages...");
delay(500);

showCurrentPage();
  } else {
    Serial.println("Weather fetch or parse failed.");

    showStatus("Weather failed", "Check Serial Monitor");
  }
}
void loop() {
  if (!hasWeatherData) {
    return;
  }

  unsigned long now = millis();

  if (now - lastPageSwitch >= PAGE_INTERVAL) {
    currentPage = 1 - currentPage;  // 0 变 1，1 变 0
    lastPageSwitch = now;

    showCurrentPage();
  }
}