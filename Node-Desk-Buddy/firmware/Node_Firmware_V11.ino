/*
  ===========================================================================
  PROJECT : NODE DESK BUDDY
  OS      : ISHT-X OS V_11
  AUTHOR  : Arnod
  BOARD   : ESP32-C3 SuperMini
  ===========================================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTA.h>

// ---------------------------------------------------------------------------
// PIN DEFINITIONS & HARDWARE CONSTANTS
// ---------------------------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

#define SDA_PIN 8
#define SCL_PIN 9
#define TOUCH_PIN 7

// Wi-Fi Credentials (Update with your network details)
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// Hardware Instances
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MPU6050 mpu;
WebServer server(80);

// System Variables
String ownerName = "Arnod";
int currentExpression = 0; // 0: Normal, 1: Happy, 2: Angry, 3: Dizzy, 4: Sleep
int systemMode = 0;        // 0: Face, 1: Date/Time, 2: Weather, 3: Manual, 4: System Info
unsigned long lastTouchTime = 0;
unsigned long lastMotionTime = 0;
int touchTapCount = 0;
bool isShakeDetected = false;

// ---------------------------------------------------------------------------
// WEB DASHBOARD HTML (STORED IN FLASH MEMORY)
// ---------------------------------------------------------------------------
const char HTML_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>NODE // ISHT-X OS V_11</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body{background:#0a0c10;color:#00f0ff;font-family:monospace;padding:20px;text-align:center;}
  .card{background:#12161f;border:1px solid #1f293d;padding:15px;margin:10px auto;max-width:400px;border-radius:8px;}
  h1{color:#ff0055;font-size:1.5rem;}
  button{background:#00f0ff;color:#000;border:none;padding:10px 15px;margin:5px;font-weight:bold;cursor:pointer;border-radius:4px;}
  button:hover{background:#ff0055;color:#fff;}
  input{background:#0d111a;border:1px solid #00f0ff;color:#fff;padding:8px;width:80%;margin-bottom:10px;}
</style></head><body>
  <div class="card">
    <h1>NODE // ISHT-X OS V_11</h1>
    <p>CREATED BY: <b>ARNOD</b></p>
  </div>
  <div class="card">
    <h3>EXPRESSION CONTROL</h3>
    <button onclick="setFace(0)">NORMAL</button>
    <button onclick="setFace(1)">HAPPY</button>
    <button onclick="setFace(2)">ANGRY</button>
    <button onclick="setFace(3)">DIZZY</button>
    <button onclick="setFace(4)">SLEEP</button>
  </div>
  <div class="card">
    <h3>OWNER SETTINGS</h3>
    <input type="text" id="owner" placeholder="New Owner Name">
    <button onclick="updateOwner()">SAVE</button>
  </div>
  <div class="card">
    <h3>FIRMWARE UPDATE</h3>
    <a href="/update"><button style="background:#ff0055;color:#fff;">OPEN OTA FLASHER</button></a>
  </div>
<script>
  function setFace(id){ fetch('/setFace?id='+id); }
  function updateOwner(){ 
    let val = document.getElementById('owner').value;
    if(val) fetch('/setOwner?name='+encodeURIComponent(val));
  }
</script></body></html>
)rawliteral";

// ---------------------------------------------------------------------------
// DRAWING FACIAL EXPRESSIONS ON OLED
// ---------------------------------------------------------------------------
void drawHeader(const char* modeTitle) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("NODE");
  
  display.setCursor(40, 0);
  display.print(modeTitle);

  // WiFi status icon indicator
  display.setCursor(110, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WIFI");
  } else {
    display.print(" OFF");
  }
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
}

void renderFace(int faceId) {
  display.clearDisplay();
  drawHeader("ISHT-X");

  switch (faceId) {
    case 0: // Normal
      display.fillRoundRect(25, 25, 25, 25, 5, SSD1306_WHITE);
      display.fillRoundRect(78, 25, 25, 25, 5, SSD1306_WHITE);
      break;

    case 1: // Happy (^ ^)
      display.drawCircle(37, 35, 12, SSD1306_WHITE);
      display.drawCircle(90, 35, 12, SSD1306_WHITE);
      display.fillRect(20, 35, 35, 15, SSD1306_BLACK);
      display.fillRect(73, 35, 35, 15, SSD1306_BLACK);
      break;

    case 2: // Angry
      display.fillRoundRect(25, 30, 25, 20, 4, SSD1306_WHITE);
      display.fillRoundRect(78, 30, 25, 20, 4, SSD1306_WHITE);
      display.drawLine(20, 20, 52, 32, SSD1306_WHITE); // Left Eyebrow
      display.drawLine(108, 20, 76, 32, SSD1306_WHITE); // Right Eyebrow
      break;

    case 3: // Dizzy (X X)
      // Left X
      display.drawLine(25, 25, 50, 50, SSD1306_WHITE);
      display.drawLine(50, 25, 25, 50, SSD1306_WHITE);
      // Right X
      display.drawLine(78, 25, 103, 50, SSD1306_WHITE);
      display.drawLine(103, 25, 78, 50, SSD1306_WHITE);
      break;

    case 4: // Sleeping (- -)
      display.drawLine(25, 35, 50, 35, SSD1306_WHITE);
      display.drawLine(78, 35, 103, 35, SSD1306_WHITE);
      display.setCursor(110, 15);
      display.print("Zz");
      break;
  }
  display.display();
}

void renderSystemInfo() {
  display.clearDisplay();
  drawHeader("SYS INFO");
  display.setCursor(0, 15);
  display.print("OS: ISHT-X V_11");
  display.setCursor(0, 27);
  display.print("OWNER: "); display.print(ownerName);
  display.setCursor(0, 39);
  display.print("IP: "); display.print(WiFi.localIP().toString());
  display.setCursor(0, 51);
  display.print("BY: ARNOD");
  display.display();
}

// ---------------------------------------------------------------------------
// WEB SERVER HANDLERS
// ---------------------------------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", HTML_INDEX);
}

void handleSetFace() {
  if (server.hasArg("id")) {
    currentExpression = server.arg("id").toInt();
    systemMode = 0; // Switch to face view
    renderFace(currentExpression);
  }
  server.send(200, "text/plain", "OK");
}

void handleSetOwner() {
  if (server.hasArg("name")) {
    ownerName = server.arg("name");
  }
  server.send(200, "text/plain", "OK");
}

// ---------------------------------------------------------------------------
// INITIAL SETUP
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Hardware Pins & I2C
  pinMode(TOUCH_PIN, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED Allocation Failed"));
    for (;;);
  }

  // Startup Animation
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print("ISHT-X OS V_11");
  display.setCursor(10, 35);
  display.print("BY - ARNOD");
  display.drawRect(10, 50, 108, 6, SSD1306_WHITE);
  display.display();

  for (int i = 0; i <= 104; i += 8) {
    display.fillRect(12, 52, i, 2, SSD1306_WHITE);
    display.display();
    delay(40);
  }

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 Init Failed");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  }

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  // Set up Web Server
  server.on("/", handleRoot);
  server.on("/setFace", handleSetFace);
  server.on("/setOwner", handleSetOwner);

  // Start Over-The-Air Update Engine
  ElegantOTA.begin(&server);
  server.begin();

  renderFace(currentExpression);
}

// ---------------------------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------------------------
void loop() {
  server.handleClient();
  ElegantOTA.loop();

  // --- TOUCH SENSOR HANDLING ---
  bool touchState = digitalRead(TOUCH_PIN);
  if (touchState == HIGH && (millis() - lastTouchTime > 300)) {
    lastTouchTime = millis();
    touchTapCount++;
  }

  // Process multi-tap gesture after a brief pause
  if (touchTapCount > 0 && (millis() - lastTouchTime > 400)) {
    if (touchTapCount == 1) {
      currentExpression = (currentExpression + 1) % 5;
      systemMode = 0;
      renderFace(currentExpression);
    } else if (touchTapCount >= 3) {
      systemMode = 4; // Long tap / Multi tap opens System Info
      renderSystemInfo();
    }
    touchTapCount = 0;
  }

  // --- MPU6050 MOTION / SHAKE DETECTION ---
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x +
                              a.acceleration.y * a.acceleration.y +
                              a.acceleration.z * a.acceleration.z);

  // Trigger "Dizzy" expression if shaken forcefully (>22 m/s²)
  if (accelMagnitude > 22.0 && systemMode == 0 && currentExpression != 3) {
    currentExpression = 3; // Set to Dizzy
    renderFace(currentExpression);
    lastMotionTime = millis();
  }

  // Reset face back to Normal after 4 seconds of quiet after a shake
  if (currentExpression == 3 && (millis() - lastMotionTime > 4000)) {
    currentExpression = 0;
    renderFace(currentExpression);
  }

  // --- IDLE SLEEP TRAIT (After 30 Seconds) ---
  if (millis() - lastTouchTime > 30000 && currentExpression != 4 && systemMode == 0) {
    currentExpression = 4; // Set to Sleeping
    renderFace(currentExpression);
  }

  delay(20);
}