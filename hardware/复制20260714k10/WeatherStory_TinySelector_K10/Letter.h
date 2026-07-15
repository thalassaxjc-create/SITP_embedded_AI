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
