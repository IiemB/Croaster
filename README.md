# ☕ Croaster - Open Source Coffee Roaster Monitor

**Croaster** is a lightweight, open-source temperature monitoring system built on ESP-based microcontrollers. Designed for coffee roasting, it reads from two thermocouple sensors and displays real-time data on an OLED screen. Croaster also provides connectivity via WiFi (ESP8266/ESP32) and BLE (ESP32 only) for remote monitoring and control.


---

## 🚀 Features

* Supports **NodeMCU ESP8266** (WiFi only)
* Supports **ESP32C3 Super Mini** (WiFi & BLE)
* Real-time monitoring of **two MAX6675 sensors** (ET and BT)
* Visual output on a **128x64 OLED display** (SSD1306, I2C)
* WiFi communication via **WebSocket**, compatible with:
  + [**Artisan Roaster Scope**](https://artisan-scope.org/)
  + [**ICRM app**](https://iiemb.github.io/#/icrm)
* BLE communication (ESP32 only) for the [**ICRM app**](https://iiemb.github.io/#/icrm)
* Custom command system via a centralized `CommandHandler` class
* Easily extendable with user-defined commands

---

## 🧩 Hardware Components

* 1× [NodeMCU ESP8266](images/NodeMCU-ESP8266.png) or [ESP32C3 Super Mini](images/ESP32C3-Super-Mini.png)
* 1× [128x64 OLED display (SSD1306, I2C)](images/OLED-Display.png)
* 2× [MAX6675 thermocouple sensor modules](images/MAX6675.png)
* 2× [K-type thermocouple probes](images/Type-K-thermocouple.png)

---

## 🔌 Wiring Diagram

|  |**NodeMCU ESP8266**|**ESP32C3 Super Mini**|
|:---|:---:|:---:|
|**OLED Display**|GND →**GND**|GND → **GND**|
| |VCC → **3.3V**|VCC → **3.3V**|
| |SCL → **D1**|SCL → **GPIO9**|
| |SDA → **D2**|SDA → **GPIO8**|
|||⠀|
|**ET Sensor**|GND → **GND**|GND → **GND**|
| |VCC → **3.3V**|VCC → **3.3V**|
| |SCK → **D5**|SCK → **GPIO4**|
| |SO  → **D7**|SO  → **GPIO5**|
| |CS  → **D8**|CS  → **GPIO6**|
|||⠀|
|**BT Sensor**|GND → **GND**|GND → **GND**|
| |VCC → **3.3V**|VCC → **3.3V**|
| |SCK → **D5**|SCK → **GPIO4**|
| |SO  → **D7**|SO  → **GPIO5**|
| |CS  → **D6**|CS  → **GPIO7**|

---

## 🛠 Software Highlights

* Written in **C++** with the **PlatformIO** build system
* Modular architecture separating BLE, WebSocket, display, and sensor logic
* **CommandHandler** class:
  + Manages all incoming BLE/WebSocket JSON commands
  + Easily customizable for user-defined actions (e.g. `restart`,  `erase`, etc.)

---

## 🔧 How to Build and Upload

### ✅ PlatformIO (recommended for ESP8266)

1. Install [PlatformIO](https://platformio.org/)
2. Clone the repository:
   

```bash
   git clone git@github.com:IiemB/Croaster.git
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

* **WebSocket (WiFi):**
  + Connects with **Artisan Roaster Scope**
  + Also supports the **ICRM app** *(Android only)*

* **BLE (ESP32 only):**
  + Connects exclusively with the **ICRM app** *(Android only)*

---

## 📘 License

MIT License — free for personal and commercial use. Contributions welcome!

---

## ❤️ Contributing

Pull requests and feedback are welcome!
