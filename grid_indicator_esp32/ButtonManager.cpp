#include "ButtonManager.h"
#include "config.h"

namespace ButtonManager {
namespace {
constexpr uint32_t LONG_PRESS_MS = 5000;
bool pressedAtStartup = false;
bool timingPress = false;
bool longPressHandled = false;
uint32_t pressStartedAt = 0;
}

void begin() {
  pinMode(SETUP_PIN, INPUT_PULLUP);
  pressedAtStartup = digitalRead(SETUP_PIN) == LOW;
  timingPress = false;
  longPressHandled = false;
  pressStartedAt = 0;
}

bool bootPressedAtStartup() { return pressedAtStartup; }

bool takeSetupRequest() {
  if (digitalRead(SETUP_PIN) == LOW) {
    if (!timingPress) {
      timingPress = true;
      pressStartedAt = millis();
    }
    if (!longPressHandled && millis() - pressStartedAt >= LONG_PRESS_MS) {
      longPressHandled = true;
      return true;
    }
  } else {
    timingPress = false;
    longPressHandled = false;
  }
  return false;
}
} // namespace ButtonManager
