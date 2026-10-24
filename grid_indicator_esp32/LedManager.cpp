#include "LedManager.h"
#include "config.h"

namespace LedManager {
namespace {
constexpr uint32_t BLINK_INTERVAL_MS = 400;
State currentState = State::OFF;
bool ledOn = false;
uint32_t lastToggleMs = 0;

void writeLed(bool on) {
  ledOn = on;
  // On ESP32-C3 Super Mini the built-in LED is active LOW.
  digitalWrite(BLUE_LED_PIN, on ? LOW : HIGH);
}
}

void begin() {
  pinMode(BLUE_LED_PIN, OUTPUT);
  currentState = State::OFF;
  lastToggleMs = millis();
  writeLed(false);
}

void setState(State state) {
  if (currentState == state) return;
  currentState = state;
  lastToggleMs = millis();
  if (state == State::ON) writeLed(true);
  else if (state == State::OFF) writeLed(false);
  // For BLINK, start with LED off; update() will toggle it after interval.
}

void update() {
  if (currentState != State::BLINK) return;
  const uint32_t now = millis();
  if (now - lastToggleMs >= BLINK_INTERVAL_MS) {
    lastToggleMs = now;
    writeLed(!ledOn);
  }
}
} // namespace LedManager
