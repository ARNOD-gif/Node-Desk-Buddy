# 🤖 Node Desk Buddy // ISHT-X OS V_11

An interactive, Cyberpunk-themed desktop companion powered by the **ESP32-C3 SuperMini**. Features an expressive OLED display, gesture/motion sensing, real-time weather and clock synchronization, a custom web dashboard, and wireless Over-The-Air (OTA) firmware updates.

> **Created by - Arnod**

---

## 🛠️ Components Used

### Core Electronics
* **Microcontroller:** ESP32-C3 SuperMini (RISC-V architecture, onboard Wi-Fi + Bluetooth)
* **Display:** 0.96-inch Monochrome I2C OLED Display (SSD1306, $128 \times 64$ resolution)
* **Motion Sensor:** MPU6050 6-Axis Accelerometer & Gyroscope Module
* **Touch Sensor:** TTP223 Capacitive Touch Button Switch Module
* **Power Management:**
  * 800mAh 3.7V Rechargeable Lithium Polymer (LiPo) Battery
  * TP4056 Micro-USB / Type-C Lithium Battery Charging Module with protection
  * SPDT Slide Switch (Power ON/OFF Toggle)

### Hardware Interconnects
* Shared I2C Bus Architecture (SDA: GPIO 8, SCL: GPIO 9)
* Custom 3D-printed enclosure chassis or acrylic mounting plate
* 28 AWG Silicone jumper wires

---

## ✨ Features

* **18 Dynamic Expressions:** Rendered vector facial animations (Normal, Happy, Angry, Dizzy, Cute, Love, Sleeping, and more).
* **Motion Reactive:** Integrated MPU6050 triggers a spinning "Dizzy" animation when shaken.
* **Capacitive Touch Gestures:**
  * **1 Tap:** Cycle through facial expressions / Return Home
  * **2 Taps:** Live Weather Mode
  * **3 Taps:** Date & Time Sync Display
  * **4 Taps:** On-screen User Manual
  * **Hold (1 Second):** Hardware System Telemetry
* **Personality Engine:** Multi-stage idle progression (Bored $\rightarrow$ Frustrated $\rightarrow$ Sleep Mode $\rightarrow$ Custom Alert Message).
* **Cyberpunk Web Dashboard:** Integrated web interface to control expressions, adjust settings, and update owner configuration remotely.
* **Wireless Firmware Updates (Web OTA):** Flash new compiled `.bin` updates over Wi-Fi without needing a USB cable.

---

## 🔌 Circuit Pinout & Wiring

| Component | ESP32-C3 Pin | Notes |
| :--- | :--- | :--- |
| **SSD1306 OLED SDA** | GPIO 8 | Shared I2C Data Line |
| **SSD1306 OLED SCL** | GPIO 9 | Shared I2C Clock Line |
| **MPU6050 SDA** | GPIO 8 | Shared I2C Data Line |
| **MPU6050 SCL** | GPIO 9 | Shared I2C Clock Line |
| **TTP223 Touch Input** | GPIO 7 | Capacitive Touch Interrupt Line |
| **VCC (All Modules)** | 3.3V / 5V | Power Rail |
| **GND (All Modules)** | GND | Common Ground Rail |

---

## 💻 Software Prerequisites & Libraries

To compile and upload the firmware using the Arduino IDE, install the following libraries via the **Library Manager** (`Ctrl + Shift + I`):

1. `Adafruit_GFX`
2. `Adafruit_SSD1306`
3. `Adafruit_MPU6050`
4. `Adafruit_Sensor`
5. `ElegantOTA`
6. `WiFi` (Built-in ESP32 core)
7. `WebServer` (Built-in ESP32 core)
8. `Preferences` (Built-in ESP32 core)

---

## 🚀 Setup & Installation Instructions

### 1. Flashing Firmware
1. Open `firmware/Node_Firmware_V11.ino` in Arduino IDE.
2. Select **ESP32C3 Dev Module** under **Tools > Board > ESP32 Arduino**.
3. Update your Wi-Fi credentials inside the `knownNetworks` array:
   ```cpp
   WifiNetwork knownNetworks[] = {
     {"YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD"}
   };