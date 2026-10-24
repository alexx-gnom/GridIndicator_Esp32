#pragma once

#include <Arduino.h>

// HTML-сторінка робочого режиму. Дані підставляються у WebServer.cpp.
static const char INDICATOR_PAGE_START[] = R"HTML(
<!doctype html>
<html lang="uk">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#101318">
  <title>Grid Indicator</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #101318;
      --panel: #1a1f27;
      --field: #11161d;
      --border: #303846;
      --text: #edf1f7;
      --muted: #9ba7b8;
      --accent: #65a8ff;
      --ok: #73d69a;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0; padding: 28px 16px; background: var(--bg); color: var(--text);
      font-family: system-ui, -apple-system, sans-serif; font-size: 15px; line-height: 1.5;
    }
    .container { width: 100%; max-width: 520px; margin: 0 auto; }
    .card {
      padding: 24px; background: var(--panel); border: 1px solid var(--border);
      border-radius: 18px; box-shadow: 0 12px 32px #0004;
    }
    h1 { margin: 0 0 6px; font-size: 25px; letter-spacing: -.5px; }
    .subtitle { margin: 0 0 24px; color: var(--muted); }
    .clock-card { margin-bottom: 12px; padding: 16px; background: var(--panel); border: 1px solid var(--border); border-radius: 14px; }
    .clock-row { display: flex; justify-content: space-between; align-items: center; gap: 12px; flex-wrap: wrap; }
    .clock-info { display: flex; flex-direction: column; gap: 5px; }
    .clock-title { font-weight: 700; }
    .actual { color: var(--muted); font-size: 12px; }
    .clock-time { font-size: 27px; font-weight: bold; font-variant-numeric: tabular-nums; }
    .clock-meta { margin-top: 8px; color: var(--muted); font-size: 12px; }
    .status { display: flex; align-items: center; gap: 10px; padding: 14px;
      border: 1px solid #73d69a44; border-radius: 12px; background: #73d69a0d; }
    .dot { width: 10px; height: 10px; flex: 0 0 10px; border-radius: 50%; background: var(--ok); }
    .status-title { font-weight: 700; }
    .status-detail { color: var(--muted); font-size: 13px; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-top: 16px; }
    .metric { min-width: 0; padding: 14px; border-radius: 12px; background: var(--field); border: 1px solid var(--border); }
    .metric-label { color: var(--muted); font-size: 13px; }
    .metric-value { margin-top: 5px; font-size: 16px; font-weight: 650; overflow-wrap: anywhere; }
    .tg-grid { display:grid; grid-template-columns:1fr 1fr; gap:8px; margin-top:10px; }
    .tg-input { min-width:0; width:100%; padding:10px; border-radius:8px; border:1px solid var(--border); background:var(--field); color:var(--text); font:inherit; }
    .tg-save { margin-top:10px; border:1px solid var(--border); border-radius:8px; padding:9px 12px; background:#303846; color:var(--text); font:inherit; cursor:pointer; }
    .history-card { margin-top: 12px; padding: 18px; background: var(--panel); border: 1px solid var(--border); border-radius: 14px; }
    .history-head { display:flex; justify-content:space-between; align-items:center; gap:10px; flex-wrap:wrap; }
    .history-nav { display:flex; align-items:center; gap:8px; }
    .history-nav button { background:#303846; color:var(--text); border:1px solid var(--border); border-radius:8px; padding:6px 10px; }
    .history-date { font-weight:700; min-width:120px; text-align:center; }
    .timeline-scroll { overflow-x:auto; margin-top:18px; padding-bottom:4px; }
    .timeline { min-width:540px; }
    .hours { display:grid; grid-template-columns:repeat(24,minmax(0,1fr)); margin-left:42px; color:var(--muted); font-size:10px; }
    .hours span { border-left:1px solid #536071; padding-left:2px; }
    .timeline-row { display:grid; grid-template-columns:38px minmax(0,1fr); gap:4px; align-items:center; margin-top:5px; }
    .timeline-label { font-size:11px; color:var(--muted); }
    .slots { display:grid; grid-template-columns:repeat(96,minmax(0,1fr)); gap:1px; height:25px; background:#303846; }
    .slot { min-width:0; background:#64748b; }
    .slot.power { background:#2b8a61; }
    .slot.outage { background:#e5a23b; }
    .slot.hour-start { border-left:2px solid #edf1f7; }
    .legend { display:flex; gap:12px; flex-wrap:wrap; margin-top:12px; font-size:11px; color:var(--muted); }
    .legend span { display:flex; align-items:center; gap:5px; }
    .legend i { width:9px; height:9px; border-radius:2px; display:inline-block; }
    .history-stats { display:grid; grid-template-columns:repeat(3,minmax(0,1fr)); gap:8px; margin-top:16px; }
    .history-stat { background:var(--field); border:1px solid var(--border); border-radius:10px; padding:10px; min-width:0; }
    .history-stat-label { font-size:11px; color:var(--muted); }
    .history-stat-value { font-size:15px; font-weight:700; margin-top:4px; }
    .history-help { color:var(--muted); font-size:11px; margin-top:10px; }
    .note { margin: 20px 0 0; color: var(--muted); font-size: 13px; }
    .footer { margin-top: 18px; color: var(--muted); font-size: 12px; text-align: center; }
    @media (max-width: 380px) { .card { padding: 18px; } .grid { grid-template-columns: 1fr; } .history-stats { grid-template-columns:1fr; } }
  </style>
</head>
<body>
  <main class="container">
    <section class="card">
      <h1>Grid Indicator</h1>
      <div class="clock-card">
        <div class="clock-row">
          <div class="clock-info"><strong class="clock-title">Годинник ESP · Київ</strong><span id="clockDate" class="actual">Очікування NTP…</span></div>
          <div id="clockTime" class="clock-time">--:--:--</div>
        </div>
        <div class="clock-meta">NTP: <span id="ntpStatus">перевірка…</span> · Uptime: <span id="uptime">—</span></div>
        <div class="clock-meta">Остання відповідь ESP: <span id="lastResponse">—</span></div>
      </div>
      <div id="connectionStatus" class="status"><span id="statusDot" class="dot"></span><div><div id="statusTitle" class="status-title">Перевірка стану…</div></div></div>
      <div class="grid">
        <div class="metric"><div class="metric-label">Об'єкт</div><div class="metric-value" id="objectName">—</div></div>
        <div class="metric"><div class="metric-label">Wi-Fi мережа</div><div class="metric-value" id="wifiSsid">—</div></div>
        <div class="metric"><div class="metric-label">IP-адреса</div><div class="metric-value" id="ipAddress">—</div></div>
        <div class="metric"><div class="metric-label">Рівень сигналу</div><div class="metric-value" id="wifiRssi">—</div></div>
        <div class="metric"><div class="metric-label">Остання подія</div><div class="metric-value" id="lastEvent">—</div></div>
      </div>
      <section class="history-card">
        <div class="history-head"><strong>Telegram</strong><span id="telegramStatus" class="actual">Завантаження…</span></div>
        <p class="history-help">Вкажи Chat ID отримувача. Для кількох людей можна використати групу Telegram.</p>
        <div class="tg-grid">
          <input id="chat0" class="tg-input" inputmode="numeric" maxlength="40" placeholder="Chat ID">
        </div>
        <button id="saveTelegram" class="tg-save" type="button">Зберегти Chat ID</button>
      </section>
      <section class="history-card">
        <div class="history-head">
          <strong>Історія електропостачання</strong>
          <div class="history-nav">
            <button id="historyPrev" type="button" aria-label="Попередній день">‹</button>
            <span id="historyDate" class="history-date">—</span>
            <button id="historyNext" type="button" aria-label="Наступний день">›</button>
          </div>
        </div>
        <div class="timeline-scroll">
          <div class="timeline">
            <div class="hours"><span>00</span><span>01</span><span>02</span><span>03</span><span>04</span><span>05</span><span>06</span><span>07</span><span>08</span><span>09</span><span>10</span><span>11</span><span>12</span><span>13</span><span>14</span><span>15</span><span>16</span><span>17</span><span>18</span><span>19</span><span>20</span><span>21</span><span>22</span><span>23</span></div>
            <div class="timeline-row"><span class="timeline-label">00–24</span><div id="historySlots" class="slots"></div></div>
          </div>
        </div>
        <div class="legend">
          <span><i style="background:#2b8a61"></i>Світло є</span>
          <span><i style="background:#e5a23b"></i>Відключення</span>
          <span><i style="background:#64748b"></i>Немає даних</span>
        </div>
        <div class="history-stats">
          <div class="history-stat"><div class="history-stat-label">Світло було</div><div id="historyMains" class="history-stat-value">—</div></div>
          <div class="history-stat"><div class="history-stat-label">Відсутнє</div><div id="historyOutage" class="history-stat-value">—</div></div>
          <div class="history-stat"><div class="history-stat-label">Офлайн / невідомо</div><div id="historyUnknown" class="history-stat-value">—</div></div>
        </div>
        <div id="historyHint" class="history-help">Дані накопичуються після запуску моніторингу.</div>
      </section>
)HTML";

static const char INDICATOR_PAGE_END[] = R"HTML(
    </section>
    <p class="footer">ESP: <span id="connection">підключення…</span> · Grid Indicator · ESP32-C3</p>
  </main>
<script>
let lastSuccessfulResponse = 0;
function formatUptime(ms) {
  const s = Math.floor(Number(ms) / 1000);
  const d = Math.floor(s / 86400), h = Math.floor((s % 86400) / 3600);
  const m = Math.floor((s % 3600) / 60), sec = s % 60;
  return (d ? d + " д " : "") + String(h).padStart(2,"0") + ":" + String(m).padStart(2,"0") + ":" + String(sec).padStart(2,"0");
}
function updateLastResponse() {
  if (!lastSuccessfulResponse) return;
  const age = Math.floor((Date.now() - lastSuccessfulResponse) / 1000);
  const el = document.getElementById("lastResponse");
  el.textContent = age < 2 ? "щойно" : age + " с тому";
  el.style.color = age > 5 ? "#ff7676" : "#aab7c5";
}
async function updateStatus() {
  try {
    const response = await fetch("/api/status", { cache: "no-store" });
    if (!response.ok) throw new Error("HTTP " + response.status);
    const d = await response.json();
    lastSuccessfulResponse = Date.now();
    document.getElementById("clockTime").textContent = d.clockValid ? d.clock.substring(11,19) : "--:--:--";
    document.getElementById("clockDate").textContent = d.clockValid ? d.clock.substring(0,10) + " · Europe/Kyiv" : "Час ще не синхронізовано";
    document.getElementById("ntpStatus").textContent = d.clockValid ? "OK" : d.clockStatus;
    document.getElementById("ntpStatus").style.color = d.clockValid ? "#55d98b" : "#ffcc66";
    document.getElementById("uptime").textContent = formatUptime(d.uptimeMs);
    document.getElementById("objectName").textContent = d.objectName || "Не задано";
    document.getElementById("wifiSsid").textContent = d.ssid || "—";
    document.getElementById("ipAddress").textContent = d.ip || "—";
    document.getElementById("wifiRssi").textContent = d.rssi + " dBm · " + d.wifiQuality;
    document.getElementById("statusTitle").textContent = d.powerPresent ? "Мережа є" : "Немає мережі";
    document.getElementById("statusDot").style.background = d.powerPresent ? "var(--ok)" : "#ff7676";
    document.getElementById("connectionStatus").style.borderColor = d.powerPresent ? "#73d69a44" : "#ff767644";
    document.getElementById("connectionStatus").style.background = d.powerPresent ? "#73d69a0d" : "#ff76760d";
    document.getElementById("lastEvent").textContent = d.lastEvent || "Подій після запуску немає";
    document.getElementById("connection").textContent = "ONLINE";
    updateLastResponse();
  } catch (e) {
    document.getElementById("connection").textContent = "OFFLINE";
  }
}
async function loadTelegramChats() {
  const status = document.getElementById("telegramStatus");
  try {
    const r = await fetch("/api/telegram/chats", {cache:"no-store"});
    if (!r.ok) throw new Error("load");
    const d = await r.json();
    (d.chats || []).forEach((id, i) => { document.getElementById("chat"+i).value = id || ""; });
    status.textContent = d.configured ? "Бот готовий" : "Потрібен токен бота в secrets.h";
  } catch (e) { status.textContent = "Не вдалося завантажити"; }
}
document.getElementById("saveTelegram").addEventListener("click", async () => {
  const button = document.getElementById("saveTelegram");
  const status = document.getElementById("telegramStatus");
  button.disabled = true;
  const params = new URLSearchParams();
  for (let i=0; i<4; i++) params.set("chat"+i, document.getElementById("chat"+i).value.trim());
  try {
    const r = await fetch("/api/telegram/chats", {method:"POST", headers:{"Content-Type":"application/x-www-form-urlencoded"}, body:params.toString()});
    const d = await r.json();
    if (!r.ok || !d.ok) throw new Error("save");
    status.textContent = "Chat ID збережено";
  } catch(e) { status.textContent = "Помилка збереження"; }
  finally { button.disabled = false; }
});
let selectedHistoryDate = "";
function fmtDuration(sec) {
  sec = Math.max(0, Number(sec) || 0);
  const h = Math.floor(sec / 3600), m = Math.floor((sec % 3600) / 60);
  return h + " год " + String(m).padStart(2, "0") + " хв";
}
function dateShift(dateText, delta) {
  const d = new Date(dateText + "T12:00:00");
  d.setDate(d.getDate() + delta);
  return d.getFullYear() + "-" + String(d.getMonth()+1).padStart(2,"0") + "-" + String(d.getDate()).padStart(2,"0");
}
async function updateHistory() {
  try {
    const r = await fetch("/api/history?date=" + selectedHistoryDate, {cache:"no-store"});
    if (!r.ok) throw new Error("history");
    const d = await r.json();
    if (!d.valid) { document.getElementById("historyHint").textContent = "Очікування синхронізації годинника."; return; }
    selectedHistoryDate = d.date;
    document.getElementById("historyDate").textContent = d.date;
    const slots = document.getElementById("historySlots");
    slots.replaceChildren();
    d.slots.forEach((s, i) => {
      const el = document.createElement("div");
      el.className = "slot";
      const known = Number(s.m) + Number(s.o);
      if (known > Number(s.u) && known > 0) {
        el.classList.add(Number(s.o) > Number(s.m) ? "outage" : "power");
      }
      if (i % 4 === 0) el.classList.add("hour-start");
      el.title = String(Math.floor(i/4)).padStart(2,"0") + ":" + (i%4*15).toString().padStart(2,"0") +
        " · світло " + fmtDuration(s.m) + " · відключення " + fmtDuration(s.o) + " · невідомо " + fmtDuration(s.u);
      slots.appendChild(el);
    });
    document.getElementById("historyMains").textContent = fmtDuration(d.mains);
    document.getElementById("historyOutage").textContent = fmtDuration(d.outage);
    document.getElementById("historyUnknown").textContent = fmtDuration(d.unknown);
    document.getElementById("historyHint").textContent = "Блоки по 15 хвилин. Наведи курсор на блок, щоб побачити деталізацію.";
    document.getElementById("historyPrev").disabled = dateShift(d.date, -1) < dateShift(d.date, -6);
    document.getElementById("historyNext").disabled = d.date >= new Date().toISOString().slice(0,10);
  } catch (e) {
    document.getElementById("historyHint").textContent = "Не вдалося завантажити історію.";
  }
}
document.getElementById("historyPrev").addEventListener("click", () => { selectedHistoryDate = dateShift(selectedHistoryDate, -1); updateHistory(); });
document.getElementById("historyNext").addEventListener("click", () => { selectedHistoryDate = dateShift(selectedHistoryDate, 1); updateHistory(); });
setInterval(updateHistory, 10000);
updateHistory();
loadTelegramChats();
setInterval(updateStatus, 1000);
setInterval(updateLastResponse, 1000);
updateStatus();
</script>
</body>
</html>
)HTML";
