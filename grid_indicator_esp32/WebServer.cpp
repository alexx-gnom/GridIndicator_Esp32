#include "WebServer.h"
#include "IndicatorPage.h"
#include "Clock.h"
#include "PowerMonitor.h"

#include <WiFi.h>

namespace {
WiFiServer server(80);

String escapeHtml(const String& value) {
  String out;
  out.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    switch (value[i]) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '\"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += value[i]; break;
    }
  }
  return out;
}

String wifiQuality(int rssi) {
  if (rssi >= -55) return "Відмінний";
  if (rssi >= -67) return "Добрий";
  if (rssi >= -75) return "Задовільний";
  return "Слабкий";
}

String jsonEscape(const String& value) {
  String out;
  out.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    char c = value[i];
    if (c == '"' || c == '\\') { out += '\\'; out += c; }
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else if ((uint8_t)c >= 0x20) out += c;
  }
  return out;
}

String clockStatusText() {
  switch (Clock::status()) {
    case Clock::SyncStatus::NOT_STARTED: return "не запущено";
    case Clock::SyncStatus::WAITING_FOR_WIFI: return "очікування Wi-Fi";
    case Clock::SyncStatus::SYNCING: return "синхронізація NTP";
    case Clock::SyncStatus::SYNCED: return "OK";
    case Clock::SyncStatus::ERROR: return "помилка NTP; повторна спроба";
  }
  return "невідомо";
}

String eventDescription() {
  if (!PowerMonitor::hasLastEvent()) return "";
  const PowerMonitor::Event e = PowerMonitor::lastEvent();
  String result = e.type == PowerMonitor::EventType::POWER_LOST
      ? "Зникнення живлення" : "Відновлення живлення";
  if (e.timestamp != 0) {
    time_t t = e.timestamp;
    struct tm localInfo;
    if (localtime_r(&t, &localInfo) != nullptr) {
      char buffer[20];
      if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localInfo) > 0) {
        result += " · ";
        result += buffer;
      }
    }
  } else {
    result += " · час недоступний";
  }
  return result;
}

void sendJsonStatus(WiFiClient& client, const String& objectName) {
  const bool timeValid = Clock::isTimeValid();
  const int rssi = WiFi.RSSI();
  String body;
  body.reserve(600);
  body = "{\"clockValid\":" + String(timeValid ? "true" : "false");
  body += ",\"clock\":\"" + jsonEscape(timeValid ? Clock::dateTime() : String("")) + "\"";
  body += ",\"clockStatus\":\"" + jsonEscape(clockStatusText()) + "\"";
  body += ",\"uptimeMs\":" + String(millis());
  body += ",\"powerPresent\":" + String(PowerMonitor::hasMainsPower() ? "true" : "false");
  body += ",\"rawHigh\":" + String(PowerMonitor::rawSignalHigh() ? "true" : "false");
  body += ",\"objectName\":\"" + jsonEscape(objectName) + "\"";
  body += ",\"ssid\":\"" + jsonEscape(WiFi.SSID()) + "\"";
  body += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  body += ",\"rssi\":" + String(rssi);
  body += ",\"wifiQuality\":\"" + wifiQuality(rssi) + "\"";
  body += ",\"lastEvent\":\"" + jsonEscape(eventDescription()) + "\"}";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json; charset=utf-8");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.print(body);
}

void sendPage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.print(INDICATOR_PAGE_START);
  client.print(INDICATOR_PAGE_END);
}
}

bool WebServerManager::begin() {
  if (running) return true;
  if (WiFi.status() != WL_CONNECTED) return false;
  server.begin();
  running = true;
  Serial.println("Web status page started.");
  Serial.print("Web page: http://");
  Serial.println(WiFi.localIP());
  return true;
}

void WebServerManager::update(const String& objectName) {
  if (!running) return;

  WiFiClient client = server.available();
  if (!client) return;
  client.setTimeout(1000);
  const unsigned long started = millis();
  while (client.connected() && !client.available() && millis() - started < 1000) delay(1);
  if (!client.available()) { client.stop(); return; }

  String requestLine = client.readStringUntil('\n');
  requestLine.trim();
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;
  }

  if (requestLine.startsWith("GET /api/status ")) {
    sendJsonStatus(client, objectName);
  } else if (requestLine.startsWith("GET / ") || requestLine.startsWith("GET /?")) {
    sendPage(client);
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain; charset=utf-8");
    client.println("Connection: close");
    client.println();
    client.println("Not found");
  }
  delay(1);
  client.stop();
}

void WebServerManager::stop() {
  if (!running) return;
  server.stop();
  running = false;
}

bool WebServerManager::isRunning() const { return running; }
