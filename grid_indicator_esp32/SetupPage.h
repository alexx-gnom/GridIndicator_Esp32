#pragma once

// Спільна HTML-оболонка для сторінок Setup Portal.
// Весь стиль сторінки налаштовується тут.

static const char SETUP_PAGE_START[] = R"HTML(
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
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      padding: 28px 16px;
      background: var(--bg);
      color: var(--text);
      font-family: system-ui, -apple-system, sans-serif;
      font-size: 15px;
      line-height: 1.5;
    }

    .container {
      width: 100%;
      max-width: 460px;
      margin: 0 auto;
    }

    .card {
      padding: 24px;
      background: var(--panel);
      border: 1px solid var(--border);
      border-radius: 18px;
      box-shadow: 0 12px 32px #0004;
    }

    h1 {
      margin: 0 0 6px;
      font-size: 25px;
      letter-spacing: -0.5px;
    }

    h2 { margin: 0 0 8px; font-size: 19px; }

    .subtitle {
      margin: 0 0 24px;
      color: var(--muted);
    }

    label {
      display: block;
      margin: 18px 0 7px;
      font-weight: 600;
    }

    input {
      display: block;
      width: 100%;
      min-height: 46px;
      padding: 11px 13px;
      border: 1px solid var(--border);
      border-radius: 10px;
      background: var(--field);
      color: var(--text);
      font: inherit;
      outline: none;
    }

    input:focus {
      border-color: var(--accent);
      box-shadow: 0 0 0 3px #65a8ff22;
    }

    button {
      width: 100%;
      min-height: 48px;
      margin-top: 24px;
      padding: 12px;
      border: 0;
      border-radius: 10px;
      background: var(--accent);
      color: #101722;
      font: inherit;
      font-weight: 700;
      cursor: pointer;
    }

    button:hover { filter: brightness(1.08); }

    .note {
      margin: 12px 0 0;
      color: var(--muted);
      font-size: 13px;
    }

    a { color: var(--accent); }

    .footer {
      margin-top: 18px;
      color: var(--muted);
      font-size: 12px;
      text-align: center;
    }

    @media (max-width: 380px) {
      .card { padding: 18px; }
    }
  </style>
</head>
<body>
  <main class="container">
)HTML";

static const char SETUP_PAGE_END[] = R"HTML(
    <p class="footer">Grid Indicator · ESP32-C3</p>
  </main>
</body>
</html>
)HTML";
