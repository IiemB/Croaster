# ☕ Croaster - Open Source Coffee Roaster Monitor

**Croaster** is a lightweight, open-source temperature monitoring system built on ESP-based microcontrollers. Designed for coffee roasting, it reads from two thermocouple sensors and displays real-time data on an OLED screen. Croaster also provides connectivity via WiFi (ESP8266/ESP32) and BLE (ESP32 only) for remote monitoring and control.

📄 [Lihat versi Bahasa Indonesia](README_ID.md)

---

## 🚀 Features

- Supports **NodeMCU ESP8266** (WiFi only)
- Supports **ESP32C3 Super Mini** (WiFi & BLE)
- Real-time monitoring of **two MAX6675 sensors** (ET and BT)
- Visual output on a **128x64 OLED display** (SSD1306, I2C)
- WiFi communication via **WebSocket**, compatible with:
  - **Artisan Roaster Scope**
  - **ICRM app** *(Android only)*
- BLE communication (ESP32 only) for the **ICRM app** *(Android only)*
- Custom command system via a centralized `CommandHandler` class
- Easily extendable with user-defined commands

---

## 🧩 Hardware Components

- 1× NodeMCU ESP8266 or ESP32C3 Super Mini
- 1× 128x64 OLED display (SSD1306, I2C)
- 2× MAX6675 thermocouple sensor modules
- 2× K-type thermocouple probes

---

## 🔌 Wiring Diagram

### NodeMCU ESP8266

#### OLED Display
- GND → GND
- VCC → 3.3V
- SCL → **D1**
- SDA → **D2**

#### ET Sensor
- SCK → **D5**
- SO  → **D7**
- CS  → **D6**

#### BT Sensor
- SCK → **D5** *(shared)*
- SO  → **D7** *(shared)*
- CS  → **D8**

### ESP32C3 Super Mini

#### OLED Display
- GND → GND
- VCC → 3.3V
- SCL → **GPIO4**
- SDA → **GPIO5**

#### ET Sensor
- SCK → **GPIO4**
- SO  → **GPIO5**
- CS  → **GPIO6**

#### BT Sensor
- SCK → **GPIO4** *(shared)*
- SO  → **GPIO5** *(shared)*
- CS  → **GPIO7**

---

## 🛠 Software Highlights

- Written in **C++** with the **PlatformIO** build system
- Modular architecture separating BLE, WebSocket, display, and sensor logic
- **CommandHandler** class:
  - Manages all incoming BLE/WebSocket JSON commands
  - Easily customizable for user-defined actions (e.g. `restart`, `erase`, etc.)

---

## 🔧 How to Build and Upload

### ✅ PlatformIO (recommended for ESP8266)

1. Install [PlatformIO](https://platformio.org/)
2. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/croaster.git
   cd croaster
   ```
3. Select your board in `platformio.ini` (ESP8266 only)
4. Upload the firmware:
   ```bash
   pio run -t upload
   ```

### ✅ Arduino IDE (required for ESP32C3)

1. Run the conversion script:
   ```bash
   ./copy_to_ino.sh
   ```
2. Open `croaster-arduino` folder in **Arduino IDE**
3. Select your board:
   - ESP8266 → **NodeMCU 1.0 (ESP-12E)**
   - ESP32C3 → **Makergo ESP32C3** *(not yet supported by PlatformIO)*

---

## 🔗 WiFi Setup Guide

To connect Croaster to your WiFi network, you can follow this quick video guide: ➡️ [How to Connect to WiFi - YouTube](https://www.youtube.com/watch?v=esNiudoCEcU\&t=434s)

---

## 📡 Communication Overview

- **WebSocket (WiFi):**
  - Connects with **Artisan Roaster Scope**
  - Also supports the **ICRM app** *(Android only)*

- **BLE (ESP32 only):**
  - Connects exclusively with the **ICRM app** *(Android only)*

---

## 📘 License

MIT License — free for personal and commercial use. Contributions welcome!

---

## ❤️ Contributing

Pull requests and feedback are welcome!