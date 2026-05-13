# ☕ Croaster - Open Source Coffee Roaster Monitor

> 🇮🇩 Versi Bahasa Indonesia tersedia di [README_ID.md](README_ID.md)

**Croaster** is a lightweight, open-source temperature monitoring system built on ESP-based microcontrollers. Designed for coffee roasting enthusiasts and professionals, it reads from two thermocouple sensors (Bean Temperature and Environment Temperature) and displays real-time data on a compact OLED screen. Croaster connects seamlessly to popular roasting software via WiFi (WebSocket) and BLE (ESP32 only), making it compatible with both desktop and mobile roasting apps.

**Current Firmware Version:** `0.45`

---

## 📑 Table of Contents

- [Features](#-features)
- [Hardware Components](#-hardware-components)
- [Wiring Diagram](#-wiring-diagram)
- [Software Architecture](#-software-architecture)
- [Libraries & Dependencies](#-libraries--dependencies)
- [How to Build and Upload](#-how-to-build-and-upload)
- [WiFi Setup Guide](#-wifi-setup-guide)
- [Communication Overview](#-communication-overview)
- [How to Connect Croaster with Artisan](#-how-to-connect-croaster-with-artisan)
- [OTA (Over-The-Air) Updates](#-ota-over-the-air-updates)
- [Custom Commands](#-custom-commands)
- [License](#-license)
- [Contributing](#️-contributing)

---

## 🚀 Features

* Supports **NodeMCU ESP8266** (WiFi only)
* Supports **ESP32C3 Super Mini** (WiFi & BLE)
* Real-time monitoring of **two MAX6675 thermocouple sensors**:
  - **BT** — Bean Temperature (inside the drum)
  - **ET** — Environment Temperature (exhaust/inlet)
* **Rate of Rise (RoR)** calculation for both BT and ET, updated automatically
* Temperature unit switching: **Celsius** or **Fahrenheit**
* Configurable data send interval (default: every **3 seconds**)
* Built-in **temperature smoothing** (smoothing factor: 5) to reduce sensor noise
* Visual output on a **128×64 OLED display** (SSD1306, I2C)
* WiFi communication via **WebSocket** on port **81**, compatible with:
  + [**Artisan Roaster Scope**](https://artisan-scope.org/) — industry-standard roasting logger
  + [**ICRM app**](https://iiemb.github.io/#/icrm) — companion mobile app (Android)
* **BLE communication** (ESP32 only) for the [**ICRM app**](https://iiemb.github.io/#/icrm)
* **OTA (Over-The-Air) firmware updates** via WebSocket (ICRM app)
* **WiFiManager** captive portal for easy WiFi credential setup — no re-flashing needed
* Unique device naming based on chip ID (e.g. `Croaster-A1B2`)
* **Dummy mode** for development and testing without physical sensors
* Custom JSON command system via the centralized `CommandHandler` class
* Easily extendable with user-defined commands

---

## 🧩 Hardware Components

| Component | Description |
|:---|:---|
| 1× [NodeMCU ESP8266](images/NodeMCU-ESP8266.png) **or** [ESP32C3 Super Mini](images/ESP32C3-Super-Mini.png) | Main microcontroller |
| 1× [128×64 OLED display (SSD1306, I2C)](images/OLED-Display.png) | Real-time temperature display |
| 2× [MAX6675 thermocouple modules](images/MAX6675.png) | SPI-based K-type thermocouple ADC |
| 2× [K-type thermocouple probes](images/Type-K-thermocouple.png) | Temperature probes (BT & ET) |

> All components run on **3.3V**. Ensure your power supply can handle the combined current draw of both sensors and the display.

---

## 🔌 Wiring Diagram

|  |**NodeMCU ESP8266**|**ESP32C3 Super Mini**|
|:---|:---:|:---:|
|**OLED Display**|GND → **GND**|GND → **GND**|
| |VCC → **3.3V**|VCC → **3.3V**|
| |SCL → **D1**|SCL → **GPIO9**|
| |SDA → **D2**|SDA → **GPIO8**|
|||⠀|
|**ET Sensor** (Environment Temp)|GND → **GND**|GND → **GND**|
| |VCC → **3.3V**|VCC → **3.3V**|
| |SCK → **D5**|SCK → **GPIO4**|
| |SO  → **D7**|SO  → **GPIO5**|
| |CS  → **D8**|CS  → **GPIO6**|
|||⠀|
|**BT Sensor** (Bean Temp)|GND → **GND**|GND → **GND**|
| |VCC → **3.3V**|VCC → **3.3V**|
| |SCK → **D5**|SCK → **GPIO4**|
| |SO  → **D7**|SO  → **GPIO5**|
| |CS  → **D6**|CS  → **GPIO7**|

> Both sensors share the **SCK** and **SO** lines (SPI bus). They are distinguished by their individual **CS** pins.

---

## 🛠 Software Architecture

Croaster uses a clean, **modular C++ architecture** built with the Arduino framework. Each subsystem is encapsulated in its own class:

| Module | File | Responsibility |
|:---|:---|:---|
| `CroasterCore` | `CroasterCore.h/.cpp` | Sensor reading, RoR calculation, temperature smoothing, data state |
| `DisplayManager` | `DisplayManager.h/.cpp` | OLED rendering loop, status screens |
| `CommandHandler` | `CommandHandler.h/.cpp` | JSON command parsing and dispatching (BLE & WebSocket) |
| `WebSocketManager` | `WebSocketManager.h/.cpp` | WebSocket server, data broadcast, OTA trigger |
| `BleManager` | `BleManager.h/.cpp` | BLE server, characteristic notify, command receive *(ESP32 only)* |
| `OtaHandler` | `OtaHandler.h/.cpp` | Binary OTA update handling over WebSocket |
| `WiFiManagerUtil` | `WiFiManagerUtil.h/.cpp` | WiFiManager captive portal setup and lifecycle |
| `DeviceIdentity` | `DeviceIdentity.h/.cpp` | Chip ID, device name, IP address helpers |

### Data Flow

```
MAX6675 Sensors → CroasterCore (read + smooth + RoR)
                       ↓
          ┌────────────┴────────────┐
    WebSocketManager           BleManager (ESP32)
          ↓                         ↓
   Artisan / ICRM              ICRM (Android)
```

---

## 📦 Libraries & Dependencies

| Library | Purpose |
|:---|:---|
| [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) | WebSocket server |
| [ArduinoJson](https://arduinojson.org/) `^7.4.3` | JSON command parsing and serialization |
| [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) `^2.5.16` | OLED display driver |
| [MAX6675_Thermocouple](https://github.com/YuriiSalimov/MAX6675_Thermocouple) `^2.0.2` | Thermocouple sensor reading |
| [WiFiManager](https://github.com/tzapu/WiFiManager) `^2.0.17` | Captive portal WiFi setup |
| ESP32 BLE Arduino *(built-in ESP32 core)* | BLE server & characteristics |

---

## 🔧 How to Build and Upload

### ✅ PlatformIO (recommended for ESP8266 & ESP32C3)

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
2. Clone the repository:

   ```bash
   git clone git@github.com:IiemB/Croaster.git
   cd Croaster
   ```

3. Review `platformio.ini` and select your target environment
4. Upload the firmware:

   ```bash
   # For ESP8266
   pio run -e esp8266 -t upload

   # For ESP32C3
   pio run -e esp32c3 -t upload
   ```

> **Note:** ESP32C3 Super Mini uses a custom partition scheme (`custom32c3sm.csv`) to maximize app storage. See [references.md](references.md) for setup details.

### ✅ Arduino IDE (alternative, required for Makergo ESP32C3 board)

> Arduino IDE is required if you use the **Makergo ESP32C3 SuperMini** board definition, which is not yet fully supported by PlatformIO.

1. Run the conversion script to copy source files into the Arduino sketch folder:

   ```bash
   ./copy_to_ino.sh
   ```

2. Open the `croaster-arduino/` folder in **Arduino IDE 2.x**
3. Select your board:
   - **ESP8266** → `NodeMCU 1.0 (ESP-12E Module)`
   - **ESP32C3** → `Makergo ESP32C3 SuperMini`
4. For **ESP32C3**, select the partition scheme:
   - Use `Huge APP` for maximum sketch size (OTA not supported)
   - Use `Custom SuperMini` for OTA support (see [references.md](references.md) for setup)

   > [!NOTE]
   > The `Huge APP` partition does **not** support OTA via the ICRM App. To enable OTA, follow the custom partition steps in [references.md](references.md).

5. Install all required libraries via Arduino Library Manager (see [Libraries & Dependencies](#-libraries--dependencies))
6. Upload via `Sketch → Upload`

---

## 🔗 WiFi Setup Guide

Croaster uses **WiFiManager** to handle WiFi credentials without re-flashing. On first boot (or after erasing credentials), Croaster creates its own access point:

1. On your phone or computer, connect to the WiFi network named `[XXXX] Croaster-XXXX`
2. A captive portal will open automatically — enter your home WiFi SSID and password
3. Croaster will save the credentials and connect automatically on subsequent boots
4. The IP address assigned to Croaster is shown on the OLED display

For a visual walkthrough, see: ➡️ [How to Connect to WiFi - YouTube](https://www.youtube.com/watch?v=esNiudoCEcU&t=434s)

---

## 📡 Communication Overview

### WebSocket (WiFi)

- **Port:** `81`
- **Protocol:** WebSocket (text frames for JSON commands, binary frames for OTA)
- **Data format:** JSON, broadcast every `intervalSend` seconds (default: 3s)
- Compatible with **Artisan Roaster Scope** and **ICRM app** (Android)

### BLE (ESP32 only)

- **Service UUID:** `1cc9b045-a6e9-4bd5-b874-07d4f2d57843`
- **Data Characteristic UUID:** `d56d0059-ad65-43f3-b971-431d48f89a69`
- Supports notify (data push) and write (command receive)
- Compatible with the **ICRM app** (Android only)

---

## 🔌 How to Connect Croaster with Artisan

You can connect your Croaster device to Artisan using either a direct WiFi connection or through your home/local WiFi network.

### 🖥️ Option 1: Direct Connection (Croaster as Access Point)

Use this method when Croaster is **not** connected to any WiFi network, or when you want a direct peer-to-peer connection.

1. On your computer, connect to the WiFi network broadcasted by Croaster (e.g. `[XXXX] Croaster-XXXX`)
2. Open Artisan → **Config → Port**
3. Set the configuration as shown below:

   ![image](images/Connect-Artisan-Directly.png)

### 🌐 Option 2: Same WiFi Network (Croaster joins your WiFi)

Use this method when Croaster is already connected to your home/office WiFi network.

1. Make sure your laptop and Croaster are on the **same WiFi network**
2. Open Artisan → **Config → Port**
3. Enter the **IP address** shown on the Croaster OLED display (or via serial monitor)
4. Set the configuration as shown:

   ![image](images/Connect-Artisan-Same-Network.png)

---

## ⬆️ OTA (Over-The-Air) Updates

Croaster supports firmware updates without a USB cable, via the **ICRM app** over WebSocket (WiFi).

- OTA is handled by the `OtaHandler` class, which receives binary firmware data in chunks
- Progress is shown on the OLED display during the update
- OTA requires the **custom partition scheme** (`custom32c3sm`) on ESP32C3 — the `Huge APP` partition does **not** support OTA
- After a successful OTA update, Croaster restarts automatically

---

## 🧪 Custom Commands

Croaster accepts JSON-formatted commands over both WebSocket and BLE. The `CommandHandler` class dispatches all incoming commands.

### Built-in Commands

| Command JSON | Action |
|:---|:---|
| `{"cmd": "restart"}` | Restarts the device |
| `{"cmd": "erase"}` | Erases WiFi credentials and restarts |
| `{"cmd": "unit", "value": "F"}` | Switches temperature unit to Fahrenheit |
| `{"cmd": "unit", "value": "C"}` | Switches temperature unit to Celsius |
| `{"cmd": "interval", "value": 5}` | Sets data send interval to 5 seconds |
| `{"cmd": "correction", "bt": 1.5, "et": -0.5}` | Applies temperature correction offset |

### Adding Custom Commands

To add your own command, edit `CommandHandler.cpp` and add a new condition inside the `handleJsonCommand` method. The handler receives a parsed `JsonObject`, so you can read any key/value from the JSON payload.

---

## 📘 License

[MIT License](LICENSE.md) — free for personal and commercial use. Contributions welcome!

---

## ❤️ Contributing

Pull requests, bug reports, and feature requests are welcome! Feel free to open an issue or submit a PR on [GitHub](https://github.com/IiemB/Croaster).

---

## 🔗 Related Links

- [ICRM App](https://iiemb.github.io/#/icrm) — companion Android app for Croaster
- [Artisan Roaster Scope](https://artisan-scope.org/) — open-source coffee roasting logger
- [WiFi Setup Video](https://www.youtube.com/watch?v=esNiudoCEcU&t=434s) — quick visual guide
- [References & Advanced Setup](references.md) — custom partitions, OTA, PlatformIO tips
- [FAQ](FAQ.md) — frequently asked questions
