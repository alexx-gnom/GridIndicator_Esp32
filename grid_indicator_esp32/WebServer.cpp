#include "WebServer.h"
#include "IndicatorPage.h"

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
      case '"': out += "&quot;"; break;
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

void sendPage(WiFiClient& client, const String& objectName) {
  String body;
  body.reserve(900);
  body += "<div class='status'><span class='dot'></span><div>"
          "<div class='status-title'>Wi-Fi підключено</div>"
          "<div class='status-detail'>Пристрій працює в локальній мережі</div>"
          "</div></div><div class='grid'>";

  body += "<div class='metric'><div class='metric-label'>Об'єкт</div><div class='metric-value'>";
  body += escapeHtml(objectName.length() ? objectName : String("Не задано"));
  body += "</div></div>";

  body += "<div class='metric'><div class='metric-label'>Wi-Fi мережа</div><div class='metric-value'>";
  body += escapeHtml(WiFi.SSID());
  body += "</div></div>";

  body += "<div class='metric'><div class='metric-label'>IP-адреса</div><div class='metric-value'>";
  body += WiFi.localIP().toString();
  body += "</div></div>";

  const int rssi = WiFi.RSSI();
  body += "<div class='metric'><div class='metric-label'>Рівень сигналу</div><div class='metric-value'>";
  body += String(rssi);
  body += " dBm · ";
  body += wifiQuality(rssi);
  body += "</div></div></div>";
  body += "<p class='note'>Моніторинг живлення та історію відключень буде додано на наступному етапі.</p>";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.print(INDICATOR_PAGE_START);
  client.print(body);
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
  while (client.connected() && !client.available() && millis() - started < 1000) {
    delay(1);
  }

  if (!client.available()) {
    client.stop();
    return;
  }

  String requestLine = client.readStringUntil('\n');
  requestLine.trim();

  // Прочитати заголовки, щоб перейти до обробки запиту.
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;
  }

  if (requestLine.startsWith("GET / ") || requestLine.startsWith("GET /?")) {
    sendPage(client, objectName);
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

bool WebServerManager::isRunning() const {
  return running;
}
