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
    /* Steampunk power indicator: CSS/SVG only, no image assets or libraries. */
    :root {
      --brass: #b98543;
      --brass-light: #e7bd73;
      --copper: #8e512f;
      --coal: #17130f;
      --lamp-glow: #ffb83f;
    }
    .steam-panel {
      position:relative; display:grid; grid-template-columns: minmax(100px, 142px) 1fr;
      gap:16px; align-items:center; margin-top:12px; padding:16px;
      border:1px solid #936a3b; border-radius:14px; overflow:hidden;
      background: radial-gradient(ellipse at 25% 45%, #50321c 0%, #241b14 36%, #13151a 78%);
      box-shadow: inset 0 0 0 3px #4b3826, inset 0 0 22px #000b, 0 8px 20px #0005;
    }
    .steam-panel:before { content:""; position:absolute; inset:5px; border:1px solid #b9854355; border-radius:10px; pointer-events:none; }
    .steam-lamp-wrap { position:relative; width:100%; max-width:142px; aspect-ratio:1; display:grid; place-items:center; }
    .steam-lamp-glow { position:absolute; inset:8%; border-radius:50%; background:radial-gradient(circle,#ffcb5c55 0%,#ff9d2430 35%,transparent 72%); opacity:0; transition:opacity .8s ease; }
    .steam-lamp { width:100%; height:100%; overflow:visible; filter:drop-shadow(0 0 2px #000); }
    .steam-glass { fill:#4b3827; fill-opacity:.45; stroke:#b98543; stroke-width:2.4; transition:fill .7s, fill-opacity .7s, stroke .7s; }
    .steam-filament { fill:none; stroke:#8f7049; stroke-width:2.5; stroke-linecap:round; stroke-linejoin:round; transition:stroke .7s, filter .7s; }
    .steam-base { fill:#4c3824; stroke:#b98543; stroke-width:2; }
    .steam-gear { transform-box:fill-box; transform-origin:center; fill:#62452a; stroke:#c08a45; stroke-width:2; }
    .steam-gear-inner { fill:#201810; stroke:#d4a35d; stroke-width:2; }
    .steam-copy { min-width:0; position:relative; z-index:1; }
    .steam-kicker { color:#c99b5c; font-size:11px; letter-spacing:1.8px; text-transform:uppercase; }
    .steam-state { margin:3px 0 5px; color:#e8c27f; font-size:clamp(19px,4.8vw,29px); line-height:1.12; font-weight:800; letter-spacing:.5px; text-shadow:0 2px 10px #000; }
    .steam-detail { color:#b6a18a; font-size:12px; }
    .steam-status-lamp { display:inline-block; width:9px; height:9px; margin-right:6px; border-radius:50%; background:#7b8a79; box-shadow:0 0 0 3px #ffffff0b; vertical-align:1px; }
    .steam-gear-box { display:flex; align-items:center; gap:6px; margin-top:12px; color:#b68a50; font-size:10px; letter-spacing:.7px; }
    .steam-gear-mini { width:24px; height:24px; animation:steamTurn 18s linear infinite; }
    .steam-gear-mini path { fill:#64472a; stroke:#bd8846; stroke-width:2; }
    .steam-gear-mini circle { fill:#21170f; stroke:#bd8846; stroke-width:2; }
    .steam-panel.power-on .steam-lamp-glow { opacity:1; animation:lampBreath 4s ease-in-out infinite; }
    .steam-panel.power-on .steam-glass { fill:#ffbd4d; fill-opacity:.24; stroke:#f0bc63; }
    .steam-panel.power-on .steam-filament { stroke:#fff0a1; filter:drop-shadow(0 0 4px #ff9d24) drop-shadow(0 0 9px #ff9d24); }
    .steam-panel.power-on .steam-status-lamp { background:#8be5a9; box-shadow:0 0 10px #5ee58a; }
    .steam-panel.power-off .steam-glass { fill:#251d17; fill-opacity:.5; stroke:#8c6440; }
    .steam-panel.power-off .steam-filament { stroke:#6e4930; filter:none; }
    .steam-panel.power-off .steam-status-lamp { background:#e17a50; box-shadow:0 0 8px #d95a35; }
    .steam-panel.power-off .steam-state { color:#e29a69; }
    @keyframes lampBreath { 0%,100% { opacity:.72; } 50% { opacity:1; } }
    @keyframes steamTurn { to { transform:rotate(360deg); } }
    @media (prefers-reduced-motion: reduce) {
      .steam-gear-mini, .steam-panel.power-on .steam-lamp-glow { animation:none; }
      .steam-glass, .steam-filament, .steam-lamp-glow { transition:none; }
    }
    /* Unified steampunk skin for the existing UI components. */
    :root {
      color-scheme: dark;
      --bg: #100e0c;
      --panel: #201912;
      --field: #17130f;
      --border: #79532f;
      --text: #f2dfbd;
      --muted: #b49a78;
      --accent: #d9a354;
      --ok: #8de5a8;
    }
    body {
      background:
        radial-gradient(ellipse at 50% -10%, #4c321d 0, transparent 45%),
        repeating-linear-gradient(135deg, #100e0c 0, #100e0c 8px, #14100c 8px, #14100c 16px);
      color:var(--text);
    }
    .container { max-width: 620px; }
    .card {
      background:linear-gradient(145deg,#292117,#1b1712 60%,#171719);
      border:1px solid #92663a;
      border-radius:17px;
      box-shadow:inset 0 0 0 3px #342619, inset 0 0 0 4px #6b492b55, 0 14px 34px #0009;
      position:relative;
    }
    h1 {
      color:#e9bd77;
      text-shadow:0 2px 0 #4b2d17,0 0 15px #d18a3840;
      letter-spacing:.2px;
    }
    .subtitle { color:#b99a70; }
    .clock-card, .history-card {
      background:linear-gradient(145deg,#251d14,#171513);
      border:1px solid #81572f;
      border-radius:13px;
      box-shadow:inset 0 0 0 2px #382819, inset 0 0 0 3px #8b633333, 0 5px 14px #0005;
    }
    .clock-title, .history-head > strong { color:#e7bd79; letter-spacing:.25px; }
    .actual, .clock-meta, .metric-label, .history-help, .timeline-label, .hours, .legend { color:#b6a080; }
    .clock-time { color:#ffdc9a; text-shadow:0 0 12px #d48a3650; }
    .metric, .history-stat {
      background:linear-gradient(145deg,#211a13,#151413);
      border:1px solid #76502d;
      border-radius:10px;
      box-shadow:inset 0 0 0 2px #302216, inset 0 0 0 3px #9b6d3a22;
    }
    .metric-value, .history-stat-value { color:#f0d7a8; }
    .tg-input {
      background:#100e0c;
      border:1px solid #9b7143;
      color:#f6dfb7;
      box-shadow:inset 0 2px 7px #0009;
    }
    .tg-input:focus { outline:2px solid #c8944e; outline-offset:1px; }
    .tg-save, .history-nav button {
      color:#f4d7a2;
      background:linear-gradient(#654526,#3b2818);
      border:1px solid #b27d3e;
      box-shadow:inset 0 1px #f5d39a44,0 2px 4px #0007;
      cursor:pointer;
    }
    .tg-save:hover, .history-nav button:hover { background:linear-gradient(#80592f,#4a301b); }
    button:disabled { opacity:.35; cursor:not-allowed; }
    .history-date { color:#e8c486; }
    .timeline-scroll { scrollbar-color:#a47a45 #20170f; }
    .slots { background:#392a1c; border:1px solid #6e4a2b; padding:2px; gap:2px; height:29px; }
    .slot { background:#655642; border-radius:1px; }
    .slot.power { background:linear-gradient(#68b77e,#287044); box-shadow:inset 0 0 0 1px #a1d18b55; }
    .slot.outage { background:linear-gradient(#edb15a,#a94d2d); box-shadow:inset 0 0 0 1px #ffe0a044; }
    .slot.hour-start { border-left:2px solid #e6c58d; }
    .hours span { border-left-color:#8d6a42; }
    .legend i { box-shadow:0 0 0 1px #d8b27433; }
    .history-stat-label { color:#b69b75; }
    .footer { color:#a88a65; }
    @media (max-width: 460px) {
      .steam-panel { grid-template-columns: minmax(84px, 112px) 1fr; gap:10px; padding:12px; }
      .steam-kicker { letter-spacing:1px; font-size:9px; }
      .steam-detail { font-size:11px; }
      .steam-gear-box { font-size:8px; }
    }
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
      <section id="steamPanel" class="steam-panel" aria-live="polite">
        <div class="steam-lamp-wrap">
          <div class="steam-lamp-glow"></div>
          <svg class="steam-lamp" viewBox="0 0 120 120" role="img" aria-label="Індикатор електропостачання">
            <path class="steam-glass" d="M42 13 C25 18 18 34 22 50 C25 63 37 71 40 81 L40 88 L80 88 L80 81 C83 71 95 63 98 50 C102 34 95 18 78 13 C67 9 53 9 42 13Z"/>
            <path class="steam-filament" d="M43 30 L49 59 L60 34 L71 59 L77 30 M49 59 L49 70 M71 59 L71 70 M49 70 Q60 78 71 70"/>
            <path class="steam-base" d="M41 87 H79 V96 H41Z M44 98 H76 V103 H44Z M48 105 H72 V110 H48Z"/>
            <path d="M48 91H72 M48 100H72" stroke="#d0a15d" stroke-width="1.5"/>
          </svg>
          <svg class="steam-gear-mini" viewBox="0 0 40 40" aria-hidden="true" style="position:absolute;right:0;bottom:3px">
            <path d="M16 1H24L26 6L31 8L36 6L40 13L36 17L36 23L40 27L36 34L31 32L26 34L24 39H16L14 34L9 32L4 34L0 27L4 23V17L0 13L4 6L9 8L14 6Z"/>
            <circle cx="20" cy="20" r="8"/>
          </svg>
        </div>
        <div class="steam-copy">
          <div class="steam-kicker">Енергетичний контур</div>
          <div id="steamState" class="steam-state">ПЕРЕВІРКА…</div>
          <div class="steam-detail"><span id="steamStatusLamp" class="steam-status-lamp"></span><span id="steamDetail">Очікування даних від ESP</span></div>
          <div class="steam-gear-box">
            <svg class="steam-gear-mini" viewBox="0 0 40 40" aria-hidden="true">
              <path d="M16 1H24L26 6L31 8L36 6L40 13L36 17L36 23L40 27L36 34L31 32L26 34L24 39H16L14 34L9 32L4 34L0 27L4 23V17L0 13L4 6L9 8L14 6Z"/>
              <circle cx="20" cy="20" r="8"/>
            </svg>
            МЕХАНІЗМ МОНІТОРИНГУ
          </div>
        </div>
      </section>
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
    <p class="footer">ESP: <span id="connection">підключення…</span> · -=GnOM=-© Grid Indicator · ESP32-C3</p>
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
    const steamPanel = document.getElementById("steamPanel");
    steamPanel.classList.toggle("power-on", !!d.powerPresent);
    steamPanel.classList.toggle("power-off", !d.powerPresent);
    document.getElementById("steamState").textContent = d.powerPresent ? "МЕРЕЖА Є" : "НЕМАЄ СВІТЛА";
    document.getElementById("steamDetail").textContent = d.powerPresent ? "Лампа працює · контур під напругою" : "Лампа згасла · очікування мережі";
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
    const input = document.getElementById("chat0");
    if (input) input.value = (d.chats && d.chats.length ? d.chats[0] : "") || "";
    status.textContent = d.configured ? "Бот готовий" : "Потрібен токен бота в secrets.h";
  } catch (e) { status.textContent = "Не вдалося завантажити"; }
}
document.getElementById("saveTelegram").addEventListener("click", async () => {
  const button = document.getElementById("saveTelegram");
  const status = document.getElementById("telegramStatus");
  button.disabled = true;
  const params = new URLSearchParams();
  const input = document.getElementById("chat0");
  if (!input) {
    console.error("[TG CFG] Не знайдено поле chat0");
    status.textContent = "Помилка: поле Chat ID не знайдено";
    button.disabled = false;
    return;
  }
  const chatId = input.value.trim();
  console.log("[TG CFG] Надсилаю Chat ID: довжина=" + chatId.length +
    ", останні символи=" + (chatId ? chatId.slice(-4) : "(порожньо)"));
  params.set("chat0", chatId);
  try {
    const r = await fetch("/api/telegram/chats", {method:"POST", headers:{"Content-Type":"application/x-www-form-urlencoded"}, body:params.toString()});
    const responseText = await r.text();
    console.log("[TG CFG] HTTP " + r.status + "; відповідь=" + responseText);
    const d = JSON.parse(responseText);
    if (!r.ok || !d.ok) throw new Error("save");
    status.textContent = "Chat ID збережено";
    await loadTelegramChats();
  } catch(e) {
    console.error("[TG CFG] Помилка збереження:", e);
    status.textContent = "Помилка збереження";
  } finally { button.disabled = false; }
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
