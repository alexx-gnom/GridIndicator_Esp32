#pragma once

#include <Arduino.h>
#include <time.h>

namespace PowerMonitor {

enum class EventType : uint8_t {
  NONE,
  POWER_LOST,
  POWER_RESTORED
};

struct Event {
  EventType type = EventType::NONE;

  // millis() captured when the input first changed, before debounce completes.
  uint32_t uptimeMs = 0;

  // Unix timestamp captured at the same moment, or 0 if Clock is not valid.
  time_t timestamp = 0;
};

void begin();
void update();

bool hasMainsPower();
bool rawSignalHigh();
bool hasLastEvent();
Event lastEvent();
bool takeEvent(Event& event);

} // namespace PowerMonitor
