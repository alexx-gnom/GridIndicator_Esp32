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
    .note { margin: 20px 0 0; color: var(--muted); font-size: 13px; }
    .footer { margin-top: 18px; color: var(--muted); font-size: 12px; text-align: center; }
    @media (max-width: 380px) { .card { padding: 18px; } .grid { grid-template-columns: 1fr; } }
  </style>
</head>
<body>
  <main class="container">
    <section class="card">
      <h1>Grid Indicator</h1>
      <p class="subtitle">Стан пристрою</p>
      <div class="clock-card">
        <div class="clock-row">
          <div class="clock-info"><strong class="clock-title">Годинник ESP · Київ</strong><span id="clockDate" class="actual">Очікування NTP…</span></div>
          <div id="clockTime" class="clock-time">--:--:--</div>
        </div>
        <div class="clock-meta">NTP: <span id="ntpStatus">перевірка…</span> · Uptime: <span id="uptime">—</span></div>
        <div class="clock-meta">Остання відповідь ESP: <span id="lastResponse">—</span></div>
      </div>
      <div id="connectionStatus" class="status"><span id="statusDot" class="dot"></span><div><div id="statusTitle" class="status-title">Перевірка стану…</div><div id="statusDetail" class="status-detail">GPIO10: —</div></div></div>
      <div class="grid">
        <div class="metric"><div class="metric-label">Об'єкт</div><div class="metric-value" id="objectName">—</div></div>
        <div class="metric"><div class="metric-label">Wi-Fi мережа</div><div class="metric-value" id="wifiSsid">—</div></div>
        <div class="metric"><div class="metric-label">IP-адреса</div><div class="metric-value" id="ipAddress">—</div></div>
        <div class="metric"><div class="metric-label">Рівень сигналу</div><div class="metric-value" id="wifiRssi">—</div></div>
        <div class="metric"><div class="metric-label">Остання подія</div><div class="metric-value" id="lastEvent">—</div></div>
      </div>
      <p class="note">Статус оновлюється раз на секунду без перезавантаження сторінки.</p>
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
    document.getElementById("statusDetail").textContent = "GPIO10: " + (d.rawHigh ? "HIGH" : "LOW") + " · підтверджений стан";
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
setInterval(updateStatus, 1000);
setInterval(updateLastResponse, 1000);
updateStatus();
</script>
</body>
</html>
)HTML";
