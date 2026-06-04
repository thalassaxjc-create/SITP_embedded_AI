/*
  Weather Story for Generic ESP32
  普通 ESP32 可用的天气小故事版本

  This version does NOT depend on UNIHIKER K10.
  这个版本不依赖 UNIHIKER K10。

  Output method / 输出方式:
  - Serial Monitor at 115200 baud
  - 使用 Arduino IDE 的 Serial Monitor 查看输出，波特率 115200

  Required library / 所需库:
  - ArduinoJson
  - ESP32 board support package

  What it does / 它做什么:
  1. Connects to Wi-Fi / 连接 Wi-Fi
  2. Fetches current weather from Open-Meteo / 从 Open-Meteo 获取当前天气
  3. Converts numeric weather data into mood tags / 把天气数值转换成语义标签
  4. Builds a weather sentence and a tiny story / 生成天气句子和小故事
  5. Prints two alternating pages to Serial Monitor / 在串口中轮流显示两页内容
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <esp_system.h>

// ===================== User settings / 用户设置 =====================

const char* WIFI_SSID = "enter the WIFI name";
const char* WIFI_PASSWORD = "enter your password";

// Current example location: Montreal / 当前示例地点：蒙特利尔
const float LATITUDE = 45.508888;
const float LONGITUDE = -73.561668;

// How often to switch between Page 1 and Page 2 / 两页切换间隔
const unsigned long PAGE_INTERVAL_MS = 5000;

// How often to refresh weather data / 天气刷新间隔
const unsigned long WEATHER_REFRESH_INTERVAL_MS = 10UL * 60UL * 1000UL;

// Wi-Fi connection retry count / Wi-Fi 连接重试次数
const int WIFI_RETRY_LIMIT = 30;

// ===================== Data structures / 数据结构 =====================

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
};

// ===================== Global state / 全局状态 =====================

WeatherData currentWeather;
WeatherTags currentTags;

String currentSentence = "";
String currentStory = "";

bool hasWeatherData = false;
int currentPage = 0;

unsigned long lastPageSwitch = 0;
unsigned long lastWeatherRefresh = 0;

// ===================== Utility printing / 输出工具 =====================

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
    Prototype setting / 原型设置:
    setInsecure() skips HTTPS certificate verification.
    setInsecure() 会跳过 HTTPS 证书校验。
    This is convenient for classroom/prototype use.
    If this becomes a production project, use a root certificate instead.
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

// ===================== Story text banks / 故事文本库 =====================

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

const char* coolImages[] = {
  "The cool air made every sound softer.",
  "The city seemed to breathe more slowly.",
  "A thin chill passed through the quiet street."
};

const char* cloudyImages[] = {
  "Clouds covered the sky like folded paper.",
  "The moon was hidden behind a grey curtain.",
  "The sky kept its thoughts to itself."
};

const char* rainyImages[] = {
  "Rain tapped gently on the ground.",
  "Small puddles opened their eyes in the street.",
  "The rain carried tiny messages from the sky."
};

const char* windyImages[] = {
  "The wind ran past and forgot where it was going.",
  "Leaves turned over like pages of a small book.",
  "The air moved as if looking for someone."
};

const char* calmImages[] = {
  "Nothing hurried.",
  "Even the shadows stayed still.",
  "The silence rested on the rooftops."
};

const char* nightEndings[] = {
  "Under the night sky, the city became a tiny story.",
  "By midnight, even the clouds seemed to fall asleep.",
  "The night kept the little secret safe."
};

const char* dayEndings[] = {
  "In the daylight, everything looked almost new.",
  "The day carried the story down the street.",
  "By afternoon, the sky had almost forgotten it."
};

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

String buildTinyStory(const WeatherTags& t) {
  String story = "";

  story += pickText(characters, 5);
  story += " ";
  story += pickText(places, 5);
  story += ".";
  story += "\n";

  if (t.rainy) {
    story += pickText(rainyImages, 3);
  } else if (t.cloudy) {
    story += pickText(cloudyImages, 3);
  } else if (t.cool) {
    story += pickText(coolImages, 3);
  } else {
    story += "The weather left a soft mark on the street.";
  }

  story += "\n";

  if (t.windy) {
    story += pickText(windyImages, 3);
  } else {
    story += pickText(calmImages, 3);
  }

  story += " ";

  if (t.night) {
    story += pickText(nightEndings, 3);
  } else {
    story += pickText(dayEndings, 3);
  }

  return story;
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

// ===================== Page display via Serial / 串口分页显示 =====================

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

void printStoryPage(const WeatherData& w, const WeatherTags& t, const String& story) {
  printDivider();
  Serial.println("Weather Story [2/2]");
  Serial.println("Tiny Story");
  Serial.println();

  Serial.print("Mood: ");
  Serial.print(t.tempMood);
  Serial.print(", ");
  Serial.print(t.skyMood);
  Serial.print(", ");
  Serial.println(t.windMood);

  Serial.println();
  Serial.println(story);
}

void showCurrentPage() {
  if (!hasWeatherData) {
    showStatus("No weather data");
    return;
  }

  if (currentPage == 0) {
    printWeatherPage(currentWeather, currentTags, currentSentence);
  } else {
    printStoryPage(currentWeather, currentTags, currentStory);
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
  String sentence = buildWeatherSentence(tags);
  String story = buildTinyStory(tags);

  currentWeather = weather;
  currentTags = tags;
  currentSentence = sentence;
  currentStory = story;

  hasWeatherData = true;
  currentPage = 0;

  printWeather(weather);
  printTags(tags);

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

  randomSeed((uint32_t)esp_random());

  Serial.println();
  Serial.println("=== Weather Story for Generic ESP32 ===");

  showStatus("Boot OK", "Starting WiFi...");

  connectWiFi();

  bool success = refreshWeatherAndStory();

  if (success) {
    showCurrentPage();
  }
}

void loop() {
  unsigned long now = millis();

  if (now - lastWeatherRefresh >= WEATHER_REFRESH_INTERVAL_MS) {
    refreshWeatherAndStory();
    showCurrentPage();
  }

  if (!hasWeatherData) {
    delay(1000);
    return;
  }

  if (now - lastPageSwitch >= PAGE_INTERVAL_MS) {
    currentPage = 1 - currentPage;
    lastPageSwitch = now;
    showCurrentPage();
  }

  delay(20);
}
