#pragma once
#include <Arduino.h>
#include <time.h>

namespace History {
void begin();
void update(bool powerPresent);
String dayJson(const String& dateText); // YYYY-MM-DD; empty means today
String availableDaysJson();
}
