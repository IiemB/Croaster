# ☕ Croaster - Monitor Sangrai Kopi Open Source

> 🇬🇧 English version available at [README.md](README.md)

**Croaster** adalah sistem pemantau suhu ringan dan open-source yang dibangun di atas mikrokontroler berbasis ESP. Dirancang untuk para pecinta dan profesional sangrai kopi, sistem ini membaca dua sensor termokopel (Suhu Biji dan Suhu Lingkungan) dan menampilkan data secara real-time di layar OLED yang ringkas. Croaster terhubung mulus ke perangkat lunak sangrai populer melalui WiFi (WebSocket) dan BLE (pada board yang mendukung), sehingga kompatibel dengan aplikasi sangrai di desktop maupun ponsel.

**Versi Firmware Saat Ini:** `0.52`

---

## 📑 Daftar Isi

- [☕ Croaster - Monitor Sangrai Kopi Open Source](#-croaster---monitor-sangrai-kopi-open-source)
  - [📑 Daftar Isi](#-daftar-isi)
  - [🚀 Fitur](#-fitur)
  - [🧩 Komponen Hardware](#-komponen-hardware)
  - [🛠 Arsitektur Software](#-arsitektur-software)
    - [Alur Data](#alur-data)
  - [📦 Library \& Dependensi](#-library--dependensi)
  - [🔧 Cara Build dan Upload](#-cara-build-dan-upload)
  - [📦 Menggunakan Croaster sebagai library di proyek Anda](#-menggunakan-croaster-sebagai-library-di-proyek-anda)
  - [🔗 Panduan Setup WiFi](#-panduan-setup-wifi)
  - [📡 Gambaran Komunikasi](#-gambaran-komunikasi)
    - [WebSocket (WiFi)](#websocket-wifi)
    - [BLE](#ble)
  - [🔌 Cara Menghubungkan Croaster dengan Artisan](#-cara-menghubungkan-croaster-dengan-artisan)
    - [🖥️ Opsi 1: Koneksi Langsung (Croaster sebagai Access Point)](#️-opsi-1-koneksi-langsung-croaster-sebagai-access-point)
    - [🌐 Opsi 2: Jaringan WiFi yang Sama (Croaster bergabung ke WiFi Anda)](#-opsi-2-jaringan-wifi-yang-sama-croaster-bergabung-ke-wifi-anda)
  - [⬆️ Update OTA (Over-The-Air)](#️-update-ota-over-the-air)
  - [🧪 Perintah Kustom](#-perintah-kustom)
    - [Perintah Bawaan](#perintah-bawaan)
    - [Menambahkan Perintah Kustom](#menambahkan-perintah-kustom)
  - [📘 Lisensi](#-lisensi)
  - [❤️ Kontribusi](#️-kontribusi)
  - [🔗 Tautan Terkait](#-tautan-terkait)

---

## 🚀 Fitur

* Mendukung **board berbasis ESP8266 dan ESP32** melalui implementasi per board
* Pemantauan real-time dua sensor termokopel **MAX6675**:
  - **BT** — Bean Temperature / Suhu Biji (di dalam drum)
  - **ET** — Environment Temperature / Suhu Lingkungan (exhaust/inlet)
* Kalkulasi **Rate of Rise (RoR)** untuk BT dan ET, diperbarui otomatis
* Pergantian satuan suhu: **Celsius** atau **Fahrenheit**
* Interval pengiriman data yang dapat dikonfigurasi (default: setiap **3 detik**)
* **Penghalusan suhu** bawaan (faktor penghalusan: 5) untuk mengurangi noise sensor
* Tampilan visual di **layar OLED 128×64** (SSD1306, I2C)
* Komunikasi WiFi via **WebSocket** di port **81**, kompatibel dengan:
  + [**Artisan Roaster Scope**](https://artisan-scope.org/) — logger sangrai standar industri
  + [**Aplikasi ICRM**](https://iiemb.github.io/#/icrm) — aplikasi pendamping mobile (Android)
* **Komunikasi BLE** (pada board yang mendukung) untuk [**aplikasi ICRM**](https://iiemb.github.io/#/icrm)
* **Update firmware OTA (Over-The-Air)** via WebSocket (WiFi) dan **BLE** (pada board yang mendukung)
* **Captive portal WiFiManager** untuk setup WiFi yang mudah — tanpa perlu flash ulang
* Penamaan perangkat unik berdasarkan chip ID (contoh: `Croaster-A1B2`)
* **Mode dummy** untuk pengembangan dan pengujian tanpa sensor fisik
* Sistem perintah JSON kustom melalui kelas `CroasterCommandHandler` yang terpusat
* Mudah diperluas dengan perintah buatan pengguna

---

## 🧩 Komponen Hardware

| Komponen | Keterangan |
|:---|:---|
| 1× Mikrokontroler berbasis ESP (WiFi dan/atau BLE) | Mikrokontroler utama — lihat `implementation/<board>/README.md` untuk board yang didukung |
| 1× [Layar OLED 128×64 (SSD1306, I2C)](images/OLED-Display.png) | Tampilan suhu real-time |
| 2× [Modul termokopel MAX6675](images/MAX6675.png) | ADC termokopel K-type berbasis SPI |
| 2× [Probe termokopel K-type](images/Type-K-thermocouple.png) | Probe suhu (BT & ET) |

> Semua komponen beroperasi pada **3.3V**. Pastikan catu daya Anda dapat menangani total konsumsi arus dari kedua sensor dan layar.

> 🔌 **Diagram pengkabelan bersifat per board** — lihat `implementation/<board>/README.md`.

---

## 🛠 Arsitektur Software

Croaster menggunakan **arsitektur C++ modular** yang bersih, dibangun dengan framework Arduino. Setiap subsistem dikemas dalam kelasnya sendiri.

Repositori disusun sebagai **library yang dapat digunakan ulang** (akar repositori: `src/` + `library.json`) ditambah **implementasi per board** di `implementation/<board>/`. Library ini **agnostik terhadap display dan konfigurasi pin** — setiap implementasi menyediakan display, tata letak pin, LED, dan konfigurasi dummy mode mereka sendiri.

### Modul library (akar repositori `src/`)

| Modul | File | Tanggung Jawab |
|:---|:---|:---|
| `CroasterCore` | `src/CroasterCore.h/.cpp` | Pembacaan sensor, kalkulasi RoR, penghalusan suhu, state data |
| `CroasterDisplay` | `src/CroasterDisplay.h` | **Antarmuka display abstrak** — diimplementasikan oleh proyek pengguna |
| `CroasterPinConfig` | `src/CroasterPinConfig.h` | Tata letak pin termokopel (diteruskan ke `CroasterCore`) |
| `CroasterCommandHandler` | `src/CroasterCommandHandler.h/.cpp` | Parsing dan dispatching perintah JSON (BLE & WebSocket) |
| `CroasterWebSocketManager` | `src/CroasterWebSocketManager.h/.cpp` | Server WebSocket, broadcast data, trigger OTA |
| `CroasterBleManager` | `src/CroasterBleManager.h/.cpp` | Server BLE, notify karakteristik, penerimaan perintah *(hanya dikompilasi bila board punya BLE)* |
| `CroasterOtaHandler` | `src/CroasterOtaHandler.h/.cpp` | Penanganan update OTA biner via WebSocket dan BLE |
| `CroasterWiFiManager` | `src/CroasterWiFiManager.h/.cpp` | Setup dan lifecycle captive portal WiFiManager |
| `CroasterDeviceIdentity` | `src/CroasterDeviceIdentity.h/.cpp` | Helper chip ID, nama perangkat, alamat IP |
| `CroasterApp` | `src/CroasterApp.h/.cpp` | **Titik masuk `begin()`/`loop()` tunggal** — agnostik display; menerima `CroasterCore&` + `CroasterDisplay*` (+ pin/level LED) |

### Modul implementasi (`implementation/`)

Komponen bersama dan proyek per board berada di bawah `implementation/`:

| Path | Kegunaan |
|:---|:---|
| `implementation/common/` | Display SSD1306 + animasi bersama (`CroasterDisplaySSD1306`, `CroasterDisplayAnimation`) untuk board berbasis OLED |
| `implementation/<board>/` | Proyek per board: `platformio.ini`, `main.cpp`, `config.h`, plus `README.md` yang mendokumentasikan board tersebut (pengkabelan, build flags, partisi) |

Setiap folder board adalah proyek PlatformIO mandiri (`main.cpp` + `config.h`) yang mengonsumsi library Croaster (`../..`) dan komponen bersama (`../common`), lalu merangkainya ke `CroasterApp` milik library yang agnostik display.

### Alur Data

```
Sensor MAX6675 → CroasterCore (baca + halus + RoR)
                       ↓
          ┌────────────┴────────────┐
  CroasterWebSocketManager  CroasterBleManager (board dengan BLE)
          ↓                         ↓
   Artisan / ICRM              ICRM (Android)
```

---

## 📦 Library & Dependensi

| Library | Kegunaan |
|:---|:---|
| [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) | Server WebSocket |
| [ArduinoJson](https://arduinojson.org/) `^7.4.3` | Parsing dan serialisasi perintah JSON |
| [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) `^2.5.16` | Driver layar OLED |
| [MAX6675_Thermocouple](https://github.com/YuriiSalimov/MAX6675_Thermocouple) `^2.0.2` | Pembacaan sensor termokopel |
| [WiFiManager](https://github.com/tzapu/WiFiManager) `^2.0.17` | Setup WiFi via captive portal |
| ESP32 BLE Arduino *(bawaan inti ESP32)* | Server BLE & karakteristik |

---

## 🔧 Cara Build dan Upload

> PlatformIO adalah satu-satunya alur kerja yang didukung (Arduino IDE tidak digunakan).

Akar repositori adalah **library yang dapat digunakan ulang**. Setiap board yang didukung memiliki folder implementasi sendiri — pilih dan build/upload dari folder tersebut:

1. Install [PlatformIO](https://platformio.org/) (ekstensi VS Code atau CLI)
2. Clone repositori dan masuk ke folder implementasi board Anda:

   ```bash
   git clone git@github.com:IiemB/Croaster.git
   cd Croaster/implementation/<board>
   ```

3. Periksa `platformio.ini` dan pilih environment target Anda
4. Upload firmware:

   ```bash
   pio run -t upload
   ```

`README.md` setiap board mendokumentasikan perintah build, pengkabelan, dan build
flags khususnya (mis. tabel partisi kustom).

---

## 📦 Menggunakan Croaster sebagai library di proyek Anda

Akar repositori adalah library PlatformIO standar. Tambahkan ke `platformio.ini` proyek lain:

```ini
[env:board_anda]
lib_deps =
    https://github.com/IiemB/Croaster.git
```

Cara termudah adalah menyalin sebuah implementasi (`implementation/<board>/`) lalu menyesuaikannya. Implementasi ini mengekspos API `begin()`/`loop()` tunggal lewat `CroasterApp`, dan semua konfigurasi khusus board (pin, dummy mode, LED, display) berada di sisi implementasi — bukan di library:

```cpp
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"                 // pins, dummyMode, ledPin/ledOnLevel (sunting di sini)
#include "CroasterDisplaySSD1306.h" // atau subclass CroasterDisplay Anda
#include "CroasterApp.h"

CroasterCore core(dummyMode, pins);
CroasterDisplaySSD1306 display(core);     // display didefinisikan di implementasi
CroasterApp app(core, &display, ledPin, ledOnLevel);

void setup() { app.begin(); }
void loop()  { app.loop(); }
```

Atau rangkai sendiri (lihat `CroasterApp.cpp` untuk rangkaian lengkap):

```cpp
#include <CroasterCore.h>
#include <CroasterPinConfig.h>
#include <CroasterCommandHandler.h>
#include <CroasterWebSocketManager.h>
#include <CroasterBleManager.h>
#include <CroasterWiFiManager.h>

CroasterPinConfig myPins = { /* sckPin, soPin, csPinBt, csPinEt */ };

CroasterCore croaster(false, myPins);  // tata letak pin Anda
MyDisplay display(croaster);           // subclass CroasterDisplay Anda
CroasterCommandHandler commands(croaster, &display);
CroasterWebSocketManager ws(croaster, commands, &display);

#if CROASTER_HAS_BLE  // terdeteksi otomatis: 1 di ESP32, 0 lainnya
CroasterBleManager ble(croaster, commands, &display);
#endif
```

- **Display** — implementasikan `CroasterDisplay` (`begin`, `loop`, `rotateScreen`,
  `blinkIndicator`, `displayToggle`, dan metode progres OTA). Teruskan `nullptr`
  bila board tidak memiliki display.
- **Pin, dummy mode & LED** — buat `CroasterPinConfig` Anda sendiri dan teruskan
  ke `CroasterCore`; pilih `dummyMode`, `ledPin`/`ledOnLevel` di `config.h`
  implementasi.
- **BLE** — library mendeteksi dukungan BLE saat kompilasi melalui
  `CROASTER_HAS_BLE` (1 di ESP32, 0 di tempat lain) dan hanya mengompilasi
  `CroasterBleManager` bila tersedia.
- **Perintah kustom** — tambahkan perintah tanpa menyentuh library:
  `app.commands().onCommand("ping", ...)` untuk perintah string dan
  `app.commands().onJsonCommand("myKey", ...)` untuk perintah JSON bersarang.

---

## 🔗 Panduan Setup WiFi

Croaster menggunakan **WiFiManager** untuk mengelola kredensial WiFi tanpa perlu flash ulang. Pada boot pertama (atau setelah menghapus kredensial), Croaster membuat access point sendiri:

1. Di ponsel atau komputer Anda, hubungkan ke jaringan WiFi bernama `[XXXX] Croaster-XXXX`
2. Captive portal akan terbuka otomatis — masukkan SSID dan password WiFi rumah Anda
3. Croaster akan menyimpan kredensial dan terhubung otomatis di boot berikutnya
4. Alamat IP yang ditetapkan ke Croaster ditampilkan di layar OLED

Untuk panduan visual, lihat: ➡️ [Cara Menghubungkan ke WiFi - YouTube](https://www.youtube.com/watch?v=esNiudoCEcU&t=434s)

---

## 📡 Gambaran Komunikasi

### WebSocket (WiFi)

- **Port:** `81`
- **Protokol:** WebSocket (frame teks untuk perintah JSON, frame biner untuk OTA)
- **Format data:** JSON, di-broadcast setiap `intervalSend` detik (default: 3 detik)
- Kompatibel dengan **Artisan Roaster Scope** dan **aplikasi ICRM** (Android)

### BLE

- **UUID Service:** `1cc9b045-a6e9-4bd5-b874-07d4f2d57843`
- **UUID Karakteristik Data:** `d56d0059-ad65-43f3-b971-431d48f89a69`
- Mendukung notify (push data) dan write (penerimaan perintah)
- Tersedia pada board yang mendukung BLE (mis. ESP32) — kompatibel dengan **aplikasi ICRM** (khusus Android)

---

## 🔌 Cara Menghubungkan Croaster dengan Artisan

Anda dapat menghubungkan Croaster ke Artisan menggunakan koneksi WiFi langsung atau melalui jaringan WiFi rumah/lokal Anda.

1. Buka Artisan → **Config → Device**
2. Pilih **Meter → WebSocket**

   ![image](images/Select-WebSocket-Device.png)

### 🖥️ Opsi 1: Koneksi Langsung (Croaster sebagai Access Point)

Gunakan metode ini ketika Croaster **tidak** terhubung ke jaringan WiFi manapun, atau ketika Anda menginginkan koneksi peer-to-peer langsung.

1. Di komputer Anda, hubungkan ke jaringan WiFi yang di-broadcast oleh Croaster (contoh: `[XXXX] Croaster-XXXX`)
2. Buka Artisan → **Config → Port**
3. Atur konfigurasi seperti yang ditunjukkan di bawah:

   ![image](images/Connect-Artisan-Directly.png)

### 🌐 Opsi 2: Jaringan WiFi yang Sama (Croaster bergabung ke WiFi Anda)

Gunakan metode ini ketika Croaster sudah terhubung ke jaringan WiFi rumah/kantor Anda.

1. Pastikan laptop dan Croaster Anda berada di **jaringan WiFi yang sama**
2. Buka Artisan → **Config → Port**
3. Masukkan **alamat IP** yang ditampilkan di layar OLED Croaster (atau via serial monitor)
4. Atur konfigurasi seperti yang ditunjukkan:

   ![image](images/Connect-Artisan-Same-Network.png)

---

## ⬆️ Update OTA (Over-The-Air)

Croaster mendukung pembaruan firmware tanpa kabel USB, melalui **aplikasi ICRM** via WebSocket (WiFi) atau BLE (pada board yang mendukung).

- OTA ditangani oleh kelas `CroasterOtaHandler`, yang menerima data firmware biner secara bertahap dan mengembalikan payload JSON progres setelah setiap potongan
- Kemajuan update ditampilkan di layar OLED selama proses berlangsung
- OTA via BLE dilengkapi pemeriksaan timeout untuk menangani transfer yang terhenti
- Beberapa board memerlukan tabel partisi dengan slot OTA — lihat `README.md` board tersebut
- Setelah update OTA berhasil, Croaster restart otomatis

---

## 🧪 Perintah Kustom

Croaster menerima perintah berformat JSON melalui WebSocket maupun BLE. Kelas `CroasterCommandHandler` mengelola semua perintah yang masuk.

### Perintah Bawaan

Semua perintah menggunakan kunci `"command"`. Perintah dasar (string):

| JSON Perintah | Aksi |
|:---|:---|
| `{"command": "restartesp"}` | Restart perangkat |
| `{"command": "erase"}` | Hapus kredensial WiFi dan restart |
| `{"command": "displayToggle"}` | Menyalakan/mematikan layar OLED |
| `{"command": "rotateScreen"}` | Memutar layar OLED 180° |
| `{"command": "dummyOn"}` | Mengaktifkan mode dummy/pengujian (tanpa sensor fisik) |
| `{"command": "dummyOff"}` | Menonaktifkan mode dummy |
| `{"command": "blink"}` | Mengedipkan LED bawaan |
| `{"command": "getDeviceInfo"}` | Mengembalikan info perangkat (IP, SSID, versi firmware) |
| `{"command": "getExtra"}` | Mengembalikan data ekstra yang ditentukan pengguna |

Perintah konfigurasi menggunakan **objek JSON bersarang** di bawah `"command"`:

| JSON Perintah | Aksi |
|:---|:---|
| `{"command": {"tempUnit": "F"}}` | Ganti satuan suhu ke Fahrenheit |
| `{"command": {"tempUnit": "C"}}` | Ganti satuan suhu ke Celsius |
| `{"command": {"interval": 5}}` | Atur interval pengiriman data ke 5 detik |
| `{"command": {"correctionBt": 1.5, "correctionEt": -0.5}}` | Terapkan offset koreksi suhu |
| `{"command": {"wifiConnect": {"ssid": "NamaWiFi", "pass": "password"}}}` | Hubungkan ke jaringan WiFi tertentu |

### Menambahkan Perintah Kustom

Perintah kustom didaftarkan di `main.cpp` implementasi Anda — **tanpa perlu
mengubah library**:

```cpp
app.commands().onCommand("ping", [](const JsonObject &json) -> String { ... });
app.commands().onJsonCommand("myKey", [](const JsonObject &json) -> String { ... });
```

`onCommand` menangani perintah string dasar (`{"command":"ping"}`); `onJsonCommand`
menangani key JSON bersarang (`{"command":{"myKey":...}}`). Setiap callback
mengembalikan string respons (kosong = tanpa respons). Keduanya tersedia melalui
WebSocket maupun BLE.

---

## 📘 Lisensi

[Lisensi MIT](LICENSE.md) — bebas digunakan untuk keperluan pribadi dan komersial. Kontribusi sangat disambut!

---

## ❤️ Kontribusi

Pull request, laporan bug, dan permintaan fitur sangat disambut! Jangan ragu untuk membuka issue atau mengirimkan PR di [GitHub](https://github.com/IiemB/Croaster).

---

## 🔗 Tautan Terkait

- [Aplikasi ICRM](https://iiemb.github.io/#/icrm) — aplikasi Android pendamping untuk Croaster
- [Artisan Roaster Scope](https://artisan-scope.org/) — logger sangrai kopi open-source
- [Video Setup WiFi](https://www.youtube.com/watch?v=esNiudoCEcU&t=434s) — panduan visual singkat
- [FAQ (Bahasa Indonesia)](FAQ_ID.md) — pertanyaan yang sering diajukan
