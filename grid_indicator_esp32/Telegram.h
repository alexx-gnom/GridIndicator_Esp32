#pragma once
#include <Arduino.h>
#include <time.h>

namespace Telegram {
void begin(const String& objectName);
void update(const String& objectName);
void onPowerEvent(bool powerPresent, time_t eventTime);
String chatIdsJson();
bool saveChatIds(const String ids[4]);
}
