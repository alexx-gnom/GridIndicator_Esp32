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
  <meta http-equiv="refresh" content="10">
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
)HTML";

static const char INDICATOR_PAGE_END[] = R"HTML(
      <p class="note">Сторінка оновлюється автоматично кожні 10 секунд.</p>
    </section>
    <p class="footer">Grid Indicator · ESP32-C3</p>
  </main>
</body>
</html>
)HTML";
