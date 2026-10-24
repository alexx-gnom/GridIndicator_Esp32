#include "WifiManager.h"
#include <WiFi.h>

namespace WifiManager {
namespace {
bool started = false;
String savedSsid;
String savedPassword;
}

void begin(const String& ssid, const String& password) {
  savedSsid = ssid;
  savedPassword = password;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
  started = true;
}

void update() {
  if (!started) return;
  // ESP32 Wi-Fi stack handles connection progress asynchronously.
}

void reconnect(const String& ssid, const String& password) {
  savedSsid = ssid;
  savedPassword = password;
  WiFi.disconnect();
  WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
  started = true;
}

void stop() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  started = false;
}

bool isConnected() { return WiFi.status() == WL_CONNECTED; }
String ipAddress() { return isConnected() ? WiFi.localIP().toString() : String(); }
String ssid() { return isConnected() ? WiFi.SSID() : String(); }
int rssi() { return isConnected() ? WiFi.RSSI() : 0; }
} // namespace WifiManager
