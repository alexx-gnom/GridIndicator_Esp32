#include "Telegram.h"
#include "Clock.h"
#include "History.h"
#include "PowerMonitor.h"
#include "secrets.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <time.h>
#include <string.h>

namespace Telegram {
namespace {
constexpr char HOST[] = "api.telegram.org";
constexpr uint16_t PORT = 443;
constexpr uint8_t MAX_CHATS = 1;
constexpr uint8_t QUEUE_CAPACITY = 8;
constexpr uint16_t MESSAGE_SIZE = 320;
constexpr uint32_t RETRY_MS = 60000;
constexpr uint32_t SEND_GAP_MS = 1500;

struct QueuedMessage { String text; uint8_t chatIndex; };
QueuedMessage queue[QUEUE_CAPACITY];
uint8_t qHead = 0, qTail = 0, qCount = 0;
String chatIds[MAX_CHATS];
Preferences prefs;
String activeObjectName;
bool initialized = false;
bool startupQueued = false;
bool lastPowerKnown = false;
bool lastPower = false;
int lastReportYear = -1;
int lastReportYday = -1;
uint32_t lastSendMs = 0;
uint32_t retryAfterMs = 0;
bool sendAttempted = false;

bool configuredToken() {
  return strlen(TELEGRAM_BOT_TOKEN) > 0 &&
         strcmp(TELEGRAM_BOT_TOKEN, "PASTE_BOT_TOKEN_HERE") != 0 &&
         strcmp(TELEGRAM_BOT_TOKEN, "***********") != 0;
}
bool hasChat() {
  for (uint8_t i=0; i<MAX_CHATS; ++i) if (chatIds[i].length()) return true;
  return false;
}
String jsonEscape(const String& in) {
  String out; out.reserve(in.length()+8);
  for (size_t i=0; i<in.length(); ++i) {
    char c=in[i];
    if (c=='"' || c=='\\') { out+='\\'; out+=c; }
    else if (c=='\n') out+="\\n";
    else if (c=='\r') out+="\\r";
    else if ((uint8_t)c>=0x20) out+=c;
  }
  return out;
}
String urlEncode(const String& in) {
  const char* hex="0123456789ABCDEF";
  String out; out.reserve(in.length()*3);
  for (size_t i=0;i<in.length();++i) {
    uint8_t c=(uint8_t)in[i];
    if ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~') out+=(char)c;
    else { out+='%'; out+=hex[c>>4]; out+=hex[c&15]; }
  }
  return out;
}
bool enqueue(const String& message) {
  if (qCount >= QUEUE_CAPACITY) return false;
  // One queued item per recipient; keep messages ordered.
  for (uint8_t i=0;i<MAX_CHATS;++i) {
    if (!chatIds[i].length()) continue;
    if (qCount >= QUEUE_CAPACITY) return false;
    queue[qTail] = {message, i};
    qTail = (qTail+1)%QUEUE_CAPACITY; ++qCount;
  }
  return true;
}
String timestampSuffix(time_t t=0, bool allowCurrent=true) {
  if (!t) {
    if (!allowCurrent || !Clock::isTimeValid()) return " Час недоступний.";
    t=Clock::timestamp();
  }
  struct tm ti;
  if (!localtime_r(&t,&ti)) return " Час недоступний.";
  char b[32];
  strftime(b,sizeof(b)," %Y-%m-%d. %H:%M.",&ti);
  return String(b);
}
String prefix(const String& objectName) {
  // The configured object name is always the literal beginning of every message.
  return objectName + " ";
}
bool sendOne(const String& chatId, const String& message) {
  if (!configuredToken()) {
    Serial.println("[TG] ERROR: bot token is missing or still a placeholder.");
    return false;
  }
  if (!chatId.length()) {
    Serial.println("[TG] ERROR: Chat ID is empty.");
    return false;
  }
  if (WiFi.status()!=WL_CONNECTED) {
    Serial.println("[TG] Waiting for Wi-Fi.");
    return false;
  }
  Serial.printf("[TG] Connecting to Telegram (message length: %u)\n", static_cast<unsigned>(message.length()));
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(2500);
  uint32_t connectStarted = millis();
  if (!client.connect(HOST,PORT)) {
    Serial.printf("[TG] ERROR: TLS connection failed after %lu ms.\n", static_cast<unsigned long>(millis() - connectStarted));
    client.stop();
    return false;
  }
  Serial.printf("[TG] TLS connected in %lu ms.\n", static_cast<unsigned long>(millis() - connectStarted));
  String payload = "{\"chat_id\":\"" + jsonEscape(chatId) + "\",\"text\":\"" + jsonEscape(message) + "\"}";
  client.print("POST /bot"); client.print(TELEGRAM_BOT_TOKEN); client.println("/sendMessage HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Connection: close");
  client.println("Content-Type: application/json; charset=utf-8");
  client.print("Content-Length: "); client.println(payload.length());
  client.println(); client.print(payload);
  uint32_t start=millis();
  while (client.connected() && !client.available() && millis()-start<2500) delay(1);
  String status;
  if (client.available()) { status=client.readStringUntil('\n'); status.trim(); }
  String responseBody;
  uint32_t bodyStarted = millis();
  while (client.connected() && millis() - bodyStarted < 500) {
    while (client.available()) {
      char c = static_cast<char>(client.read());
      if (responseBody.length() < 220) responseBody += c;
    }
    delay(1);
  }
  client.stop();
  const bool ok = status.startsWith("HTTP/1.1 200") || status.startsWith("HTTP/1.0 200");
  if (ok) Serial.printf("[TG] Send OK. %s\n", status.c_str());
  else {
    Serial.printf("[TG] ERROR: HTTP response: %s\n", status.length() ? status.c_str() : "(no status line)");
    if (responseBody.length()) Serial.printf("[TG] API response: %s\n", responseBody.c_str());
  }
  return ok;
}
String formatDuration(uint32_t seconds) {
  uint32_t h=seconds/3600, m=(seconds%3600)/60;
  return String(h)+" год "+String(m)+" хв";
}
void queueStartup() {
  if (startupQueued || !hasChat() || !configuredToken() || !Clock::isTimeValid()) return;
  String msg=prefix(activeObjectName);
  msg += "ℹ️ Запуск індикатора мережі. ";
  msg += PowerMonitor::hasMainsPower() ? "Мережа є." : "Мережі немає.";
  msg += timestampSuffix();
  if (enqueue(msg)) { startupQueued=true; Serial.println("[TG] Startup notification queued."); }
  else Serial.println("[TG] ERROR: could not queue startup notification (queue full).");
}
void queueDailyReport() {
  if (!Clock::isTimeValid() || !hasChat() || !configuredToken()) return;
  time_t now=Clock::timestamp(); struct tm local;
  if (!localtime_r(&now,&local) || local.tm_hour!=0 || local.tm_min!=0) return;
  // Report the previous local calendar day.
  local.tm_mday -= 1;
  local.tm_hour=12; local.tm_min=0; local.tm_sec=0;
  time_t previous=mktime(&local); struct tm day;
  if (previous==(time_t)-1 || !localtime_r(&previous,&day)) return;
  int year=day.tm_year+1900, yday=day.tm_yday;
  if (year==lastReportYear && yday==lastReportYday) return;
  char date[16]; strftime(date,sizeof(date),"%Y-%m-%d",&day);
  String stats=History::dayJson(String(date));
  // Extract the three totals from the small JSON response.
  auto getNum = [&stats](const char* key)->uint32_t {
    String needle=String("\"")+key+"\":"; int p=stats.indexOf(needle);
    if (p<0) return 0; p+=needle.length(); return (uint32_t)stats.substring(p).toInt();
  };
  String msg=prefix(activeObjectName);
  msg += "📊 Звіт за добу "; msg += date; msg += ".\n";
  msg += "🟩 Світло було — "; msg += formatDuration(getNum("mains")); msg += ".\n";
  msg += "🟧 Світло було відсутнє — "; msg += formatDuration(getNum("outage")); msg += ".\n";
  msg += "⬜ Оффлайн — "; msg += formatDuration(getNum("unknown")); msg += ".";
  msg += timestampSuffix(now);
  if (enqueue(msg)) { lastReportYear=year; lastReportYday=yday; Serial.println("[TG] Daily report queued."); }
  else Serial.println("[TG] ERROR: could not queue daily report (queue full).");
}
} // namespace

void begin(const String& objectName) {
  activeObjectName=objectName;
  prefs.begin("grid_tg", false);
  for (uint8_t i=0;i<MAX_CHATS;++i) {
    String key="chat"+String(i);
    chatIds[i]=prefs.getString(key.c_str(),"");
  }
  initialized=true;
  lastPower=PowerMonitor::hasMainsPower();
  lastPowerKnown=true;
  Serial.printf("[TG] Initialized. Token: %s, Chat ID: %s, Wi-Fi: %s, object: %s\n",
    configuredToken() ? "configured" : "MISSING",
    hasChat() ? "configured" : "MISSING",
    WiFi.status()==WL_CONNECTED ? "connected" : "not connected",
    activeObjectName.c_str());
  if (!configuredToken()) Serial.println("[TG] Set TELEGRAM_BOT_TOKEN in secrets.h; never share the token.");
  if (!hasChat()) Serial.println("[TG] Add your Chat ID on the web page and save it.");
}
void update(const String& objectName) {
  if (!initialized) return;
  activeObjectName=objectName;
  queueStartup();
  queueDailyReport();
  if (!qCount) return;
  if (WiFi.status()!=WL_CONNECTED || !configuredToken() || !hasChat()) return;
  uint32_t now=millis();
  if (sendAttempted && (uint32_t)(now-lastSendMs)<retryAfterMs) return;
  sendAttempted=true; lastSendMs=now;
  QueuedMessage& item=queue[qHead];
  if (sendOne(chatIds[item.chatIndex],item.text)) {
    qHead=(qHead+1)%QUEUE_CAPACITY; --qCount; retryAfterMs=SEND_GAP_MS;
  } else retryAfterMs=RETRY_MS;
}
void onPowerEvent(bool powerPresent, time_t eventTime) {
  if (!initialized || !lastPowerKnown) { lastPower=powerPresent; lastPowerKnown=true; return; }
  if (powerPresent==lastPower) return;
  lastPower=powerPresent;
  String msg=prefix(activeObjectName);
  msg += powerPresent ? "✅ Світло з'явилось." : "🚨 Світло зникло.";
  msg += timestampSuffix(eventTime, false);
  if (enqueue(msg)) Serial.println("[TG] Power transition notification queued.");
  else Serial.println("[TG] ERROR: could not queue power transition (queue full).");
}
String chatIdsJson() {
  String out="{\"chats\":[";
  for(uint8_t i=0;i<MAX_CHATS;++i) { if(i) out+=","; out+="\""+jsonEscape(chatIds[i])+"\""; }
  out+="],\"configured\":"; out+=(configuredToken()?"true":"false"); out+="}";
  return out;
}
bool saveChatIds(const String ids[4]) {
  for(uint8_t i=0;i<MAX_CHATS;++i) if(ids[i].length()>40) return false;
  for(uint8_t i=0;i<MAX_CHATS;++i) {
    chatIds[i]=ids[i]; String key="chat"+String(i);
    prefs.putString(key.c_str(),chatIds[i]);
  }
  return true;
}
} // namespace Telegram
