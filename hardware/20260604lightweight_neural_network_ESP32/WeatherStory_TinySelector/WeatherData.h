#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <Arduino.h>

// WeatherData = 外部天气数据。
// 它是系统的“感官输入”：温度、湿度、雨量、风速、昼夜等。
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

// WeatherTags = 对天气数据的语义解释。
// 它把数值变成故事系统更容易使用的标签。
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

#endif
