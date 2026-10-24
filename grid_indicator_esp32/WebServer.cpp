#include "WebServer.h"
#include "IndicatorPage.h"
#include "Clock.h"
#include "PowerMonitor.h"
#include "History.h"
#include "Telegram.h"

#include <WiFi.h>
#include <stdlib.h>

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
  int contentLength = 0;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;
    if (line.startsWith("Content-Length:")) contentLength = line.substring(15).toInt();
  }
  String requestBody;
  if (requestLine.startsWith("POST ") && contentLength > 0 && contentLength <= 512) {
    uint32_t bodyStarted = millis();
    while (requestBody.length() < contentLength && client.connected() &&
           millis() - bodyStarted < 1000) {
      while (client.available() && requestBody.length() < contentLength) {
        requestBody += (char)client.read();
      }
      delay(1);
    }
  }

  if (requestLine.startsWith("GET /api/status ")) {
    sendJsonStatus(client, objectName);
  } else if (requestLine.startsWith("GET /api/telegram/chats ")) {
    String body = Telegram::chatIdsJson();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json; charset=utf-8");
    client.println("Cache-Control: no-store");
    client.println("Connection: close");
    client.println();
    client.print(body);
  } else if (requestLine.startsWith("POST /api/telegram/chats ")) {
    auto getValue = [&requestBody](const String& key) -> String {
      String needle = key + "=";
      int start = requestBody.indexOf(needle);
      if (start < 0) return "";
      if (start > 0 && requestBody[start - 1] != '&') return "";
      start += needle.length();
      int end = requestBody.indexOf('&', start);
      if (end < 0) end = requestBody.length();
      String value = requestBody.substring(start, end);
      value.replace("+", " ");
      String decoded;
      for (int i=0; i<(int)value.length(); ++i) {
        if (value[i]=='%' && i+2<(int)value.length()) {
          char hex[3]={value[i+1],value[i+2],0};
          char* endPtr=nullptr;
          long ch=strtol(hex,&endPtr,16);
          if (endPtr && *endPtr==0) { decoded+=(char)ch; i+=2; continue; }
        }
        decoded+=value[i];
      }
      return decoded;
    };
    String ids[4] = {getValue("chat0"),getValue("chat1"),getValue("chat2"),getValue("chat3")};
    bool ok = contentLength > 0 && requestBody.length() == contentLength &&
              Telegram::saveChatIds(ids);
    client.println(ok ? "HTTP/1.1 200 OK" : "HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json; charset=utf-8");
    client.println("Cache-Control: no-store");
    client.println("Connection: close");
    client.println();
    client.print(ok ? "{\"ok\":true}" : "{\"ok\":false}");
  } else if (requestLine.startsWith("GET /api/history ")) {
    String dateText;
    int q = requestLine.indexOf("date=");
    if (q >= 0) {
      int start = q + 5;
      int end = requestLine.indexOf(' ', start);
      if (end < 0) end = requestLine.length();
      dateText = requestLine.substring(start, end);
      int amp = dateText.indexOf('&');
      if (amp >= 0) dateText = dateText.substring(0, amp);
    }
    String body = History::dayJson(dateText);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json; charset=utf-8");
    client.println("Cache-Control: no-store");
    client.println("Connection: close");
    client.println();
    client.print(body);
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
