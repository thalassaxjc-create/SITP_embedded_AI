#include <Arduino.h>
#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
/*
  Weather Story + TinyML-ready Story Selector
  第一阶段：架构拆分版

  现在还没有真正接入 TinyML 模型。
  StorySelector.cpp 里先用规则模拟未来的模型输出。

  Output:
  - UNIHIKER K10 screen
  - Serial Monitor at 115200 baud
*/

#include "unihiker_k10.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <esp_system.h>

#include "WeatherData.h"
#include "PersonalityState.h"
#include "StoryControl.h"
#include "FeatureExtractor.h"
#include "StorySelector.h"
#include "StoryGenerator.h"
//#include "model.h"
#include "StoryMoodModel.h"
#include "PersonalityLearning.h"
#include "PersonalityMemory.h"

// ===================== User settings / 用户设置 =====================

const char* WIFI_SSID = "enter Wifi";
const char* WIFI_PASSWORD = "enter password";

// Current example location: Montreal / 当前示例地点：蒙特利尔
const float LATITUDE = 45.508888;
const float LONGITUDE = -73.561668;

// How often to switch between Page 1 and Page 2 / 两页切换间隔
const unsigned long PAGE_INTERVAL_MS = 5000;

// K10 screen page carousel / K10 屏幕页面轮播
const bool ENABLE_PAGE_CAROUSEL = true;

// K10 direction 2 display. The K10 0.0.3 driver accepts text rows 1..13.
const uint8_t SCREEN_DIRECTION = 2;
const int SCREEN_MAX_LINE = 13;
const int SCREEN_MAX_CHARS = 21;

// How often to refresh weather data / 天气刷新间隔
const unsigned long WEATHER_REFRESH_INTERVAL_MS = 10UL * 60UL * 1000UL;

// Wi-Fi connection retry count / Wi-Fi 连接重试次数
const int WIFI_RETRY_LIMIT = 30;

// ===================== Global state / 全局状态 =====================

UNIHIKER_K10 k10;

WeatherData currentWeather;
WeatherTags currentTags;
PersonalityState currentPersonality = makeDefaultPersonalityState();
StoryControl currentControl;
float currentFeatures[FEATURE_COUNT];

// True only if the TFLite/ArduTFLite model initializes successfully.
bool storyMoodModelAvailable = false;

String currentSentence = "";
String currentStory = "";

bool hasWeatherData = false;
int currentPage = 0;

unsigned long lastPageSwitch = 0;
unsigned long lastWeatherRefresh = 0;


// ===================== K10 screen + Serial output / 屏幕与串口输出 =====================

#line 82 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void clearScreen();
#line 87 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool appendScreenLine( String lines[], int maxLines, int& lineCount, const String& value );
#line 102 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
int wrapTextToLines( const String& text, String lines[], int maxLines, int maxChars );
#line 167 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
int drawWrappedText( const String& text, int startLine, int endLine, int color );
#line 189 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void initDisplay();
#line 199 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printDivider();
#line 236 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool connectWiFi();
#line 267 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void ensureWiFiConnected();
#line 280 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
String buildWeatherUrl();
#line 291 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool fetchWeather(WeatherData& w);
#line 365 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool isRainCode(int code);
#line 373 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool isCloudCode(int code);
#line 381 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool isStormCode(int code);
#line 386 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
WeatherTags makeTags(const WeatherData& w);
#line 459 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
String buildWeatherSentence(const WeatherTags& t);
#line 499 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printWeather(const WeatherData& w);
#line 533 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printTags(const WeatherTags& t);
#line 591 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printFeatureVector(const float features[FEATURE_COUNT]);
#line 619 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printControl(const StoryControl& c);
#line 641 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printWeatherPage(const WeatherData& w, const WeatherTags& t, const String& sentence);
#line 672 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void printStoryPage( const WeatherData& w, const WeatherTags& t, const StoryControl& c, const String& story );
#line 700 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void drawWeatherPage(const WeatherData& w, const WeatherTags& t, const String& sentence);
#line 716 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void drawStoryPage( const WeatherTags& t, const StoryControl& c, const String& story );
#line 732 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void showCurrentPage();
#line 749 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
bool refreshWeatherAndStory();
#line 873 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void setup();
#line 923 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void loop();
#line 82 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\WeatherStory_TinySelector_K10.ino"
void clearScreen() {
  k10.canvas->canvasClear();
  k10.setScreenBackground(0xFFFFFF);
}

bool appendScreenLine(
  String lines[],
  int maxLines,
  int& lineCount,
  const String& value
) {
  if (lineCount >= maxLines) {
    return false;
  }

  lines[lineCount] = value;
  lineCount++;
  return true;
}

int wrapTextToLines(
  const String& text,
  String lines[],
  int maxLines,
  int maxChars
) {
  int lineCount = 0;
  String buffer = "";
  bool truncated = false;

  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      // One source newline means exactly one screen line break.
      if (!appendScreenLine(lines, maxLines, lineCount, buffer)) {
        truncated = true;
        break;
      }
      buffer = "";
      continue;
    }

    buffer += c;

    if (buffer.length() > maxChars) {
      int breakAt = buffer.lastIndexOf(' ');
      if (breakAt <= 0) {
        breakAt = maxChars;
      }

      String completedLine = buffer.substring(0, breakAt);
      completedLine.trim();

      if (!appendScreenLine(lines, maxLines, lineCount, completedLine)) {
        truncated = true;
        break;
      }

      buffer = buffer.substring(breakAt);
      buffer.trim();
    }
  }

  if (!truncated && buffer.length() > 0) {
    if (!appendScreenLine(lines, maxLines, lineCount, buffer)) {
      truncated = true;
    }
  }

  if (truncated && lineCount > 0) {
    String& lastLine = lines[lineCount - 1];
    if (lastLine.length() > maxChars - 3) {
      lastLine = lastLine.substring(0, maxChars - 3);
    }
    lastLine += "...";
  }

  return lineCount;
}

int drawWrappedText(
  const String& text,
  int startLine,
  int endLine,
  int color
) {
  const int availableLines = endLine - startLine + 1;
  String lines[SCREEN_MAX_LINE];
  int lineCount = wrapTextToLines(
    text,
    lines,
    availableLines,
    SCREEN_MAX_CHARS
  );

  for (int i = 0; i < lineCount; i++) {
    k10.canvas->canvasText(lines[i], startLine + i, color);
  }

  return startLine + lineCount;
}

void initDisplay() {
  k10.begin();
  k10.initScreen(SCREEN_DIRECTION);
  k10.creatCanvas();
  clearScreen();
  k10.canvas->canvasText("Weather Story", 1, 0x000000);
  k10.canvas->canvasText("K10 display ready", 3, 0x0000FF);
  k10.canvas->updateCanvas();
}

void printDivider() {
  Serial.println();
  Serial.println("--------------------------------------------------");
}

void showStatus(const String& line1, const String& line2 = "", const String& line3 = "") {
  printDivider();
  Serial.println("Weather Story");
  Serial.println(line1);

  if (line2.length() > 0) {
    Serial.println(line2);
  }

  if (line3.length() > 0) {
    Serial.println(line3);
  }

  Serial.println("--------------------------------------------------");

  clearScreen();
  k10.canvas->canvasText("Weather Story", 1, 0x000000);
  drawWrappedText(line1, 3, 5, 0x0000FF);

  if (line2.length() > 0) {
    drawWrappedText(line2, 7, 8, 0x000000);
  }

  if (line3.length() > 0) {
    drawWrappedText(line3, 10, 12, 0x000000);
  }

  k10.canvas->updateCanvas();
}

// ===================== Wi-Fi / Wi-Fi 连接 =====================

bool connectWiFi() {
  Serial.println("Connecting WiFi...");
  showStatus("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 0;

  while (WiFi.status() != WL_CONNECTED && retry < WIFI_RETRY_LIMIT) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    showStatus("WiFi connected", WiFi.localIP().toString());
    return true;
  }

  Serial.println("WiFi failed.");
  showStatus("WiFi failed", "Check SSID/password", "or use a phone hotspot");
  return false;
}

void ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("WiFi disconnected. Reconnecting...");
  WiFi.disconnect();
  delay(500);
  connectWiFi();
}

// ===================== Weather API / 天气 API =====================

String buildWeatherUrl() {
  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=";
  url += String(LATITUDE, 6);
  url += "&longitude=";
  url += String(LONGITUDE, 6);
  url += "&current=temperature_2m,relative_humidity_2m,precipitation,rain,weather_code,wind_speed_10m,is_day";
  url += "&timezone=auto";
  return url;
}

bool fetchWeather(WeatherData& w) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot fetch weather: WiFi not connected.");
    return false;
  }

  WiFiClientSecure client;

  /*
    Prototype setting:
    setInsecure() skips HTTPS certificate verification.
  */
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(8000);

  String weatherUrl = buildWeatherUrl();

  Serial.println();
  Serial.println("Requesting weather API...");
  Serial.println(weatherUrl);

  bool ok = http.begin(client, weatherUrl);

  if (!ok) {
    Serial.println("HTTP begin failed.");
    return false;
  }

  int httpCode = http.GET();

  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("HTTP failed: ");
    Serial.println(http.errorToString(httpCode));
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  Serial.println("Raw JSON received.");
  Serial.print("Payload length: ");
  Serial.println(payload.length());

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

// ===================== Weather interpretation / 天气解释 =====================

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

WeatherTags makeTags(const WeatherData& w) {
  WeatherTags t;

  // Temperature / 温度
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

  // Humidity / 湿度
  t.humid = w.humidity >= 75;
  t.dry = w.humidity <= 35;

  if (t.humid) {
    t.airMood = "humid";
  } else if (t.dry) {
    t.airMood = "dry";
  } else {
    t.airMood = "normal";
  }

  // Rain, cloud, storm / 雨、云、雷暴
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

  // Wind / 风
  t.windy = w.windSpeed >= 20.0;

  if (t.windy) {
    t.windMood = "windy";
  } else {
    t.windMood = "calm";
  }

  // Light / 光线
  t.daytime = w.isDay == 1;
  t.night = w.isDay == 0;

  if (t.night) {
    t.lightMood = "night";
  } else {
    t.lightMood = "day";
  }

  return t;
}

// ===================== Text generation / 文本生成 =====================

String buildWeatherSentence(const WeatherTags& t) {
  String sentence = "Today feels ";

  if (t.hot) {
    sentence += "hot";
  } else if (t.cold) {
    sentence += "cold";
  } else if (t.cool) {
    sentence += "cool";
  } else {
    sentence += "mild";
  }

  if (t.stormy) {
    sentence += ", stormy";
  } else if (t.rainy) {
    sentence += ", rainy";
  } else if (t.cloudy) {
    sentence += ", cloudy";
  } else {
    sentence += ", clear";
  }

  if (t.windy) {
    sentence += ", and windy";
  } else {
    sentence += ", and calm";
  }

  if (t.night) {
    sentence += " under the night sky.";
  } else {
    sentence += " in the daylight.";
  }

  return sentence;
}

// ===================== Debug printing / 调试输出 =====================

void printWeather(const WeatherData& w) {
  printDivider();
  Serial.println("Parsed Weather");

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
}

void printTags(const WeatherTags& t) {
  printDivider();
  Serial.println("Weather Tags");

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
}

void printFeatureVector(const float features[FEATURE_COUNT]) {
  static const char* names[FEATURE_COUNT] = {
    "temp_norm",
    "humidity_norm",
    "rain_norm",
    "wind_norm",
    "is_day",
    "is_hot",
    "is_humid",
    "is_rainy",
    "is_cloudy",
    "is_stormy",
    "personality_calm",
    "personality_weird",
    "personality_warm",
    "personality_lonely"
  };

  Serial.println();
  Serial.println("Feature Vector:");

  for (int i = 0; i < FEATURE_COUNT; i++) {
    Serial.print(names[i]);
    Serial.print(": ");
    Serial.println(features[i], 3);
  }
}

void printControl(const StoryControl& c) {
  printDivider();
  Serial.println("Story Control");

  Serial.print("mood: ");
  Serial.println(moodName(c.mood_id));

  Serial.print("template: ");
  Serial.println(templateName(c.template_id));

  Serial.print("vocab bank: ");
  Serial.println(vocabBankName(c.vocab_bank_id));

  Serial.print("length: ");
  Serial.println(lengthName(c.length_id));

  Serial.print("randomness: ");
  Serial.println(c.randomness, 2);
}

// ===================== Page display / K10 与串口分页显示 =====================

void printWeatherPage(const WeatherData& w, const WeatherTags& t, const String& sentence) {
  printDivider();
  Serial.println("Weather Story [1/2]");
  Serial.println("Weather Summary");
  Serial.println();

  Serial.print("Time: ");
  Serial.println(w.time);

  Serial.print("Temp: ");
  Serial.print(w.temperature, 1);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(w.humidity);
  Serial.println(" %");

  Serial.print("Sky: ");
  Serial.println(t.skyMood);

  Serial.print("Wind: ");
  Serial.println(t.windMood);

  Serial.print("Light: ");
  Serial.println(t.lightMood);

  Serial.println();
  Serial.println("Sentence:");
  Serial.println(sentence);
}

void printStoryPage(
  const WeatherData& w,
  const WeatherTags& t,
  const StoryControl& c,
  const String& story
) {
  printDivider();
  Serial.println("Weather Story [2/2]");
  Serial.println("Tiny Story");
  Serial.println();

  Serial.print("Weather mood: ");
  Serial.print(t.tempMood);
  Serial.print(", ");
  Serial.print(t.skyMood);
  Serial.print(", ");
  Serial.println(t.windMood);

  Serial.print("Selected mood: ");
  Serial.println(moodName(c.mood_id));

  Serial.print("Template: ");
  Serial.println(templateName(c.template_id));

  Serial.println();
  Serial.println(story);
}

void drawWeatherPage(const WeatherData& w, const WeatherTags& t, const String& sentence) {
  clearScreen();

  k10.canvas->canvasText("Weather Story [1/2]", 1, 0x000000);
  k10.canvas->canvasText("Weather Summary", 2, 0x0000FF);
  k10.canvas->canvasText("Temp: " + String(w.temperature, 1) + " C", 4, 0x000000);
  k10.canvas->canvasText("Humidity: " + String(w.humidity) + " %", 5, 0x000000);
  k10.canvas->canvasText("Sky: " + t.skyMood, 6, 0x0000FF);
  k10.canvas->canvasText("Wind: " + t.windMood, 7, 0x0000FF);
  k10.canvas->canvasText("Light: " + t.lightMood, 8, 0x0000FF);
  k10.canvas->canvasText("Sentence:", 10, 0x000000);
  drawWrappedText(sentence, 11, SCREEN_MAX_LINE, 0x333333);

  k10.canvas->updateCanvas();
}

void drawStoryPage(
  const WeatherTags& t,
  const StoryControl& c,
  const String& story
) {
  clearScreen();

  k10.canvas->canvasText("Weather Story [2/2]", 1, 0x000000);
  k10.canvas->canvasText("Tiny Story", 2, 0x0000FF);
  k10.canvas->canvasText("Weather: " + t.skyMood, 3, 0x000000);
  k10.canvas->canvasText("Mood: " + String(moodName(c.mood_id)), 4, 0x0000FF);
  drawWrappedText(story, 6, SCREEN_MAX_LINE, 0x333333);

  k10.canvas->updateCanvas();
}

void showCurrentPage() {
  if (!hasWeatherData) {
    showStatus("No weather data");
    return;
  }

  if (currentPage == 0) {
    printWeatherPage(currentWeather, currentTags, currentSentence);
    drawWeatherPage(currentWeather, currentTags, currentSentence);
  } else {
    printStoryPage(currentWeather, currentTags, currentControl, currentStory);
    drawStoryPage(currentTags, currentControl, currentStory);
  }
}

// ===================== Refresh logic / 刷新逻辑 =====================

bool refreshWeatherAndStory() {
  ensureWiFiConnected();

  WeatherData weather;
  bool success = fetchWeather(weather);

  if (!success) {
    Serial.println("Weather fetch or parse failed.");
    showStatus("Weather failed", "Check Serial Monitor");
    return false;
  }

WeatherTags tags = makeTags(weather);

float features[FEATURE_COUNT];

makeFeatures(
  weather,
  tags,
  currentPersonality,
  features
);

/*
  第一次 features 用来让天气影响 personality。
*/
updatePersonalityFromWeatherFeatures(
  currentPersonality,
  features
);

maybeSavePersonalityToStorage(
  currentPersonality
);

/*
  personality 已经变化，所以重新生成 features。
  这一次 features 会被神经网络使用。
*/
makeFeatures(
  weather,
  tags,
  currentPersonality,
  features
);

printWeather(weather);
printTags(tags);
printPersonalityState(currentPersonality);
printFeatureVector(features);

  StoryControl ruleControl = selectStoryControl(features);
  StoryControl control = ruleControl;

  Serial.println();
  Serial.print("Rule selected mood: ");
  Serial.println(moodName(ruleControl.mood_id));

  float modelProbs[4] = {0, 0, 0, 0};
  int modelMood = -1;

  if (storyMoodModelAvailable) {
    modelMood = runStoryMoodModel(features, modelProbs);
  } else {
    Serial.println("TinyML model was not initialized. Skipping model inference.");
  }

  Serial.println();
  Serial.println("TinyML Model Output:");

  if (modelMood >= 0) {
    for (int i = 0; i < 4; i++) {
      Serial.print(modelMoodName(i));
      Serial.print(": ");
      Serial.println(modelProbs[i], 4);
    }

    Serial.print("Model predicted mood: ");
    Serial.println(modelMoodName(modelMood));

    if (modelMood != ruleControl.mood_id) {
      Serial.println("Note: model and rule selector disagree.");
    }

    control = makeStoryControlFromMood(modelMood);
    Serial.println("StoryControl source: TinyML model");
  } else {
    Serial.println("Model prediction failed or skipped.");
    Serial.println("StoryControl source: rule fallback");
  }

  String sentence = buildWeatherSentence(tags);
  String story = generateStory(weather, tags, control);

  currentWeather = weather;
  currentTags = tags;
  currentControl = control;
  for (int i = 0; i < FEATURE_COUNT; i++) {
    currentFeatures[i] = features[i];
  }
  currentSentence = sentence;
  currentStory = story;

  hasWeatherData = true;
  currentPage = 0;

  printControl(control);

  Serial.println();
  Serial.println("Weather sentence:");
  Serial.println(sentence);

  Serial.println();
  Serial.println("Tiny weather story:");
  Serial.println(story);

  lastWeatherRefresh = millis();
  lastPageSwitch = millis();

  return true;
}

// ===================== Arduino lifecycle / Arduino 主程序 =====================

void setup() {
  Serial.begin(115200);
  delay(1500);

  initDisplay();

  randomSeed((uint32_t)esp_random() ^ micros());

  Serial.println();
  Serial.println("=== Weather Story + TinyML Story Selector ===");

  Serial.println("Loading personality from storage...");

bool personalityLoaded = loadPersonalityFromStorage(currentPersonality);

if (personalityLoaded) {
  Serial.println("Loaded saved personality.");
} else {
  Serial.println("No saved personality found. Using default personality.");
  savePersonalityToStorage(currentPersonality);
}

printPersonalityState(currentPersonality);

  Serial.print("Story mood model size: ");
  Serial.println(getStoryMoodModelSize());

  Serial.println("Initializing story mood model...");
  storyMoodModelAvailable = initStoryMoodModel();

  Serial.print("storyMoodModelAvailable = ");
  Serial.println(storyMoodModelAvailable ? "true" : "false");

  if (storyMoodModelAvailable) {
    Serial.println("Story mood model is ready.");
  } else {
    Serial.println("Story mood model init failed. Rule selector will be used.");
  }

  showStatus("Boot OK", "Starting WiFi...");

  connectWiFi();

  bool success = refreshWeatherAndStory();

  if (success) {
    showCurrentPage();
  }
}

void loop() {
  unsigned long now = millis();

  // 1. 不断电自动刷新天气和故事
  if (now - lastWeatherRefresh >= WEATHER_REFRESH_INTERVAL_MS) {
    Serial.println();
    Serial.println("Auto refreshing weather...");

    bool success = refreshWeatherAndStory();

    if (success) {
      Serial.println("Auto refresh finished.");
      
      showCurrentPage();
    } else {
      Serial.println("Auto refresh failed.");
    }
  }

  // 2. 页面轮播：现在默认关闭
  if (
    ENABLE_PAGE_CAROUSEL &&
    hasWeatherData &&
    now - lastPageSwitch >= PAGE_INTERVAL_MS
  ) {
    currentPage = 1 - currentPage;
    lastPageSwitch = now;
    showCurrentPage();
  }

  delay(20);
}

