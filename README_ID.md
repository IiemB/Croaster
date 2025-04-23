# ☕ Croaster - Monitor Roaster Kopi Open Source

**Croaster** adalah sistem monitoring suhu berbasis mikrokontroler ESP yang bersifat open-source dan ringan. Dirancang untuk proses roasting kopi, Croaster membaca dua sensor thermocouple dan menampilkan data suhu secara real-time di layar OLED. Croaster juga menyediakan konektivitas melalui WiFi (ESP8266/ESP32) dan BLE (khusus ESP32) untuk monitoring dan kontrol jarak jauh.

📄 [View this in English](README.md)

---

## 🚀 Fitur

* Mendukung **NodeMCU ESP8266** (hanya WiFi)
* Mendukung **ESP32C3 Super Mini** (WiFi & BLE)
* Monitoring suhu real-time dari **dua sensor MAX6675** (ET dan BT)
* Tampilan visual menggunakan **OLED 128x64** (SSD1306, I2C)
* Komunikasi WiFi via **WebSocket**, kompatibel dengan:
  + **Artisan Roaster Scope**
  + **Aplikasi ICRM** *(hanya Android)*
* Komunikasi BLE (hanya ESP32) untuk **aplikasi ICRM** *(hanya Android)*
* Sistem perintah kustom melalui kelas `CommandHandler`
* Mudah dikembangkan dengan perintah-perintah khusus tambahan

---

## 🧩 Komponen Hardware

* 1× NodeMCU ESP8266 atau ESP32C3 Super Mini
* 1× OLED display 128x64 (SSD1306, I2C)
* 2× Sensor thermocouple MAX6675
* 2× Probe thermocouple K-type

---

## 🔌 Diagram Koneksi

### NodeMCU ESP8266

#### OLED Display

* GND → GND
* VCC → 3.3V
* SCL → **D1**
* SDA → **D2**

#### Sensor ET

* GND → GND
* VCC → 3.3V
* SCK → **D5**
* SO  → **D7**
* CS  → **D6**

#### Sensor BT

* GND → GND
* VCC → 3.3V
* SCK → **D5** *(berbagi)*
* SO  → **D7** *(berbagi)*
* CS  → **D8**

### ESP32C3 Super Mini

#### OLED Display

* GND → GND
* VCC → 3.3V
* SCL → **GPIO4**
* SDA → **GPIO5**

#### Sensor ET

* GND → GND
* VCC → 3.3V
* SCK → **GPIO4**
* SO  → **GPIO5**
* CS  → **GPIO6**

#### Sensor BT

* GND → GND
* VCC → 3.3V
* SCK → **GPIO4** *(berbagi)*
* SO  → **GPIO5** *(berbagi)*
* CS  → **GPIO7**

---

## 🛠 Sorotan Perangkat Lunak

* Ditulis dalam **C++** menggunakan sistem build **PlatformIO**
* Arsitektur modular: BLE, WebSocket, tampilan, dan logika sensor dipisahkan
* Kelas **CommandHandler**:
  + Menangani semua perintah JSON dari BLE/WebSocket
  + Mudah dikustomisasi untuk aksi tambahan seperti `restart`,   `erase`, dll

---

## 🔧 Cara Build dan Upload

### ✅ PlatformIO (disarankan untuk ESP8266)

1. Install [PlatformIO](https://platformio.org/)
2. Clone repository:
   

```bash
   git clone git@github.com:IiemB/Croaster.git
   cd croaster
   ```

3. Pilih board Anda di `platformio.ini` (hanya untuk ESP8266)
4. Upload firmware:
   

```bash
   pio run -t upload
   ```

### ✅ Arduino IDE (untuk ESP32C3)

1. Jalankan skrip konversi:
   

```bash
   ./copy_to_ino.sh
   ```

2. Buka folder `croaster-arduino` di **Arduino IDE**
3. Pilih board Anda:
   - ESP8266 → **NodeMCU 1.0 (ESP-12E)**
   - ESP32C3 → **Makergo ESP32C3** *(belum didukung oleh PlatformIO)*

---

## 📡 Komunikasi

* **WebSocket (WiFi):**

  + Terhubung ke **Artisan Roaster Scope**
  + Mendukung **aplikasi ICRM** *(Android saja)*

* **BLE (ESP32 saja):**

  + Khusus untuk **aplikasi ICRM** *(Android saja)*

---

## 🔗 Panduan Koneksi WiFi

Untuk menghubungkan Croaster ke jaringan WiFi Anda, Anda bisa mengikuti panduan video berikut: ➡️ [Cara Koneksi WiFi - YouTube](https://www.youtube.com/watch?v=esNiudoCEcU\&t=434s)

---

## 📘 Lisensi

Lisensi MIT — gratis untuk penggunaan pribadi dan komersial. Kontribusi sangat dipersilakan!

---

## ❤️ Kontribusi

Pull request dan saran sangat diterima.
