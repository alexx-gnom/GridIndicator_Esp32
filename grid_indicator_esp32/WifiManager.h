#pragma once
#include <Arduino.h>

namespace WifiManager {
void begin(const String& ssid, const String& password);
void update();
void reconnect(const String& ssid, const String& password);
void stop();
bool isConnected();
String ipAddress();
String ssid();
int rssi();
}
