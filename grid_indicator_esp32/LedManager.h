#pragma once
#include <Arduino.h>

namespace LedManager {
enum class State : uint8_t {
  OFF,
  ON,
  BLINK
};
void begin();
void setState(State state);
void update();
}
