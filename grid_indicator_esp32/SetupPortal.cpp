#include "SetupPortal.h"
#include "Settings.h"
#include "config.h"
#include "SetupPage.h"

#include <WiFi.h>
#include <stdlib.h>

namespace {

WiFiServer server(80);
SettingsManager settings;
bool* savedFlag = nullptr;

String decodeUrl(const String& value) {
  String result;
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); i++) {
    char c = value[i];

    if (c == '+') {
      result += ' ';
    } else if (c == '%' && i + 2 < value.length()) {
      char hex[3] = { value[i + 1], value[i + 2], '\0' };
      char* endPtr = nullptr;
      long decoded = strtol(hex, &endPtr, 16);

      if (endPtr != hex && *endPtr == '\0') {
        result += static_cast<char>(decoded);
        i += 2;
      } else {
        result += c;
      }
    } else {
      result += c;
    }
  }

  return result;
}

String getFormValue(const String& body, const String& key) {
  const String search = key + "=";
  int start = body.indexOf(search);

  while (start >= 0) {
    if (start == 0 || body[start - 1] == '&') {
      break;
    }
    start = body.indexOf(search, start + 1);
  }

  if (start < 0) {
    return "";
  }

  start += search.length();
  int end = body.indexOf('&', start);
  if (end < 0) {
    end = body.length();
  }

  return decodeUrl(body.substring(start, end));
}

void sendResponse(
  WiFiClient& client,
  const String& body,
  const char* status = "200 OK"
) {
  client.print("HTTP/1.1 ");
  client.println(status);
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();

  client.print(SETUP_PAGE_START);
  client.print(body);
  client.print(SETUP_PAGE_END);
}

void sendForm(WiFiClient& client) {
  String body =
    "<section class='card'>"
    "<h1>Grid Indicator</h1>"
    "<p class='subtitle'>Налаштування пристрою</p>"
    "<form method='POST' action='/save' accept-charset='UTF-8'>"
    "<label for='ssid'>Назва Wi-Fi (SSID)</label>"
    "<input id='ssid' name='ssid' maxlength='32' required "
    "autocomplete='off' autocapitalize='none'>"
    "<label for='password'>Пароль Wi-Fi</label>"
    "<input id='password' name='password' type='password' maxlength='63' "
    "autocomplete='new-password'>"
    "<label for='object'>Назва об'єкта</label>"
    "<input id='object' name='object' maxlength='48' "
    "placeholder='Наприклад, Дача' required>"
    "<button type='submit'>Зберегти та перезапустити</button>"
    "</form>"
    "<p class='note'>Після збереження пристрій перезапуститься "
    "і спробує підключитися до цієї мережі.</p>"
    "</section>";

  sendResponse(client, body);
}

void handleSave(WiFiClient& client, const String& payload) {
  Serial.println("Handling POST /save");

  String ssid = getFormValue(payload, "ssid");
  String password = getFormValue(payload, "password");
  String objectName = getFormValue(payload, "object");

  if (ssid.length() == 0 || objectName.length() == 0 ||
      ssid.length() > 32 || password.length() > 63 ||
      objectName.length() > 48) {
    sendResponse(
      client,
      "<section class='card'><h2>Помилка</h2>"
      "<p>Перевір SSID, пароль і назву об'єкта.</p>"
      "<p><a href='/'>Повернутися до форми</a></p></section>",
      "400 Bad Request"
    );
    return;
  }

  DeviceConfig config;
  config.ssid = ssid;
  config.password = password;
  config.objectName = objectName;

  if (!settings.saveConfig(config)) {
    sendResponse(
      client,
      "<section class='card'><h2>Помилка збереження</h2>"
      "<p>Не вдалося записати налаштування.</p>"
      "<p><a href='/'>Спробувати ще раз</a></p></section>",
      "500 Internal Server Error"
    );
    return;
  }

  Serial.println("Configuration saved successfully.");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Object: ");
  Serial.println(objectName);
  Serial.println("Password: [hidden]");

  // Перезапуск дозволено лише після успішного запису конфігурації.
  if (savedFlag != nullptr) {
    *savedFlag = true;
  }

  sendResponse(
    client,
    "<section class='card'><h1>Збережено!</h1>"
    "<p class='subtitle'>Налаштування успішно записані.</p>"
    "<p>Пристрій перезапуститься за мить і підключиться до Wi-Fi.</p>"
    "</section>"
  );
}

void handleClient() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  client.setTimeout(1500);

  unsigned long waitStarted = millis();

  while (client.connected() &&
         !client.available() &&
         millis() - waitStarted < 2000) {
    delay(1);
  }

  if (!client.available()) {
    client.stop();
    return;
  }

  String requestLine = client.readStringUntil('\n');
  requestLine.trim();

  if (requestLine.length() == 0) {
    client.stop();
    return;
  }

  Serial.print("HTTP request: [");
  Serial.print(requestLine);
  Serial.println("]");

  int contentLength = 0;

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
      break;
    }

    if (line.startsWith("Content-Length:")) {
      contentLength = line.substring(15).toInt();
    }
  }

  Serial.print("Content-Length: ");
  Serial.println(contentLength);

  // Розбираємо метод і шлях окремо, щоб запит із query string
  // або додатковим слешем не спричиняв хибний 404.
  int firstSpace = requestLine.indexOf(' ');
  int secondSpace = firstSpace >= 0
      ? requestLine.indexOf(' ', firstSpace + 1)
      : -1;

  String method;
  String path;
  if (firstSpace > 0 && secondSpace > firstSpace) {
    method = requestLine.substring(0, firstSpace);
    path = requestLine.substring(firstSpace + 1, secondSpace);
  }

  int queryPos = path.indexOf('?');
  if (queryPos >= 0) {
    path = path.substring(0, queryPos);
  }

  if (method == "GET" && (path == "/" || path == "/index.html")) {
    sendForm(client);
  } else if (method == "POST" && path == "/save") {
    if (contentLength <= 0 || contentLength > 512) {
      sendResponse(
        client,
        "<section class='card'><h2>Некоректний запит</h2>"
        "<p>Не вдалося прочитати дані форми.</p></section>",
        "400 Bad Request"
      );
    } else {
      String payload;
      payload.reserve(contentLength);
      unsigned long started = millis();

      while (payload.length() < static_cast<size_t>(contentLength) &&
             millis() - started < 3000) {
        while (client.available() &&
               payload.length() < static_cast<size_t>(contentLength)) {
          payload += static_cast<char>(client.read());
        }
        delay(1);
      }

      if (payload.length() != static_cast<size_t>(contentLength)) {
        Serial.println("ERROR: incomplete POST body");
        sendResponse(
          client,
          "<section class='card'><h2>Не вдалося прочитати форму</h2>"
          "<p>Спробуй ще раз.</p></section>",
          "400 Bad Request"
        );
      } else {
        handleSave(client, payload);
      }
    }
  } else {
    Serial.print("HTTP 404, method=");
    Serial.print(method);
    Serial.print(" path=");
    Serial.println(path);

    sendResponse(
      client,
      "<section class='card'><h2>Сторінку не знайдено</h2>"
      "<p>Повернися до <a href='/'>сторінки налаштувань</a>.</p>"
      "</section>",
      "404 Not Found"
    );
  }

  delay(2);
  client.stop();
}

}  // namespace

bool SetupPortalManager::begin() {
  if (running) {
    return true;
  }

  configSaved = false;
  savedFlag = &configSaved;

  if (!settings.begin()) {
    Serial.println("Settings initialization failed!");
    savedFlag = nullptr;
    return false;
  }

  WiFi.mode(WIFI_AP);
  delay(200);

  WiFi.disconnect(true, true);
WiFi.mode(WIFI_OFF);
delay(500);

WiFi.mode(WIFI_AP);
delay(500);

IPAddress apIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

if (!WiFi.softAPConfig(apIP, gateway, subnet)) {
  Serial.println("AP IP configuration failed!");
  return false;
}

Serial.println("Starting SoftAP...");

if (!WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASSWORD, 6, false, 4)) {
  Serial.println("Setup AP start failed!");
  return false;
}

delay(500);

Serial.print("AP IP: ");
Serial.println(WiFi.softAPIP());

Serial.print("AP MAC: ");
Serial.println(WiFi.softAPmacAddress());

Serial.print("Stations: ");
Serial.println(WiFi.softAPgetStationNum());

server.begin();

  server.begin();
  running = true;

  Serial.println("Setup Portal started!");
  Serial.print("AP SSID: ");
  Serial.println(SETUP_AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  return true;
}

void SetupPortalManager::update() {
  if (running) {
    handleClient();
  }
}

bool SetupPortalManager::configurationSaved() const {
  return configSaved;
}

void SetupPortalManager::stop() {
  if (!running) {
    return;
  }

  server.stop();
  WiFi.softAPdisconnect(true);
  running = false;
  savedFlag = nullptr;
}
