
#include <WiFi.h>

#include "config.h"
#include "Settings.h"
#include "Clock.h"
#include "SetupPortal.h"
#include "WebServer.h"


SettingsManager settings;
SetupPortalManager setupPortal;
WebServerManager webServer;
DeviceConfig config;

enum class DeviceMode {
  STARTING,
  WORKING,
  SETUP
};

DeviceMode mode = DeviceMode::STARTING;

// GPIO8: вбудований LED активний LOW
constexpr unsigned long BOOT_HOLD_MS = 5000;
constexpr unsigned long LED_BLINK_MS = 400;
constexpr unsigned long WIFI_RETRY_MS = 10000;

unsigned long bootPressStart = 0;
unsigned long lastLedToggle = 0;
unsigned long lastWifiAttempt = 0;

bool bootPressTiming = false;
bool bootLongPressHandled = false;
bool ledState = false;
bool wifiAttemptStarted = false;

void ledOn() {
  digitalWrite(BLUE_LED_PIN, LOW);
}

void ledOff() {
  digitalWrite(BLUE_LED_PIN, HIGH);
}

void updateLedBlink() {
  if (millis() - lastLedToggle >= LED_BLINK_MS) {
    lastLedToggle = millis();
    ledState = !ledState;
    digitalWrite(BLUE_LED_PIN, ledState ? LOW : HIGH);
  }
}

// Повертає true лише один раз після утримання BOOT 5 секунд.
bool checkBootLongPress() {
  if (digitalRead(SETUP_PIN) == LOW) {
    if (!bootPressTiming) {
      bootPressTiming = true;
      bootPressStart = millis();
    }

    if (!bootLongPressHandled &&
        millis() - bootPressStart >= BOOT_HOLD_MS) {
      bootLongPressHandled = true;
      return true;
    }
  } else {
    bootPressTiming = false;
    bootLongPressHandled = false;
  }

  return false;
}

void startSetupMode() {
  webServer.stop();
  Serial.println();
  Serial.println("=== SETUP MODE ===");

  // Спочатку вимикаємо Wi-Fi станції.
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(200);

  mode = DeviceMode::SETUP;
  ledState = false;
  lastLedToggle = millis();

  if (!setupPortal.begin()) {
    Serial.println("ERROR: Setup Portal failed!");
    // Залишаємо індикацію помилки блиманням.
    return;
  }

  Serial.println("Setup mode ready.");
}

void startWorkingMode() {
  Serial.println();
  Serial.println("=== WORKING MODE ===");
  Serial.print("Object: ");
  Serial.println(config.objectName);
  Serial.print("Wi-Fi SSID: ");
  Serial.println(config.ssid);

  setupPortal.stop();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  ledOn();
  mode = DeviceMode::WORKING;

  Serial.println("Connecting to saved Wi-Fi...");
  WiFi.begin(config.ssid.c_str(), config.password.c_str());

  lastWifiAttempt = millis();
  wifiAttemptStarted = true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(BLUE_LED_PIN, OUTPUT);
  ledOff();

  pinMode(SETUP_PIN, INPUT_PULLUP);

  Serial.println();
  Serial.println("Grid Indicator starting...");

  if (!settings.begin()) {
    Serial.println("ERROR: Settings initialization failed!");
    ledOn();
    return;
  }

  // Якщо BOOT затиснутий під час старту, одразу відкриваємо
  // налаштування. Не потрібно чекати 5 секунд після завантаження.
  if (digitalRead(SETUP_PIN) == LOW) {
    startSetupMode();
    return;
  }

  if (!settings.loadConfig(config) || config.ssid.isEmpty()) {
    Serial.println("No saved Wi-Fi configuration.");
    startSetupMode();
    return;
  }

  startWorkingMode();

  Clock::begin();
}

void loop() {

  Clock::update();

  static uint32_t lastClockPrint = 0;

Clock::update();

if (Clock::isTimeValid() &&
    millis() - lastClockPrint >= 5000) {
  lastClockPrint = millis();
  Serial.println(Clock::dateTime());
}

  if (mode == DeviceMode::SETUP) {
    setupPortal.update();
    updateLedBlink();

    if (setupPortal.configurationSaved()) {
      Serial.println("Configuration saved successfully.");
      Serial.println("Restarting in 1 second...");
      delay(1000);
      ESP.restart();
    }

    return;
  }

  if (mode != DeviceMode::WORKING) {
    return;
  }

  if (checkBootLongPress()) {
    Serial.println("BOOT held for 5 seconds.");
    startSetupMode();
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!webServer.isRunning()) {
      webServer.begin();
    }
    webServer.update(config.objectName);
    ledOff();

    if (wifiAttemptStarted) {
      wifiAttemptStarted = false;
      Serial.println("Wi-Fi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    }

    return;
  }

  // Якщо Wi-Fi тимчасово втрачено, сторінка стане доступною після відновлення.
  // LED світиться, поки Wi-Fi не підключений.
  ledOn();

  // Повторна спроба підключення, якщо з'єднання не відновилося.
  if (millis() - lastWifiAttempt >= WIFI_RETRY_MS) {
    lastWifiAttempt = millis();

    Serial.println("Wi-Fi not connected. Retrying...");
    WiFi.disconnect();
    WiFi.begin(config.ssid.c_str(), config.password.c_str());
    wifiAttemptStarted = true;
  }
}
