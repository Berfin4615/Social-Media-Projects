#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// OLED AYARLARI
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 20
#define OLED_SCL 21
#define OLED_ADDRESS 0x3C
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// WI-FI AYARLARI
// =====================================================

// Ev Wi-Fi bilgilerini buraya yaz.
const char* WIFI_SSID = "********";
const char* WIFI_PASSWORD = "********";

// Ev Wi-Fi ağına bağlanamazsa açılacak ağ.
const char* AP_SSID = "BMO-Control";
const char* AP_PASSWORD = "bmo12345";

WebServer server(80);

bool accessPointMode = false;

// =====================================================
// POMODORO AYARLARI
// =====================================================

enum TimerState {
  TIMER_READY,
  TIMER_RUNNING,
  TIMER_PAUSED,
  TIMER_FINISHED
};

enum TimerMode {
  MODE_FOCUS,
  MODE_BREAK,
  MODE_CUSTOM
};

TimerState timerState = TIMER_READY;
TimerMode timerMode = MODE_FOCUS;

unsigned long timerDurationMs = 25UL * 60UL * 1000UL;
unsigned long remainingTimeMs = 25UL * 60UL * 1000UL;
unsigned long previousTimerUpdate = 0;

int completedPomodoros = 0;

// =====================================================
// ANİMASYON AYARLARI
// =====================================================

unsigned long previousAnimationUpdate = 0;
unsigned long finishedAnimationStarted = 0;

bool eyesClosed = false;
bool celebrationFrame = false;

const unsigned long BLINK_INTERVAL = 3500;
const unsigned long BLINK_DURATION = 160;

// =====================================================
// WEB SAYFASI
// =====================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="UTF-8">
  <meta
    name="viewport"
    content="width=device-width, initial-scale=1.0"
  >

  <title>BMO Pomodoro</title>

  <style>
    * {
      box-sizing: border-box;
    }

    body {
      margin: 0;
      min-height: 100vh;
      padding: 24px 16px;
      display: flex;
      justify-content: center;
      align-items: center;
      font-family: Arial, Helvetica, sans-serif;
      color: #153838;
      background:
        radial-gradient(
          circle at top,
          #d8fff5 0%,
          #8bd8c4 45%,
          #58ae9e 100%
        );
    }

    .container {
      width: 100%;
      max-width: 420px;
    }

    .bmo-card {
      padding: 22px;
      border: 4px solid #225d59;
      border-radius: 28px;
      background: #74c9b5;
      box-shadow:
        0 18px 45px rgba(21, 56, 56, 0.25),
        inset 0 2px 0 rgba(255, 255, 255, 0.45);
    }

    .title {
      margin: 0 0 5px;
      text-align: center;
      font-size: 28px;
      letter-spacing: 2px;
    }

    .subtitle {
      margin: 0 0 18px;
      text-align: center;
      font-size: 13px;
      opacity: 0.75;
    }

    .screen {
      min-height: 175px;
      padding: 22px 16px;
      border: 5px solid #173a3a;
      border-radius: 18px;
      background: #caffb7;
      box-shadow:
        inset 0 0 15px rgba(23, 58, 58, 0.18),
        0 5px 0 rgba(23, 58, 58, 0.2);
    }

    .face {
      height: 65px;
      display: flex;
      justify-content: center;
      align-items: center;
      font-family: monospace;
      font-size: 42px;
      font-weight: bold;
      white-space: pre;
    }

    .mode {
      margin-top: 4px;
      text-align: center;
      font-size: 14px;
      font-weight: bold;
      letter-spacing: 2px;
    }

    .time {
      margin-top: 4px;
      text-align: center;
      font-family: monospace;
      font-size: 36px;
      font-weight: bold;
    }

    .status {
      margin-top: 6px;
      text-align: center;
      font-size: 13px;
      font-weight: bold;
    }

    .preset-grid,
    .control-grid {
      display: grid;
      gap: 10px;
      margin-top: 16px;
    }

    .preset-grid {
      grid-template-columns: 1fr 1fr;
    }

    .control-grid {
      grid-template-columns: 1fr 1fr;
    }

    button {
      min-height: 48px;
      padding: 12px 10px;
      border: 0;
      border-radius: 14px;
      font-size: 14px;
      font-weight: bold;
      cursor: pointer;
      color: white;
      background: #265d59;
      box-shadow: 0 4px 0 #153a38;
      transition:
        transform 0.12s ease,
        box-shadow 0.12s ease,
        opacity 0.12s ease;
    }

    button:active {
      transform: translateY(3px);
      box-shadow: 0 1px 0 #153a38;
    }

    .start {
      background: #397b4f;
    }

    .pause {
      background: #d39434;
    }

    .resume {
      background: #3972a8;
    }

    .reset {
      background: #a94c59;
    }

    .custom-box {
      display: flex;
      gap: 10px;
      margin-top: 16px;
    }

    input {
      min-width: 0;
      flex: 1;
      padding: 12px;
      border: 3px solid #245955;
      border-radius: 12px;
      outline: none;
      font-size: 16px;
      background: rgba(255, 255, 255, 0.9);
    }

    .custom-button {
      min-width: 115px;
    }

    .counter {
      margin-top: 16px;
      padding: 12px;
      border-radius: 13px;
      text-align: center;
      font-size: 14px;
      font-weight: bold;
      background: rgba(255, 255, 255, 0.35);
    }

    .connection {
      margin-top: 12px;
      text-align: center;
      font-size: 11px;
      opacity: 0.7;
    }

    @media (max-width: 360px) {
      .control-grid {
        grid-template-columns: 1fr;
      }

      .custom-box {
        flex-direction: column;
      }

      .custom-button {
        width: 100%;
      }
    }
  </style>
</head>

<body>
  <main class="container">
    <section class="bmo-card">
      <h1 class="title">BMO</h1>
      <p class="subtitle">Pomodoro Control System</p>

      <div class="screen">
        <div id="face" class="face">(•‿•)</div>
        <div id="mode" class="mode">FOCUS MODE</div>
        <div id="time" class="time">25:00</div>
        <div id="status" class="status">Hazırım!</div>
      </div>

      <div class="preset-grid">
        <button onclick="setPreset('focus')">
          25 DK ODAK
        </button>

        <button onclick="setPreset('break')">
          5 DK MOLA
        </button>
      </div>

      <div class="control-grid">
        <button class="start" onclick="sendCommand('start')">
          BAŞLAT
        </button>

        <button class="pause" onclick="sendCommand('pause')">
          DURAKLAT
        </button>

        <button class="resume" onclick="sendCommand('resume')">
          DEVAM ET
        </button>

        <button class="reset" onclick="sendCommand('reset')">
          SIFIRLA
        </button>
      </div>

      <div class="custom-box">
        <input
          id="customMinutes"
          type="number"
          min="1"
          max="180"
          value="25"
          placeholder="Dakika"
        >

        <button
          class="custom-button"
          onclick="setCustomTime()"
        >
          SÜREYİ AYARLA
        </button>
      </div>

      <div class="counter">
        Bugün tamamlanan Pomodoro:
        <span id="completed">0</span>
      </div>

      <div id="connection" class="connection">
        BMO bağlantısı kontrol ediliyor...
      </div>
    </section>
  </main>

  <script>
    async function request(path) {
      try {
        const response = await fetch(path, {
          method: "GET",
          cache: "no-store"
        });

        if (!response.ok) {
          throw new Error("HTTP " + response.status);
        }

        await updateStatus();
      } catch (error) {
        document.getElementById("connection").textContent =
          "BMO ile bağlantı kurulamadı.";
      }
    }

    async function sendCommand(command) {
      await request("/api/" + command);
    }

    async function setPreset(mode) {
      await request("/api/preset?mode=" + mode);
    }

    async function setCustomTime() {
      const input =
        document.getElementById("customMinutes");

      const minutes = Number(input.value);

      if (
        !Number.isInteger(minutes) ||
        minutes < 1 ||
        minutes > 180
      ) {
        alert("1 ile 180 dakika arasında bir değer gir.");
        return;
      }

      await request(
        "/api/custom?minutes=" +
        encodeURIComponent(minutes)
      );
    }

    function getFace(state, mode) {
      if (state === "running") {
        return mode === "break" ? "( -‿- )" : "(•̀ᴗ•́)";
      }

      if (state === "paused") {
        return "( •_•)";
      }

      if (state === "finished") {
        return "\\(^o^)/";
      }

      return "(•‿•)";
    }

    function getStatusText(state) {
      switch (state) {
        case "running":
          return "BMO seninle çalışıyor!";
        case "paused":
          return "Sayaç duraklatıldı.";
        case "finished":
          return "Başardın! BMO seninle gurur duyuyor!";
        default:
          return "Hazırım!";
      }
    }

    async function updateStatus() {
      try {
        const response = await fetch("/api/status", {
          cache: "no-store"
        });

        if (!response.ok) {
          throw new Error("Durum alınamadı.");
        }

        const data = await response.json();

        document.getElementById("time").textContent =
          data.time;

        document.getElementById("mode").textContent =
          data.modeLabel;

        document.getElementById("status").textContent =
          getStatusText(data.state);

        document.getElementById("face").textContent =
          getFace(data.state, data.mode);

        document.getElementById("completed").textContent =
          data.completed;

        document.getElementById("connection").textContent =
          data.network;
      } catch (error) {
        document.getElementById("connection").textContent =
          "BMO bağlantısı bekleniyor...";
      }
    }

    updateStatus();
    setInterval(updateStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

// =====================================================
// YARDIMCI FONKSİYONLAR
// =====================================================

String timerStateToString() {
  switch (timerState) {
    case TIMER_RUNNING:
      return "running";

    case TIMER_PAUSED:
      return "paused";

    case TIMER_FINISHED:
      return "finished";

    default:
      return "ready";
  }
}

String timerModeToString() {
  switch (timerMode) {
    case MODE_BREAK:
      return "break";

    case MODE_CUSTOM:
      return "custom";

    default:
      return "focus";
  }
}

String timerModeLabel() {
  switch (timerMode) {
    case MODE_BREAK:
      return "BREAK MODE";

    case MODE_CUSTOM:
      return "CUSTOM TIMER";

    default:
      return "FOCUS MODE";
  }
}

String formatTime(unsigned long milliseconds) {
  unsigned long totalSeconds =
    (milliseconds + 999UL) / 1000UL;

  unsigned int minutes = totalSeconds / 60UL;
  unsigned int seconds = totalSeconds % 60UL;

  char buffer[10];

  snprintf(
    buffer,
    sizeof(buffer),
    "%02u:%02u",
    minutes,
    seconds
  );

  return String(buffer);
}

void centerText(
  const String& text,
  int y,
  int textSize
) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.setTextSize(textSize);

  display.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &width,
    &height
  );

  int x = (SCREEN_WIDTH - width) / 2;

  display.setCursor(x, y);
  display.print(text);
}

// =====================================================
// BMO YÜZ ÇİZİMLERİ
// =====================================================

void drawHappyFace() {
  // Gözler
  display.fillCircle(38, 19, 4, SSD1306_WHITE);
  display.fillCircle(90, 19, 4, SSD1306_WHITE);

  // Gülümseme
  display.drawLine(52, 30, 58, 35, SSD1306_WHITE);
  display.drawLine(58, 35, 70, 35, SSD1306_WHITE);
  display.drawLine(70, 35, 76, 30, SSD1306_WHITE);
}

void drawBlinkFace() {
  // Kapalı gözler
  display.drawLine(32, 19, 44, 19, SSD1306_WHITE);
  display.drawLine(84, 19, 96, 19, SSD1306_WHITE);

  // Gülümseme
  display.drawLine(52, 30, 58, 35, SSD1306_WHITE);
  display.drawLine(58, 35, 70, 35, SSD1306_WHITE);
  display.drawLine(70, 35, 76, 30, SSD1306_WHITE);
}

void drawFocusFace() {
  // Kaşlar
  display.drawLine(30, 13, 44, 17, SSD1306_WHITE);
  display.drawLine(84, 17, 98, 13, SSD1306_WHITE);

  // Gözler
  display.fillCircle(40, 22, 4, SSD1306_WHITE);
  display.fillCircle(88, 22, 4, SSD1306_WHITE);

  // Düz ağız
  display.drawLine(54, 35, 74, 35, SSD1306_WHITE);
}

void drawPausedFace() {
  // Gözler
  display.fillCircle(38, 20, 4, SSD1306_WHITE);
  display.fillCircle(90, 20, 4, SSD1306_WHITE);

  // Şaşkın ağız
  display.drawCircle(64, 34, 5, SSD1306_WHITE);
}

void drawBreakFace() {
  // Uykulu gözler
  display.drawLine(31, 20, 44, 22, SSD1306_WHITE);
  display.drawLine(84, 22, 97, 20, SSD1306_WHITE);

  // Rahat gülümseme
  display.drawLine(54, 32, 60, 36, SSD1306_WHITE);
  display.drawLine(60, 36, 68, 36, SSD1306_WHITE);
  display.drawLine(68, 36, 74, 32, SSD1306_WHITE);
}

void drawCelebrationFace() {
  if (celebrationFrame) {
    // Kollar
    display.drawLine(18, 32, 28, 22, SSD1306_WHITE);
    display.drawLine(100, 22, 110, 32, SSD1306_WHITE);

    // Gözler
    display.drawLine(32, 18, 38, 23, SSD1306_WHITE);
    display.drawLine(38, 23, 44, 18, SSD1306_WHITE);

    display.drawLine(84, 18, 90, 23, SSD1306_WHITE);
    display.drawLine(90, 23, 96, 18, SSD1306_WHITE);
  } else {
    // Kolların ikinci karesi
    display.drawLine(18, 22, 28, 30, SSD1306_WHITE);
    display.drawLine(100, 30, 110, 22, SSD1306_WHITE);

    // Gözler
    display.fillCircle(38, 20, 4, SSD1306_WHITE);
    display.fillCircle(90, 20, 4, SSD1306_WHITE);
  }

  // Büyük mutlu ağız
  display.drawLine(50, 31, 57, 38, SSD1306_WHITE);
  display.drawLine(57, 38, 71, 38, SSD1306_WHITE);
  display.drawLine(71, 38, 78, 31, SSD1306_WHITE);
}

// =====================================================
// OLED EKRANLARI
// =====================================================

void drawReadyScreen() {
  display.clearDisplay();

  if (eyesClosed) {
    drawBlinkFace();
  } else {
    drawHappyFace();
  }

  centerText(timerModeLabel(), 44, 1);
  centerText(formatTime(remainingTimeMs), 54, 1);

  display.display();
}

void drawRunningScreen() {
  display.clearDisplay();

  if (timerMode == MODE_BREAK) {
    drawBreakFace();
  } else {
    drawFocusFace();
  }

  centerText(timerModeLabel(), 43, 1);
  centerText(formatTime(remainingTimeMs), 53, 1);

  display.display();
}

void drawPausedScreen() {
  display.clearDisplay();

  drawPausedFace();

  centerText("PAUSED", 43, 1);
  centerText(formatTime(remainingTimeMs), 53, 1);

  display.display();
}

void drawFinishedScreen() {
  display.clearDisplay();

  drawCelebrationFace();

  if (timerMode == MODE_BREAK) {
    centerText("BREAK FINISHED!", 44, 1);
  } else {
    centerText("YOU DID IT!", 44, 1);
  }

  centerText("BMO IS PROUD!", 54, 1);

  display.display();
}

void updateDisplay() {
  switch (timerState) {
    case TIMER_RUNNING:
      drawRunningScreen();
      break;

    case TIMER_PAUSED:
      drawPausedScreen();
      break;

    case TIMER_FINISHED:
      drawFinishedScreen();
      break;

    default:
      drawReadyScreen();
      break;
  }
}

// =====================================================
// POMODORO KONTROLÜ
// =====================================================

void startTimer() {
  if (timerState == TIMER_FINISHED) {
    remainingTimeMs = timerDurationMs;
  }

  if (remainingTimeMs == 0) {
    remainingTimeMs = timerDurationMs;
  }

  timerState = TIMER_RUNNING;
  previousTimerUpdate = millis();

  updateDisplay();
}

void pauseTimer() {
  if (timerState != TIMER_RUNNING) {
    return;
  }

  unsigned long now = millis();
  unsigned long elapsed = now - previousTimerUpdate;

  if (elapsed >= remainingTimeMs) {
    remainingTimeMs = 0;
  } else {
    remainingTimeMs -= elapsed;
  }

  timerState = TIMER_PAUSED;

  updateDisplay();
}

void resumeTimer() {
  if (timerState != TIMER_PAUSED) {
    return;
  }

  timerState = TIMER_RUNNING;
  previousTimerUpdate = millis();

  updateDisplay();
}

void resetTimer() {
  timerState = TIMER_READY;
  remainingTimeMs = timerDurationMs;

  updateDisplay();
}

void setTimerDuration(
  unsigned long durationMinutes,
  TimerMode newMode
) {
  if (durationMinutes < 1) {
    durationMinutes = 1;
  }

  if (durationMinutes > 180) {
    durationMinutes = 180;
  }

  timerMode = newMode;

  timerDurationMs =
    durationMinutes * 60UL * 1000UL;

  remainingTimeMs = timerDurationMs;
  timerState = TIMER_READY;

  updateDisplay();
}

void finishTimer() {
  remainingTimeMs = 0;
  timerState = TIMER_FINISHED;

  finishedAnimationStarted = millis();

  if (timerMode == MODE_FOCUS) {
    completedPomodoros++;
  }

  updateDisplay();
}

void updateTimer() {
  if (timerState != TIMER_RUNNING) {
    return;
  }

  unsigned long now = millis();
  unsigned long elapsed = now - previousTimerUpdate;

  // Ekranı ve süreyi yaklaşık her saniye güncelle.
  if (elapsed < 1000UL) {
    return;
  }

  previousTimerUpdate = now;

  if (elapsed >= remainingTimeMs) {
    finishTimer();
    return;
  }

  remainingTimeMs -= elapsed;

  updateDisplay();
}

// =====================================================
// ANİMASYON KONTROLÜ
// =====================================================

void updateAnimations() {
  unsigned long now = millis();

  if (timerState == TIMER_READY) {
    if (
      !eyesClosed &&
      now - previousAnimationUpdate >= BLINK_INTERVAL
    ) {
      eyesClosed = true;
      previousAnimationUpdate = now;
      updateDisplay();
    } else if (
      eyesClosed &&
      now - previousAnimationUpdate >= BLINK_DURATION
    ) {
      eyesClosed = false;
      previousAnimationUpdate = now;
      updateDisplay();
    }
  }

  if (timerState == TIMER_FINISHED) {
    if (
      now - previousAnimationUpdate >= 300UL
    ) {
      celebrationFrame = !celebrationFrame;
      previousAnimationUpdate = now;
      updateDisplay();
    }
  }
}

// =====================================================
// JSON DURUM CEVABI
// =====================================================

String buildStatusJson() {
  String networkText;

  if (accessPointMode) {
    networkText =
      "BMO-Control | 192.168.4.1";
  } else {
    networkText =
      "Wi-Fi: " +
      WiFi.localIP().toString();
  }

  String json = "{";

  json += "\"state\":\"";
  json += timerStateToString();
  json += "\",";

  json += "\"mode\":\"";
  json += timerModeToString();
  json += "\",";

  json += "\"modeLabel\":\"";
  json += timerModeLabel();
  json += "\",";

  json += "\"time\":\"";
  json += formatTime(remainingTimeMs);
  json += "\",";

  json += "\"completed\":";
  json += String(completedPomodoros);
  json += ",";

  json += "\"network\":\"";
  json += networkText;
  json += "\"";

  json += "}";

  return json;
}

// =====================================================
// WEB ENDPOINTLERİ
// =====================================================

void handleRoot() {
  server.send_P(
    200,
    "text/html; charset=utf-8",
    INDEX_HTML
  );
}

void handleStatus() {
  server.send(
    200,
    "application/json; charset=utf-8",
    buildStatusJson()
  );
}

void handleStart() {
  startTimer();

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

void handlePause() {
  pauseTimer();

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

void handleResume() {
  resumeTimer();

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

void handleReset() {
  resetTimer();

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

void handlePreset() {
  if (!server.hasArg("mode")) {
    server.send(
      400,
      "application/json",
      "{\"success\":false,\"error\":\"mode missing\"}"
    );
    return;
  }

  String mode = server.arg("mode");

  if (mode == "focus") {
    setTimerDuration(25, MODE_FOCUS);
  } else if (mode == "break") {
    setTimerDuration(5, MODE_BREAK);
  } else {
    server.send(
      400,
      "application/json",
      "{\"success\":false,\"error\":\"invalid mode\"}"
    );
    return;
  }

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

void handleCustomTime() {
  if (!server.hasArg("minutes")) {
    server.send(
      400,
      "application/json",
      "{\"success\":false,\"error\":\"minutes missing\"}"
    );
    return;
  }

  int minutes = server.arg("minutes").toInt();

  if (minutes < 1 || minutes > 180) {
    server.send(
      400,
      "application/json",
      "{\"success\":false,\"error\":\"invalid minutes\"}"
    );
    return;
  }

  setTimerDuration(minutes, MODE_CUSTOM);

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}

void handleNotFound() {
  server.send(
    404,
    "application/json",
    "{\"success\":false,\"error\":\"not found\"}"
  );
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);

  server.on(
    "/api/status",
    HTTP_GET,
    handleStatus
  );

  server.on(
    "/api/start",
    HTTP_GET,
    handleStart
  );

  server.on(
    "/api/pause",
    HTTP_GET,
    handlePause
  );

  server.on(
    "/api/resume",
    HTTP_GET,
    handleResume
  );

  server.on(
    "/api/reset",
    HTTP_GET,
    handleReset
  );

  server.on(
    "/api/preset",
    HTTP_GET,
    handlePreset
  );

  server.on(
    "/api/custom",
    HTTP_GET,
    handleCustomTime
  );

  server.onNotFound(handleNotFound);

  server.begin();
}

// =====================================================
// WI-FI BAĞLANTISI
// =====================================================

void showWiFiConnectingScreen() {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  centerText("BMO IS WAKING", 13, 1);
  centerText("CONNECTING...", 30, 1);

  display.display();
}

void showNetworkScreen(
  const String& title,
  const String& address
) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  centerText(title, 6, 1);
  centerText(address, 24, 1);
  centerText("OPEN IN BROWSER", 45, 1);

  display.display();
}

void connectToWiFi() {
  showWiFiConnectingScreen();

  WiFi.mode(WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.println();
  Serial.print("Wi-Fi baglantisi deneniyor");

  unsigned long connectionStarted = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - connectionStarted < 15000UL
  ) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    accessPointMode = false;

    Serial.println();
    Serial.println("Wi-Fi baglandi.");

    Serial.print("BMO IP adresi: ");
    Serial.println(WiFi.localIP());

    showNetworkScreen(
      "BMO CONNECTED",
      WiFi.localIP().toString()
    );

    delay(3000);
    return;
  }

  Serial.println();
  Serial.println("Ev Wi-Fi agina baglanilamadi.");
  Serial.println("BMO kendi agini aciyor.");

  WiFi.disconnect(true);
  delay(300);

  WiFi.mode(WIFI_AP);

  bool apStarted = WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );

  accessPointMode = true;

  if (apStarted) {
    IPAddress apIP = WiFi.softAPIP();

    Serial.print("Ag adi: ");
    Serial.println(AP_SSID);

    Serial.print("Ag sifresi: ");
    Serial.println(AP_PASSWORD);

    Serial.print("Panel adresi: ");
    Serial.println(apIP);

    showNetworkScreen(
      "BMO-CONTROL",
      apIP.toString()
    );
  } else {
    Serial.println("BMO agi acilamadi.");

    showNetworkScreen(
      "NETWORK ERROR",
      "RESTART BMO"
    );
  }

  delay(3500);
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    )
  ) {
    Serial.println(
      "SSD1306 OLED bulunamadi."
    );

    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  centerText("HELLO!", 10, 2);
  centerText("I AM BMO", 37, 1);
  display.display();

  delay(2000);

  connectToWiFi();
  setupWebServer();

  previousAnimationUpdate = millis();

  updateDisplay();

  Serial.println();
  Serial.println("BMO hazir!");

  if (accessPointMode) {
    Serial.println(
      "Telefonundan BMO-Control agina baglan."
    );

    Serial.println(
      "Tarayicida 192.168.4.1 adresini ac."
    );
  } else {
    Serial.print("Tarayicida su adresi ac: http://");
    Serial.println(WiFi.localIP());
  }
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  server.handleClient();

  updateTimer();
  updateAnimations();

  delay(2);
}
