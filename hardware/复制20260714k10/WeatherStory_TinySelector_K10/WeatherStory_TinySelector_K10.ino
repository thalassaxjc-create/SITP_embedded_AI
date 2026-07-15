/*
  Two-city Weather Letter for UNIHIKER K10.

  TinyML inference is active through StoryMoodModel.cpp. The rule selector is
  retained as a fallback. Personality learning and Preferences persistence are
  also retained. Motors are driven only through an external L298N module.
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
#include "StoryMoodModel.h"
#include "PersonalityLearning.h"
#include "PersonalityMemory.h"
#include "Letter.h"
#include "MotorController.h"

// The installed K10 library defines this mutex in initBoard.cpp but does not
// declare it in its public header. Minimal initialization must create it because
// initScreen() and canvas drawing share the SPI bus.
extern SemaphoreHandle_t xSPIlMutex;

// ===================== Central configuration / 集中配置 =====================

const char* WIFI_SSID = "神威无敌大将军核弹";
const char* WIFI_PASSWORD = "34362312";

struct CityConfig {
  const char* name;
  float latitude;
  float longitude;
};

const CityConfig CITY_A = {"上海", 31.2304f, 121.4737f};
const CityConfig CITY_B = {"台中", 24.1477f, 120.6736f};

const bool DEMO_MODE = false;
const unsigned long WEATHER_REFRESH_INTERVAL_MS = 10UL * 60UL * 1000UL;
const unsigned long WEATHER_RETRY_INTERVAL_MS = 60UL * 1000UL;
const unsigned long DEMO_REFRESH_INTERVAL_MS = 30UL * 1000UL;
const unsigned long PAGE_INTERVAL_MS = 5000UL;
const unsigned long ERROR_SCREEN_MS = 5000UL;
const unsigned long WIFI_RETRY_DELAY_MS = 500UL;
const int WIFI_RETRY_LIMIT = 20;
const int HTTP_TIMEOUT_MS = 8000;

// Installed K10 library: directions 1 and 3 are 320x240; 0 and 2 are 240x320.
// Direction 1 is the selected landscape orientation; direction 3 is its inverse.
constexpr uint8_t SCREEN_DIRECTION = 1;
constexpr int SCREEN_WIDTH = 320;
constexpr int SCREEN_HEIGHT = 240;
constexpr int MARGIN_X = 10;
constexpr int MARGIN_Y = 8;
constexpr int HEADER_HEIGHT = 34;
constexpr int FOOTER_HEIGHT = 22;
constexpr int BODY_TOP = HEADER_HEIGHT + 6;
constexpr int BODY_BOTTOM = SCREEN_HEIGHT - FOOTER_HEIGHT;
constexpr int BODY_LINE_HEIGHT = 20;
constexpr int BODY_TEXT_WIDTH = SCREEN_WIDTH - 2 * MARGIN_X;
constexpr int LETTER_LINES_PER_PAGE = (BODY_BOTTOM - BODY_TOP) / BODY_LINE_HEIGHT;
constexpr int MAX_LETTER_PAGES = 3;
constexpr int MAX_LETTER_LINES = LETTER_LINES_PER_PAGE * MAX_LETTER_PAGES;
constexpr int MAX_LETTER_BODY_BYTES = 1400;
constexpr int FONT_16_NON_ASCII_WIDTH = 12;

// The edge I2C pins are deliberately repurposed as GPIO for L298N channel B.
// setup() must not call k10.begin(), because its gesture task continuously uses I2C.
const uint8_t LEFT_MOTOR_IN1 = P0;
const uint8_t LEFT_MOTOR_IN2 = P1;
const uint8_t RIGHT_MOTOR_IN3 = SDA;
const uint8_t RIGHT_MOTOR_IN4 = SCL;
const bool LEFT_MOTOR_REVERSED = false;
const bool RIGHT_MOTOR_REVERSED = true;
const unsigned long MOTOR_INITIAL_STOP_MS = 100UL;
const unsigned long MOTOR_FORWARD_MS = 450UL;
const unsigned long MOTOR_DIRECTION_STOP_MS = 150UL;
const unsigned long MOTOR_BACKWARD_MS = 450UL;
const uint8_t MOTOR_REPEAT_COUNT = 2;

static_assert(FEATURE_COUNT == 14, "TinyML input must remain 14-dimensional");

// ===================== State =====================

enum AppState {
  APP_STARTUP,
  APP_WAITING,
  APP_NEW_LETTER,
  APP_READING,
  APP_ERROR_OFFLINE
};

struct CityWeatherCache {
  WeatherData data;
  bool hasData;
  bool lastRequestOk;
  String lastError;
};

UNIHIKER_K10 k10;
MotorController motorController(
  LEFT_MOTOR_IN1,
  LEFT_MOTOR_IN2,
  RIGHT_MOTOR_IN3,
  RIGHT_MOTOR_IN4,
  LEFT_MOTOR_REVERSED,
  RIGHT_MOTOR_REVERSED,
  MOTOR_INITIAL_STOP_MS,
  MOTOR_FORWARD_MS,
  MOTOR_DIRECTION_STOP_MS,
  MOTOR_BACKWARD_MS,
  MOTOR_REPEAT_COUNT
);

CityWeatherCache cityAWeather = {{}, false, false, "not requested"};
CityWeatherCache cityBWeather = {{}, false, false, "not requested"};
PersonalityState currentPersonality = makeDefaultPersonalityState();
StoryControl currentControl;
WeatherData currentCombinedWeather;
WeatherTags currentCombinedTags;
float currentFeatures[FEATURE_COUNT];
bool storyMoodModelAvailable = false;

Letter currentLetter;
bool hasLetter = false;
String lastLetterId = "";
String letterLines[MAX_LETTER_LINES];
int letterLineCount = 0;
int readingPage = 0;
bool letterTextTruncated = false;

AppState appState = APP_STARTUP;
String appError = "";
bool screenDirty = true;
bool lastRenderedWifiConnected = false;
bool demoRequested = false;
bool lastButtonA = false;
bool lastButtonB = false;
uint32_t demoCounter = 0;
unsigned long stateEnteredMs = 0;
unsigned long nextWeatherAttemptMs = 0;
unsigned long lastDemoLetterMs = 0;
unsigned long lastPageSwitchMs = 0;
unsigned long lastButtonPollMs = 0;

// ===================== Time and state helpers =====================

bool timeReached(unsigned long now, unsigned long deadline) {
  return (long)(now - deadline) >= 0;
}

void setAppState(AppState nextState, const String& error = "") {
  if (appState != nextState || appError != error) {
    appState = nextState;
    appError = error;
    stateEnteredMs = millis();
    screenDirty = true;
  }
}

// ===================== UTF-8-safe text layout =====================

int utf8SequenceLength(uint8_t lead) {
  if ((lead & 0x80) == 0) return 1;
  if ((lead & 0xE0) == 0xC0) return 2;
  if ((lead & 0xF0) == 0xE0) return 3;
  if ((lead & 0xF8) == 0xF0) return 4;
  return 1;
}

int glyphWidthPx(uint8_t leadByte) {
  if ((leadByte & 0x80) == 0) {
    uint8_t ascii = leadByte;
    if (ascii >= 0x20 && ascii <= 0x7E) {
      int measured = ASCII_GetInterval(ascii, ASCII_12_A);
      return measured > 0 ? measured : 8;
    }
  }
  return FONT_16_NON_ASCII_WIDTH;
}

int textWidthPx(const String& text) {
  int width = 0;
  for (int i = 0; i < text.length();) {
    uint8_t lead = (uint8_t)text.charAt(i);
    int byteCount = utf8SequenceLength(lead);
    if (i + byteCount > text.length()) byteCount = 1;
    width += glyphWidthPx(lead);
    i += byteCount;
  }
  return width;
}

String truncateToPixelWidth(const String& text, int maxWidthPx) {
  if (textWidthPx(text) <= maxWidthPx) return text;
  const String ellipsis = "...";
  int available = max(0, maxWidthPx - textWidthPx(ellipsis));
  String result;
  result.reserve(min((unsigned int)text.length(), 64U));
  int width = 0;
  for (int i = 0; i < text.length();) {
    uint8_t lead = (uint8_t)text.charAt(i);
    int byteCount = utf8SequenceLength(lead);
    if (i + byteCount > text.length()) byteCount = 1;
    int charWidth = glyphWidthPx(lead);
    if (width + charWidth > available) break;
    for (int b = 0; b < byteCount; b++) result += text.charAt(i + b);
    width += charWidth;
    i += byteCount;
  }
  return result + ellipsis;
}

String truncateUtf8Bytes(const String& text, int maxBytes, bool& truncated) {
  if (text.length() <= maxBytes) return text;
  int end = maxBytes;
  while (end > 0 && (((uint8_t)text.charAt(end) & 0xC0) == 0x80)) end--;
  truncated = true;
  return text.substring(0, end) + "...";
}

bool appendWrappedLine(String lines[], int maxLines, int& count, String value) {
  if (count >= maxLines) return false;
  value.trim();
  lines[count++] = value;
  return true;
}

int wrapUtf8Text(
  const String& text,
  String lines[],
  int maxLines,
  int maxWidthPx,
  bool& truncated
) {
  int count = 0;
  String buffer = "";
  buffer.reserve(64);
  int bufferWidth = 0;
  int lastSpaceByte = -1;
  truncated = false;

  for (int i = 0; i < text.length();) {
    uint8_t lead = (uint8_t)text.charAt(i);
    int byteCount = utf8SequenceLength(lead);
    if (i + byteCount > text.length()) byteCount = 1;
    if (byteCount == 1 && lead == '\r') {
      i++;
      continue;
    }
    if (byteCount == 1 && lead == '\n') {
      i++;
      if (!appendWrappedLine(lines, maxLines, count, buffer)) {
        truncated = true;
        break;
      }
      buffer = "";
      bufferWidth = 0;
      lastSpaceByte = -1;
      continue;
    }

    int charWidth = glyphWidthPx(lead);
    while (buffer.length() > 0 && bufferWidth + charWidth > maxWidthPx) {
      if (lastSpaceByte >= 0) {
        String completed = buffer.substring(0, lastSpaceByte);
        if (!appendWrappedLine(lines, maxLines, count, completed)) {
          truncated = true;
          break;
        }
        buffer = buffer.substring(lastSpaceByte + 1);
        buffer.trim();
        bufferWidth = textWidthPx(buffer);
        lastSpaceByte = buffer.lastIndexOf(' ');
      } else {
        if (!appendWrappedLine(lines, maxLines, count, buffer)) {
          truncated = true;
          break;
        }
        buffer = "";
        bufferWidth = 0;
        lastSpaceByte = -1;
      }
    }
    if (truncated) break;

    for (int b = 0; b < byteCount; b++) buffer += text.charAt(i + b);
    bufferWidth += charWidth;
    if (byteCount == 1 && lead == ' ') lastSpaceByte = buffer.length() - 1;
    i += byteCount;
  }

  if (count < maxLines && buffer.length() > 0) {
    appendWrappedLine(lines, maxLines, count, buffer);
  } else if (buffer.length() > 0) {
    truncated = true;
  }

  if (truncated && count > 0) {
    lines[count - 1] = truncateToPixelWidth(lines[count - 1] + "...", maxWidthPx);
  }
  return count;
}

void clearScreen() {
  // Canvas rectangle does not call lv_task_handler; the completed page is
  // presented once by updateCanvas(), avoiding an intermediate blank frame.
  k10.canvas->canvasRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xFFFFFF, 0xFFFFFF, true);
}

// ===================== Weather API and interpretation =====================

String buildWeatherUrl(const CityConfig& city) {
  String url = "https://api.open-meteo.com/v1/forecast?latitude=";
  url += String(city.latitude, 6);
  url += "&longitude=";
  url += String(city.longitude, 6);
  url += "&current=temperature_2m,relative_humidity_2m,precipitation,rain,weather_code,wind_speed_10m,is_day";
  url += "&timezone=auto";
  return url;
}

bool connectWiFiBounded() {
  if (WiFi.status() == WL_CONNECTED) return true;

  motorController.stop();
  Serial.println("Wi-Fi: connecting (password is never logged)");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  for (int retry = 0; retry < WIFI_RETRY_LIMIT; retry++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wi-Fi: connected");
      return true;
    }
    delay(WIFI_RETRY_DELAY_MS);
    yield();
  }

  Serial.println("Wi-Fi: connection timed out");
  return false;
}

bool fetchCityWeather(
  const CityConfig& city,
  WeatherData& result,
  String& errorMessage
) {
  Serial.println();
  Serial.print("Weather request city: ");
  Serial.println(city.name);

  if (WiFi.status() != WL_CONNECTED) {
    errorMessage = "Wi-Fi offline";
    Serial.println("Request skipped: Wi-Fi offline");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);

  String url = buildWeatherUrl(city);
  if (!http.begin(client, url)) {
    errorMessage = "HTTP begin failed";
    Serial.println(errorMessage);
    return false;
  }

  int httpCode = http.GET();
  Serial.print("HTTP status: ");
  Serial.println(httpCode);

  if (httpCode != HTTP_CODE_OK) {
    errorMessage = String("HTTP ") + String(httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(4096);
  DeserializationError jsonError = deserializeJson(doc, payload);
  if (jsonError) {
    errorMessage = String("JSON: ") + jsonError.c_str();
    Serial.print("Parse failed: ");
    Serial.println(errorMessage);
    return false;
  }

  JsonObject current = doc["current"];
  if (current.isNull()) {
    errorMessage = "missing current object";
    Serial.println("Parse failed: missing current object");
    return false;
  }

  WeatherData parsed;
  parsed.time = current["time"].as<String>();
  parsed.temperature = current["temperature_2m"] | -999.0f;
  parsed.humidity = current["relative_humidity_2m"] | -1;
  parsed.precipitation = current["precipitation"] | -1.0f;
  parsed.rain = current["rain"] | -1.0f;
  parsed.weatherCode = current["weather_code"] | -1;
  parsed.windSpeed = current["wind_speed_10m"] | -1.0f;
  parsed.isDay = current["is_day"] | -1;

  if (parsed.time.length() == 0 || parsed.temperature < -100.0f ||
      parsed.humidity < 0 || parsed.weatherCode < 0 || parsed.isDay < 0) {
    errorMessage = "invalid weather fields";
    Serial.println("Parse failed: invalid weather fields");
    return false;
  }

  result = parsed;
  errorMessage = "";
  Serial.print("Parsed: time=");
  Serial.print(result.time);
  Serial.print(", temp=");
  Serial.print(result.temperature, 1);
  Serial.print("C, humidity=");
  Serial.print(result.humidity);
  Serial.print("%, code=");
  Serial.print(result.weatherCode);
  Serial.print(", wind=");
  Serial.println(result.windSpeed, 1);
  return true;
}

bool isRainCode(int code) {
  return (code >= 51 && code <= 67) || (code >= 80 && code <= 82);
}

bool isSnowCode(int code) {
  return (code >= 71 && code <= 77) || (code >= 85 && code <= 86);
}

bool isCloudCode(int code) {
  return (code >= 1 && code <= 3) || code == 45 || code == 48;
}

bool isStormCode(int code) {
  return code >= 95 && code <= 99;
}

int weatherSeverity(int code) {
  if (isStormCode(code)) return 6;
  if (isSnowCode(code)) return 5;
  if (code == 65 || code == 67 || code == 82) return 4;
  if (isRainCode(code)) return 3;
  if (code == 45 || code == 48) return 2;
  if (isCloudCode(code)) return 1;
  return 0;
}

WeatherTags makeTags(const WeatherData& w) {
  WeatherTags t;
  t.hot = w.temperature >= 30.0f;
  t.cold = w.temperature <= 5.0f;
  t.cool = w.temperature > 5.0f && w.temperature <= 18.0f;
  t.mild = w.temperature > 18.0f && w.temperature < 30.0f;
  t.humid = w.humidity >= 75;
  t.dry = w.humidity <= 35;
  t.rainy = w.rain > 0.0f || w.precipitation > 0.0f || isRainCode(w.weatherCode);
  t.snowy = isSnowCode(w.weatherCode);
  t.cloudy = isCloudCode(w.weatherCode);
  t.stormy = isStormCode(w.weatherCode);
  t.windy = w.windSpeed >= 20.0f;
  t.daytime = w.isDay == 1;
  t.night = w.isDay == 0;

  t.tempMood = t.hot ? "hot" : (t.cold ? "cold" : (t.cool ? "cool" : "mild"));
  t.airMood = t.humid ? "humid" : (t.dry ? "dry" : "normal");
  if (t.stormy) t.skyMood = "stormy";
  else if (t.snowy) t.skyMood = "snowy";
  else if (t.rainy) t.skyMood = "rainy";
  else if (t.cloudy) t.skyMood = "cloudy";
  else t.skyMood = "clear";
  t.windMood = t.windy ? "windy" : "calm";
  t.lightMood = t.night ? "night" : "day";
  return t;
}

WeatherData combineWeatherData(const WeatherData& a, const WeatherData& b) {
  WeatherData combined;
  combined.time = b.time;
  combined.temperature = (a.temperature + b.temperature) * 0.5f;
  combined.humidity = (int)roundf(((float)a.humidity + (float)b.humidity) * 0.5f);
  combined.precipitation = max(a.precipitation, b.precipitation);
  combined.rain = max(a.rain, b.rain);
  combined.windSpeed = max(a.windSpeed, b.windSpeed);
  combined.weatherCode = weatherSeverity(a.weatherCode) >= weatherSeverity(b.weatherCode)
    ? a.weatherCode : b.weatherCode;
  combined.isDay = b.isDay;
  return combined;
}

WeatherTags combineWeatherTags(
  const WeatherData& a,
  const WeatherData& b,
  const WeatherData& combined
) {
  WeatherTags aTags = makeTags(a);
  WeatherTags bTags = makeTags(b);
  WeatherTags tags = makeTags(combined);
  tags.rainy = aTags.rainy || bTags.rainy;
  tags.snowy = aTags.snowy || bTags.snowy;
  tags.stormy = aTags.stormy || bTags.stormy;
  tags.cloudy = aTags.cloudy || bTags.cloudy;
  tags.windy = aTags.windy || bTags.windy;
  if (tags.stormy) tags.skyMood = "stormy";
  else if (tags.snowy) tags.skyMood = "snowy";
  else if (tags.rainy) tags.skyMood = "rainy";
  else if (tags.cloudy) tags.skyMood = "cloudy";
  else tags.skyMood = "clear";
  tags.windMood = tags.windy ? "windy" : "calm";
  return tags;
}

// ===================== Letter generation =====================

String weatherSummary(const CityConfig& city, const WeatherData& weather) {
  WeatherTags tags = makeTags(weather);
  String summary = city.name;
  summary += ": ";
  summary += String(weather.temperature, 1);
  summary += "C, ";
  summary += tags.skyMood;
  summary += ", ";
  summary += String(weather.humidity);
  summary += "% RH";
  return summary;
}

uint32_t fnv1a(const String& text) {
  uint32_t hash = 2166136261UL;
  for (int i = 0; i < text.length(); i++) {
    hash ^= (uint8_t)text.charAt(i);
    hash *= 16777619UL;
  }
  return hash;
}

String makeLetterId(const Letter& letter, uint32_t demoNonce) {
  String source = cityAWeather.data.time + "|" + cityBWeather.data.time;
  source += "|" + String(cityAWeather.data.temperature, 2);
  source += "|" + String(cityBWeather.data.temperature, 2);
  source += "|" + String(cityAWeather.data.weatherCode);
  source += "|" + String(cityBWeather.data.weatherCode);
  source += "|" + letter.body;
  if (demoNonce > 0) source += "|demo:" + String(demoNonce);
  return String(fnv1a(source), HEX);
}

void rebuildLetterPages() {
  bool lengthTruncated = false;
  currentLetter.body = truncateUtf8Bytes(
    currentLetter.body,
    MAX_LETTER_BODY_BYTES,
    lengthTruncated
  );
  bool layoutTruncated = false;
  letterLineCount = wrapUtf8Text(
    currentLetter.body,
    letterLines,
    MAX_LETTER_LINES,
    BODY_TEXT_WIDTH,
    layoutTruncated
  );
  letterTextTruncated = lengthTruncated || layoutTruncated;
  if (letterLineCount == 0) {
    letterLines[0] = "(empty letter)";
    letterLineCount = 1;
  }
  readingPage = 0;
  lastPageSwitchMs = millis();
}

void printFeatureVector(const float features[FEATURE_COUNT]) {
  Serial.println("Feature vector (14 inputs):");
  for (int i = 0; i < FEATURE_COUNT; i++) {
    Serial.print(featureName(i));
    Serial.print("=");
    Serial.println(features[i], 3);
  }
}

bool generateLetterFromCachedWeather(bool updatePersonality, uint32_t demoNonce) {
  if (!cityAWeather.hasData || !cityBWeather.hasData) return false;

  currentCombinedWeather = combineWeatherData(cityAWeather.data, cityBWeather.data);
  currentCombinedTags = combineWeatherTags(
    cityAWeather.data,
    cityBWeather.data,
    currentCombinedWeather
  );

  float features[FEATURE_COUNT];
  makeFeatures(currentCombinedWeather, currentCombinedTags, currentPersonality, features);
  if (updatePersonality) {
    updatePersonalityFromWeatherFeatures(currentPersonality, features);
    maybeSavePersonalityToStorage(currentPersonality);
    makeFeatures(currentCombinedWeather, currentCombinedTags, currentPersonality, features);
  }

  for (int i = 0; i < FEATURE_COUNT; i++) currentFeatures[i] = features[i];
  printFeatureVector(features);

  StoryControl ruleControl = selectStoryControl(features);
  StoryControl selectedControl = ruleControl;
  float modelProbabilities[4] = {0, 0, 0, 0};
  int modelMood = storyMoodModelAvailable
    ? runStoryMoodModel(features, modelProbabilities) : -1;

  if (modelMood >= 0) {
    selectedControl = makeStoryControlFromMood(modelMood);
    Serial.print("TinyML mood (4 outputs): ");
    Serial.println(modelMoodName(modelMood));
  } else {
    Serial.print("Rule fallback mood: ");
    Serial.println(moodName(ruleControl.mood_id));
  }
  currentControl = selectedControl;

  WeatherTags cityATags = makeTags(cityAWeather.data);
  WeatherTags cityBTags = makeTags(cityBWeather.data);
  String story = generateStory(
    CITY_A.name,
    cityAWeather.data,
    cityATags,
    CITY_B.name,
    cityBWeather.data,
    cityBTags,
    selectedControl
  );
  Letter candidate;
  candidate.fromCity = CITY_A.name;
  candidate.toCity = CITY_B.name;
  candidate.title = String("Weather Letter - ") + moodName(selectedControl.mood_id);
  candidate.body = String("From: ") + candidate.fromCity + "\n";
  candidate.body += String("To: ") + candidate.toCity + "\n";
  candidate.body += weatherSummary(CITY_A, cityAWeather.data) + "\n";
  candidate.body += weatherSummary(CITY_B, cityBWeather.data) + "\n";
  candidate.body += String("Mood: ") + moodName(selectedControl.mood_id) + "\n\n";
  candidate.body += story;
  candidate.id = makeLetterId(candidate, demoNonce);
  candidate.unread = true;

  Serial.print("Generated letter id: ");
  Serial.println(candidate.id);
  if (candidate.id == lastLetterId) {
    Serial.println("Letter id unchanged; motor notification suppressed.");
    if (hasLetter) setAppState(APP_READING);
    return false;
  }

  currentLetter = candidate;
  hasLetter = true;
  lastLetterId = candidate.id;
  rebuildLetterPages();
  setAppState(APP_NEW_LETTER);
  motorController.triggerNewLetterMotion();
  Serial.println("New letter accepted; motor motion triggered once.");
  return true;
}

bool refreshBothCities() {
  motorController.stop();
  setAppState(APP_WAITING);
  if (screenDirty) renderScreen();

  if (!connectWiFiBounded()) {
    setAppState(APP_ERROR_OFFLINE, "Wi-Fi connection timed out");
    return false;
  }

  WeatherData freshA;
  WeatherData freshB;
  String errorA;
  String errorB;
  bool okA = fetchCityWeather(CITY_A, freshA, errorA);
  if (okA) {
    cityAWeather.data = freshA;
    cityAWeather.hasData = true;
  }
  cityAWeather.lastRequestOk = okA;
  cityAWeather.lastError = errorA;

  bool okB = fetchCityWeather(CITY_B, freshB, errorB);
  if (okB) {
    cityBWeather.data = freshB;
    cityBWeather.hasData = true;
  }
  cityBWeather.lastRequestOk = okB;
  cityBWeather.lastError = errorB;

  if (okA && okB) {
    generateLetterFromCachedWeather(true, 0);
    return true;
  }

  String error = "Weather failed: ";
  if (!okA) error += String(CITY_A.name) + " " + errorA;
  if (!okA && !okB) error += "; ";
  if (!okB) error += String(CITY_B.name) + " " + errorB;
  setAppState(APP_ERROR_OFFLINE, error);
  Serial.println(error);
  Serial.println("Successful city caches and the previous letter were preserved.");
  return false;
}

// ===================== Screen states =====================

void drawText16(const String& text, int x, int y, uint32_t color) {
  k10.canvas->canvasText(
    text,
    x,
    y,
    color,
    k10.canvas->eCNAndENFont16,
    50,
    false
  );
}

void drawCenteredText16(const String& text, int y, uint32_t color) {
  String fitted = truncateToPixelWidth(text, SCREEN_WIDTH - 2 * MARGIN_X);
  int x = max(MARGIN_X, (SCREEN_WIDTH - textWidthPx(fitted)) / 2);
  drawText16(fitted, x, y, color);
}

void drawHeader(
  const String& title,
  const String& route = "",
  const String& page = ""
) {
  k10.canvas->canvasRectangle(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, 0xDDEEFF, 0xDDEEFF, true);
  int pageWidth = textWidthPx(page);
  int pageX = SCREEN_WIDTH - MARGIN_X - pageWidth;
  String fittedTitle = truncateToPixelWidth(title, 125);
  drawText16(fittedTitle, MARGIN_X, MARGIN_Y, 0x003366);
  if (route.length() > 0) {
    int routeX = 142;
    int routeWidth = max(20, pageX - routeX - 8);
    drawText16(truncateToPixelWidth(route, routeWidth), routeX, MARGIN_Y, 0x334455);
  }
  if (page.length() > 0) drawText16(page, pageX, MARGIN_Y, 0x334455);
  k10.canvas->canvasLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, HEADER_HEIGHT - 1, 0x99AABB);
}

void drawFooter() {
  k10.canvas->canvasLine(0, BODY_BOTTOM, SCREEN_WIDTH, BODY_BOTTOM, 0x99AABB);
  String weather = "No weather cache";
  if (cityAWeather.hasData && cityBWeather.hasData) {
    weather = String(CITY_A.name) + " " + String(cityAWeather.data.temperature, 1) + "C";
    weather += " | ";
    weather += String(CITY_B.name) + " " + String(cityBWeather.data.temperature, 1) + "C";
  }
  String network = WiFi.status() == WL_CONNECTED ? "WiFi OK" : "Offline";
  int networkWidth = textWidthPx(network);
  int networkX = SCREEN_WIDTH - MARGIN_X - networkWidth;
  drawText16(truncateToPixelWidth(weather, networkX - MARGIN_X - 8), MARGIN_X, BODY_BOTTOM + 3, 0x445566);
  drawText16(network, networkX, BODY_BOTTOM + 3, 0x445566);
}

void drawEnvelope(int x, int y, int width, int height, uint32_t color) {
  k10.canvas->canvasRectangle(x, y, width, height, color, 0xFFFFFF, false);
  k10.canvas->canvasLine(x, y, x + width / 2, y + height / 2, color);
  k10.canvas->canvasLine(x + width, y, x + width / 2, y + height / 2, color);
}

void drawStartupScreen() {
  clearScreen();
  drawHeader("STARTUP");
  drawCenteredText16("Weather Letter", 88, 0x003366);
  drawCenteredText16("Initializing TinyML and motors...", 122, 0x445566);
  drawFooter();
  k10.canvas->updateCanvas();
}

void drawWaitingScreen() {
  clearScreen();
  drawHeader("WAITING");
  String cityA = truncateToPixelWidth(CITY_A.name, 86);
  String cityB = truncateToPixelWidth(CITY_B.name, 86);
  drawText16(cityA, 28, 78, 0x003366);
  drawText16(cityB, SCREEN_WIDTH - 28 - textWidthPx(cityB), 78, 0x003366);
  drawEnvelope(137, 62, 46, 32, 0x557799);
  for (int x = 105; x < 132; x += 8) k10.canvas->canvasLine(x, 78, x + 4, 78, 0x99AABB);
  for (int x = 188; x < 215; x += 8) k10.canvas->canvasLine(x, 78, x + 4, 78, 0x99AABB);
  drawCenteredText16("Waiting for a weather letter...", 132, 0x445566);
  drawFooter();
  k10.canvas->updateCanvas();
}

void drawNewLetterScreen(const Letter& letter) {
  clearScreen();
  drawHeader("NEW LETTER", String(letter.fromCity) + " -> " + letter.toCity);
  constexpr int splitX = 216;
  drawText16(String("From: ") + truncateToPixelWidth(letter.fromCity, 145), MARGIN_X, 52, 0x223344);
  drawText16(String("To: ") + truncateToPixelWidth(letter.toCity, 160), MARGIN_X, 78, 0x223344);
  String weather = String(CITY_A.name) + " " + String(cityAWeather.data.temperature, 1) + "C";
  weather += String(" | ") + CITY_B.name + " " + String(cityBWeather.data.temperature, 1) + "C";
  drawText16(truncateToPixelWidth(weather, splitX - 2 * MARGIN_X), MARGIN_X, 110, 0x445566);
  k10.canvas->canvasLine(splitX, BODY_TOP + 4, splitX, BODY_BOTTOM - 8, 0xCCD5DD);
  drawEnvelope(238, 54, 60, 40, 0xCC6600);
  String mood = moodName(currentControl.mood_id);
  int rightCenter = (splitX + SCREEN_WIDTH) / 2;
  String fittedMood = truncateToPixelWidth(mood, SCREEN_WIDTH - splitX - 12);
  drawText16(fittedMood, rightCenter - textWidthPx(fittedMood) / 2, 116, 0x663300);
  String motion = motorController.isActive() ? "MOVING" : "READY";
  drawText16(motion, rightCenter - textWidthPx(motion) / 2, 148, 0xCC5500);
  drawFooter();
  k10.canvas->updateCanvas();
}

void drawReadingScreen(const Letter& letter, int pageIndex) {
  int totalPages = max(1, (letterLineCount + LETTER_LINES_PER_PAGE - 1) / LETTER_LINES_PER_PAGE);
  totalPages = min(totalPages, MAX_LETTER_PAGES);
  if (pageIndex >= totalPages) pageIndex = 0;
  clearScreen();
  drawHeader(
    letter.title,
    String(letter.fromCity) + " -> " + letter.toCity,
    String(pageIndex + 1) + "/" + String(totalPages)
  );
  int first = pageIndex * LETTER_LINES_PER_PAGE;
  for (int line = 0; line < LETTER_LINES_PER_PAGE; line++) {
    int index = first + line;
    if (index >= letterLineCount) break;
    drawText16(letterLines[index], MARGIN_X, BODY_TOP + line * BODY_LINE_HEIGHT, 0x111111);
  }
  drawFooter();
  k10.canvas->updateCanvas();
}

void drawOfflineScreen(const String& reason) {
  clearScreen();
  drawHeader("ERROR / OFFLINE");
  String errorType = reason.indexOf("Wi-Fi") >= 0 ? "Wi-Fi unavailable" : "Weather service error";
  drawCenteredText16(errorType, 82, 0xAA2222);
  drawCenteredText16(hasLetter ? "Cached letter is still available" : "No cached letter yet", 116, 0x445566);
  drawCenteredText16(hasLetter ? "Press B to read cache" : "Automatic retry is scheduled", 148, 0x445566);
  drawFooter();
  k10.canvas->updateCanvas();
}

void renderScreen() {
  switch (appState) {
    case APP_STARTUP:
      drawStartupScreen();
      break;
    case APP_WAITING:
      drawWaitingScreen();
      break;
    case APP_NEW_LETTER:
      drawNewLetterScreen(currentLetter);
      break;
    case APP_READING:
      drawReadingScreen(currentLetter, readingPage);
      break;
    case APP_ERROR_OFFLINE:
      drawOfflineScreen(appError);
      break;
  }
  screenDirty = false;
  lastRenderedWifiConnected = WiFi.status() == WL_CONNECTED;
}

// ===================== Input and loop updates =====================

void handleButtons(unsigned long now) {
  if (now - lastButtonPollMs < 80UL) return;
  lastButtonPollMs = now;
  bool buttonA = k10.buttonA->isPressed();
  bool buttonB = k10.buttonB->isPressed();

  if (buttonA && !lastButtonA && DEMO_MODE) {
    demoRequested = true;
    Serial.println("Button A: demo letter requested.");
  }

  if (buttonB && !lastButtonB) {
    if (appState == APP_READING && hasLetter) {
      int pages = max(1, (letterLineCount + LETTER_LINES_PER_PAGE - 1) / LETTER_LINES_PER_PAGE);
      readingPage = (readingPage + 1) % pages;
      lastPageSwitchMs = now;
      screenDirty = true;
    } else if (appState == APP_ERROR_OFFLINE && hasLetter) {
      setAppState(APP_READING);
    }
  }

  lastButtonA = buttonA;
  lastButtonB = buttonB;
}

void handleSerialCommands() {
  while (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'r' || command == 'R') {
      motorController.stop();
      resetPersonalityStorage(currentPersonality);
    } else if ((command == 'd' || command == 'D') && DEMO_MODE) {
      demoRequested = true;
    }
  }
}

void updateLetterState(unsigned long now) {
  if (appState == APP_NEW_LETTER && !motorController.isActive()) {
    currentLetter.unread = false;
    setAppState(APP_READING);
    lastPageSwitchMs = now;
  }

  if (appState == APP_READING && hasLetter && now - lastPageSwitchMs >= PAGE_INTERVAL_MS) {
    int pages = max(1, (letterLineCount + LETTER_LINES_PER_PAGE - 1) / LETTER_LINES_PER_PAGE);
    readingPage = (readingPage + 1) % pages;
    lastPageSwitchMs = now;
    screenDirty = true;
  }

  if (appState == APP_ERROR_OFFLINE && hasLetter && now - stateEnteredMs >= ERROR_SCREEN_MS) {
    setAppState(APP_READING);
  }
}

// ===================== Arduino lifecycle =====================

void setup() {
  Serial.begin(115200);
  delay(1000);
  randomSeed((uint32_t)esp_random() ^ micros());

  // Minimal K10 initialization keeps the display/buttons but does not start Wire
  // or the background gesture task, leaving SDA/SCL available as motor GPIO.
  init_board();
  digital_write(eLCD_BLK, 0);
  xSPIlMutex = xSemaphoreCreateMutex();
  k10.buttonA = new Button(eP5_KeyA);
  k10.buttonB = new Button(eP11_KeyB);
  GT_Font_Init();
  k10.initScreen(SCREEN_DIRECTION);
  k10.creatCanvas();
  k10.canvas->canvasSetLineWidth(2);
  k10.setScreenBackground(0xFFFFFF);
  motorController.begin();
  renderScreen();

  Serial.println("=== Two-city Weather Letter / K10 ===");
  Serial.println("Screen: direction=1, canvas=320x240 landscape (library-verified)");
  Serial.println("Motor pins: P0->IN1, P1->IN2, SDA->IN3, SCL->IN4");
  Serial.println("No LED/RGB code is enabled.");

  if (loadPersonalityFromStorage(currentPersonality)) {
    Serial.println("Loaded saved personality.");
  } else {
    Serial.println("No saved personality; saving defaults.");
    savePersonalityToStorage(currentPersonality);
  }

  Serial.print("Story mood model bytes: ");
  Serial.println(getStoryMoodModelSize());
  storyMoodModelAvailable = initStoryMoodModel();
  Serial.print("TinyML model ready: ");
  Serial.println(storyMoodModelAvailable ? "yes" : "no; rule fallback active");

  setAppState(APP_WAITING);
  nextWeatherAttemptMs = millis();
  lastDemoLetterMs = millis();
}

void loop() {
  unsigned long now = millis();

  if ((WiFi.status() == WL_CONNECTED) != lastRenderedWifiConnected) {
    screenDirty = true;
  }

  handleSerialCommands();
  handleButtons(now);
  motorController.update();
  updateLetterState(now);

  if (demoRequested) {
    demoRequested = false;
    motorController.stop();
    if (cityAWeather.hasData && cityBWeather.hasData) {
      demoCounter++;
      generateLetterFromCachedWeather(false, demoCounter);
      lastDemoLetterMs = now;
    } else {
      Serial.println("Demo cache unavailable; scheduling a real weather refresh.");
      nextWeatherAttemptMs = now;
    }
  }

  if (DEMO_MODE && cityAWeather.hasData && cityBWeather.hasData &&
      now - lastDemoLetterMs >= DEMO_REFRESH_INTERVAL_MS) {
    demoCounter++;
    generateLetterFromCachedWeather(false, demoCounter);
    lastDemoLetterMs = now;
  }

  if (timeReached(now, nextWeatherAttemptMs)) {
    bool refreshed = refreshBothCities();
    now = millis();
    nextWeatherAttemptMs = now + (refreshed
      ? WEATHER_REFRESH_INTERVAL_MS : WEATHER_RETRY_INTERVAL_MS);
  }

  if (screenDirty) renderScreen();
  delay(5);
  yield();
}
