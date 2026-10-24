#include "History.h"
#include "Clock.h"
#include <Preferences.h>
#include <time.h>

namespace History {
namespace {
constexpr uint8_t DAYS = 7;
constexpr uint8_t SLOTS_PER_DAY = 96;
constexpr uint32_t SLOT_SECONDS = 900;
constexpr uint32_t SAVE_INTERVAL_MS = 60000;

struct Slot {
  uint32_t key; // YYYYMMDD * 100 + quarter-hour index
  uint16_t mainsSeconds;
  uint16_t outageSeconds;
  uint16_t unknownSeconds;
};

Preferences prefs;
Slot slots[DAYS][SLOTS_PER_DAY];
bool initialized = false;
uint32_t lastTickMs = 0;
uint32_t lastSaveMs = 0;
time_t lastEpoch = 0;
bool lastPower = false;

uint32_t dayKey(const tm& t) {
  return (uint32_t)(t.tm_year + 1900) * 10000UL +
         (uint32_t)(t.tm_mon + 1) * 100UL + (uint32_t)t.tm_mday;
}
bool parseDate(const String& s, uint32_t& key) {
  if (s.length() != 10 || s[4] != '-' || s[7] != '-') return false;
  int y=s.substring(0,4).toInt(), m=s.substring(5,7).toInt(), d=s.substring(8,10).toInt();
  if (y < 2024 || m < 1 || m > 12 || d < 1 || d > 31) return false;
  key = (uint32_t)y*10000UL + (uint32_t)m*100UL + (uint32_t)d;
  return true;
}
int dayIndex(uint32_t key) {
  for (int i=0;i<DAYS;i++) for (int q=0;q<SLOTS_PER_DAY;q++)
    if (slots[i][q].key / 100 == key) return i;
  return -1;
}
void save() {
  prefs.putBytes("slots", slots, sizeof(slots));
  prefs.putULong("lastEpoch", (uint32_t)lastEpoch);
  lastSaveMs = millis();
}
void ensureSlot(uint32_t date, uint8_t q) {
  Slot& s = slots[date % DAYS][q];
  uint32_t key = date * 100UL + q;
  if (s.key != key) {
    s.key = key; s.mainsSeconds = 0; s.outageSeconds = 0; s.unknownSeconds = 0;
  }
}
void addSeconds(Slot& s, bool power, uint32_t seconds) {
  if (power) s.mainsSeconds = (uint16_t)min(900UL, (uint32_t)s.mainsSeconds + seconds);
  else s.outageSeconds = (uint16_t)min(900UL, (uint32_t)s.outageSeconds + seconds);
}
String slotJson(const Slot& s) {
  String out = "{\"m\":" + String(s.mainsSeconds) + ",\"o\":" + String(s.outageSeconds) +
               ",\"u\":" + String(s.unknownSeconds) + "}";
  return out;
}
} // namespace

void begin() {
  prefs.begin("grid_history", false);
  memset(slots, 0, sizeof(slots));
  if (prefs.getBytesLength("slots") == sizeof(slots)) prefs.getBytes("slots", slots, sizeof(slots));
  lastEpoch = (time_t)prefs.getULong("lastEpoch", 0);
  lastTickMs = millis();
  lastSaveMs = millis();
  initialized = true;
}

void update(bool powerPresent) {
  if (!initialized) return;
  uint32_t nowMs = millis();
  if (lastTickMs == 0) lastTickMs = nowMs;
  uint32_t elapsed = (nowMs - lastTickMs) / 1000;
  if (elapsed == 0) return;
  lastTickMs += elapsed * 1000;
  if (!Clock::isTimeValid()) {
    // Without a trustworthy calendar time, we cannot assign the interval to a day.
    return;
  }
  time_t now = Clock::timestamp();
  if (lastEpoch != 0 && now > lastEpoch + (time_t)elapsed + 3) {
    // Time elapsed while the MCU was rebooted/offline is unknown; mark it grey
    // only when the gap is within the same calendar day and can be represented.
    time_t gapStart = lastEpoch;
    time_t gapEnd = now - elapsed;
    while (gapStart < gapEnd) {
      tm t; localtime_r(&gapStart, &t);
      uint32_t date = dayKey(t);
      uint8_t q = (uint8_t)(t.tm_hour * 4 + t.tm_min / 15);
      ensureSlot(date, q);
      uint32_t boundary = (uint32_t)(900 - ((t.tm_min % 15) * 60 + t.tm_sec));
      uint32_t part = (uint32_t)min((time_t)boundary, gapEnd-gapStart);
      Slot& s = slots[date % DAYS][q];
      s.unknownSeconds = (uint16_t)min(900UL, (uint32_t)s.unknownSeconds + part);
      gapStart += part;
    }
  }
  tm t; localtime_r(&now, &t);
  uint32_t date = dayKey(t);
  uint8_t q = (uint8_t)(t.tm_hour * 4 + t.tm_min / 15);
  ensureSlot(date, q);
  addSeconds(slots[date % DAYS][q], powerPresent, elapsed);
  lastPower = powerPresent;
  lastEpoch = now;
  if (nowMs - lastSaveMs >= SAVE_INTERVAL_MS) save();
}

String dayJson(const String& dateText) {
  if (!initialized || !Clock::isTimeValid()) return "{\"valid\":false}";
  uint32_t target;
  if (dateText.isEmpty()) target = 0;
  else if (!parseDate(dateText, target)) return "{\"valid\":false}";
  if (target == 0) { tm now; time_t ts=Clock::timestamp(); localtime_r(&ts,&now); target=dayKey(now); }
  String out = "{\"valid\":true,\"date\":\"";
  char dateBuf[11]; snprintf(dateBuf,sizeof(dateBuf),"%04lu-%02lu-%02lu",
    (unsigned long)(target/10000), (unsigned long)((target/100)%100), (unsigned long)(target%100));
  out += dateBuf; out += "\",\"slots\":[";
  uint32_t totalM=0,totalO=0,totalU=0;
  for (uint8_t q=0;q<SLOTS_PER_DAY;q++) {
    if (q) out += ",";
    Slot s{}; uint32_t key=target*100UL+q;
    Slot& stored=slots[target % DAYS][q];
    if (stored.key == key) s=stored;
    uint32_t known=(uint32_t)s.mainsSeconds+s.outageSeconds;
    s.unknownSeconds=(uint16_t)max((int32_t)0, (int32_t)900-(int32_t)known);
    totalM+=s.mainsSeconds; totalO+=s.outageSeconds; totalU+=s.unknownSeconds;
    out += slotJson(s);
  }
  out += "],\"mains\":" + String(totalM) + ",\"outage\":" + String(totalO) +
         ",\"unknown\":" + String(totalU) + "}";
  return out;
}
String availableDaysJson() {
  if (!Clock::isTimeValid()) return "{\"valid\":false}";
  time_t now=Clock::timestamp(); tm t; localtime_r(&now,&t);
  String out="{\"valid\":true,\"today\":\""+Clock::date()+"\",\"days\":[";
  for(int i=6;i>=0;i--) {
    time_t day=now-(time_t)i*86400; tm dt; localtime_r(&day,&dt);
    if(i!=6) out+=",";
    char b[11]; strftime(b,sizeof(b),"%Y-%m-%d",&dt);
    out+="\""; out+=b; out+="\"";
  }
  out+="]}"; return out;
}
} // namespace History
