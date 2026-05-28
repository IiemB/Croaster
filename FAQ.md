# ❓ Croaster FAQ — Frequently Asked Questions

> 🇮🇩 Versi Bahasa Indonesia tersedia di [FAQ_ID.md](FAQ_ID.md)

---

## 📦 General

### What is Croaster?

Croaster is an open-source firmware for ESP8266 and ESP32C3-based microcontrollers that turns your hardware into a coffee roasting temperature monitor. It reads two thermocouple sensors (Bean Temperature and Environment Temperature), calculates Rate of Rise (RoR), and streams data over WiFi (WebSocket) or BLE to compatible apps like Artisan Roaster Scope and ICRM.

---

### Which microcontrollers are supported?

| Board | WiFi | BLE | OTA |
|:---|:---:|:---:|:---:|
| NodeMCU ESP8266 | ✅ | ❌ | ✅ |
| ESP32C3 Super Mini | ✅ | ✅ | ✅ |

---

### What apps can I use with Croaster?

- **[Artisan Roaster Scope](https://artisan-scope.org/)** — connects via WebSocket (WiFi). Available for Windows, macOS, and Linux.
- **[ICRM App](https://iiemb.github.io/#/icrm)** — connects via WebSocket (WiFi) or BLE (ESP32 only). Available for Android.

---

### Is Croaster free to use?

Yes. Croaster is released under the **MIT License**, which means it is free for both personal and commercial use. You can modify, distribute, and build upon it freely.

---

## 🔌 Hardware

### What sensors does Croaster use?

Croaster uses two **MAX6675** thermocouple ADC modules with **K-type thermocouple probes**:
- **BT (Bean Temperature)** — placed inside the roasting drum to measure bean temperature
- **ET (Environment Temperature)** — placed at the exhaust or inlet to measure airflow temperature

---

### Can I use only one sensor?

The firmware is designed to read two sensors. If you connect only one, the missing sensor will likely report `0` or erratic values. You can modify `CroasterCore.cpp` to handle a single-sensor setup, but this is not officially supported.

---

### What is the operating voltage for the components?

All components (OLED display, MAX6675 modules, and the microcontroller) operate at **3.3V**. Do not connect any of these to 5V directly, as it may damage the components.

---

### Do both sensors share the same SPI pins?

Yes. Both MAX6675 sensors share the **SCK** and **SO** SPI lines. They are differentiated by individual **CS (Chip Select)** pins:
- **BT sensor CS:** `D8` (ESP8266) / `GPIO7` (ESP32C3)
- **ET sensor CS:** `D6` (ESP8266) / `GPIO6` (ESP32C3)

---

## 📶 WiFi & Connectivity

### How do I connect Croaster to my WiFi network?

On first boot, Croaster creates its own WiFi access point named `[XXXX] Croaster-XXXX`. Connect to it from your phone or computer, and a captive portal page will open. Enter your home WiFi credentials there — Croaster will save them and connect automatically on future boots.

See the [WiFi Setup Video](https://www.youtube.com/watch?v=esNiudoCEcU&t=434s) for a visual guide.

---

### I forgot my WiFi password or want to change the network. How do I reset?

Send the command `{"command": "erase"}` via WebSocket or BLE (ICRM app), or press the reset button while holding the boot button on the ESP32C3. This erases saved WiFi credentials and puts Croaster back into access point mode for reconfiguration.

---

### Can I use Croaster without a WiFi network?

Yes. When Croaster is not connected to any WiFi network, it broadcasts its own access point. You can connect Artisan directly to that access point. See [Option 1 in the README](README.md#-option-1-direct-connection-croaster-as-access-point).

---

### What is the WebSocket port?

Croaster's WebSocket server runs on **port 81**.

---

### How often does Croaster send data?

By default, data is broadcast every **3 seconds**. You can change this by sending:
```json
{"command": {"interval": 5}}
```
Replace `5` with your desired interval in seconds.

---

### Is BLE available on ESP8266?

No. BLE is only available on the **ESP32** platform. The ESP8266 only supports WiFi.

---

## 🌡️ Temperature & Sensors

### What temperature units are supported?

Croaster supports **Celsius (C)** and **Fahrenheit (F)**. Switch units by sending:
```json
{"command": {"tempUnit": "F"}}
```
or
```json
{"command": {"tempUnit": "C"}}
```

---

### What is Rate of Rise (RoR)?

Rate of Rise (RoR) measures how fast the temperature is increasing, expressed in **degrees per minute**. Croaster calculates RoR for both BT and ET automatically based on recent sensor history. RoR is a critical metric in coffee roasting for tracking the development phase.

---

### My sensor readings seem noisy or unstable. What can I do?

Croaster applies a **smoothing filter** (factor: 5) to reduce noise from the MAX6675 sensors. If readings are still unstable, check your wiring, ensure the thermocouple probes are securely seated in the MAX6675 modules, and make sure the VCC supply is stable at 3.3V.

---

### Can I apply a temperature correction/offset?

Yes. Send a correction command:
```json
{"command": {"correctionBt": 1.5, "correctionEt": -0.5}}
```
This applies a `+1.5°` offset to BT and a `-0.5°` offset to ET. The correction is applied on top of the smoothed sensor reading.

---

## 🔧 Build & Upload

### Which build method should I use?

| Scenario | Recommended Method |
|:---|:---|
| ESP8266 | PlatformIO or Arduino IDE |
| ESP32C3 (with OTA support) | Arduino IDE with Custom SuperMini partition |
| ESP32C3 (without OTA, max sketch size) | Arduino IDE with Huge APP partition |

---

### Why can't I use PlatformIO for the ESP32C3 Super Mini?

The **Makergo ESP32C3 SuperMini** board definition is not officially available in the PlatformIO board registry. You can add it manually using the community config from [this repository](https://github.com/sigmdel/supermini_esp32c3_sketches.git), but the officially supported method is to use **Arduino IDE** with the Makergo board package.

---

### What is `copy_to_ino.sh`?

This is a shell script that copies the source files from the `src/` directory (PlatformIO structure) into the `croaster-arduino/` folder with the correct Arduino sketch naming convention. Run it before opening the project in Arduino IDE.

---

### What is the `custom32c3sm.csv` file?

It is a **custom partition table** for the ESP32C3 Super Mini. This partition layout allocates more storage for the application (`1900544` bytes) while still reserving space for OTA updates. Without it, OTA updates cannot coexist with a large firmware binary. See [references.md](references.md) for installation steps.

---

## ⬆️ OTA Updates

### How do I update the firmware over the air?

Use the **ICRM app** on Android. The app will send the compiled firmware binary over WebSocket. Croaster receives it in chunks, writes it to flash, and restarts automatically once complete.

---

### OTA doesn't work on my ESP32C3. Why?

The most common reason is that you are using the **Huge APP** partition scheme, which does not reserve space for OTA. Switch to the **Custom SuperMini** partition as described in [references.md](references.md). After re-flashing with the correct partition, OTA will work.

---

### Does OTA work on ESP8266?

OTA via the ICRM app is supported on ESP8266, as long as you are using a partition scheme that includes OTA space (the default NodeMCU scheme supports this).

---

## 🧪 Development

### How do I test Croaster without physical sensors?

Set `dummyMode = true` in `Constants.h`:
```cpp
const bool dummyMode = true;
```
In this mode, Croaster generates simulated temperature data, so you can test the WebSocket connection, OLED display, and BLE without attaching any hardware.

---

### How do I add a custom command?

For a **basic string command**, add a new `else if` branch inside `handleBasicCommand` in `CommandHandler.cpp`:
```cpp
else if (command == "mycommand") {
    // your logic here
    responseOut = "{\"status\": \"ok\"}";
}
```
Send it as: `{"command": "mycommand"}`

For a **configuration command** (key-value style), add a new condition inside `handleJsonCommand`:
```cpp
if (json["mykey"].is<String>()) {
    String myValue = json["mykey"].as<String>();
    // your logic here
}
```
Send it as: `{"command": {"mykey": "somevalue"}}`

Both types are available over WebSocket and BLE automatically.

---

### What JSON format does Croaster broadcast?

Croaster sends a JSON payload over WebSocket and BLE at each interval. The payload includes temperature readings, RoR values, a timer, and the firmware version. The exact structure can be found in `CroasterCore.cpp` and `WebSocketManager.cpp`.

---

## 🐛 Troubleshooting

### Croaster is not showing on my WiFi list.

- Wait up to 30 seconds after powering on for the access point to appear.
- If Croaster was previously connected to a WiFi network, it will try to reconnect first. Hold the boot button during startup to force AP mode (or erase credentials).
- Check that your power supply provides sufficient current for the ESP32C3.

---

### The OLED display is blank or shows garbage.

- Verify that the SDA/SCL wires are not swapped.
- Confirm the I2C address. The SSD1306 usually uses `0x3C`. If yours uses `0x3D`, update `DisplayManager.cpp`.
- Check the 3.3V supply to the display.

---

### Artisan shows "no connection" even though I'm on the same network.

- Confirm the IP address shown on the Croaster OLED matches what you entered in Artisan.
- Make sure both devices are on the same network (same router, not guest vs. main network).
- Check that port `81` is not blocked by your firewall or router settings.
- Try restarting Croaster and reconnecting.

---

### The temperature readings are stuck at 0 or show `-0`.

- Check the wiring of the MAX6675 sensor, especially the CS pin.
- Make sure the K-type thermocouple is firmly plugged into the MAX6675 module.
- Verify your 3.3V power supply is stable.
- Try swapping the sensor modules to rule out a faulty unit.

---

_Have a question that isn't answered here? Open an issue on [GitHub](https://github.com/IiemB/Croaster)._
