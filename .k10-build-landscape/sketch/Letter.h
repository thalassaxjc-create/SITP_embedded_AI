#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\Letter.h"
#ifndef LETTER_H
#define LETTER_H

#include <Arduino.h>

struct Letter {
  String fromCity;
  String toCity;
  String title;
  String body;
  String id;
  bool unread;
};

#endif
