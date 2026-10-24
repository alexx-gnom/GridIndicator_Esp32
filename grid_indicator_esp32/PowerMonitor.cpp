#include "PowerMonitor.h"
#include "Clock.h"
#include "config.h"

namespace PowerMonitor {
namespace {
constexpr uint32_t DEBOUNCE_MS = 500;

bool initialized = false;
bool stablePowerPresent = true;
bool candidatePowerPresent = true;
bool candidateActive = false;
uint32_t candidateSinceMs = 0;
time_t candidateTimestamp = 0;
uint32_t candidateUptimeMs = 0;

Event pendingEvent;
Event mostRecentEvent;

bool readPowerPresent() {
  return digitalRead(GRID_SIGNAL_PIN) == HIGH;
}
} // namespace

void begin() {
  pinMode(GRID_SIGNAL_PIN, INPUT);

  // Establish the initial state without creating a false transition event.
  stablePowerPresent = readPowerPresent();
  candidatePowerPresent = stablePowerPresent;
  candidateActive = false;
  candidateSinceMs = millis();
  candidateTimestamp = 0;
  candidateUptimeMs = candidateSinceMs;
  pendingEvent = Event{};
  mostRecentEvent = Event{};
  initialized = true;
}

void update() {
  if (!initialized) return;

  const bool reading = readPowerPresent();
  const uint32_t nowMs = millis();

  if (reading != stablePowerPresent) {
    if (!candidateActive || reading != candidatePowerPresent) {
      candidateActive = true;
      candidatePowerPresent = reading;
      candidateSinceMs = nowMs;
      candidateUptimeMs = nowMs;
      candidateTimestamp = Clock::isTimeValid() ? Clock::timestamp() : 0;
    }

    if (nowMs - candidateSinceMs >= DEBOUNCE_MS) {
      stablePowerPresent = candidatePowerPresent;
      candidateActive = false;

      // Keep the most recent event until the consumer retrieves it.
      pendingEvent.type = stablePowerPresent
          ? EventType::POWER_RESTORED
          : EventType::POWER_LOST;
      pendingEvent.uptimeMs = candidateUptimeMs;
      pendingEvent.timestamp = candidateTimestamp;
      mostRecentEvent = pendingEvent;
    }
  } else {
    // The input returned to the stable state before debounce elapsed.
    candidateActive = false;
  }
}

bool hasMainsPower() {
  return initialized && stablePowerPresent;
}

bool rawSignalHigh() {
  return initialized && digitalRead(GRID_SIGNAL_PIN) == HIGH;
}

bool hasLastEvent() {
  return mostRecentEvent.type != EventType::NONE;
}

Event lastEvent() {
  return mostRecentEvent;
}

bool takeEvent(Event& event) {
  if (pendingEvent.type == EventType::NONE) return false;
  event = pendingEvent;
  pendingEvent = Event{};
  return true;
}

} // namespace PowerMonitor
