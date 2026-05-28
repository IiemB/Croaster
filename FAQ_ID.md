# ❓ Croaster FAQ — Pertanyaan yang Sering Diajukan

> 🇬🇧 English version available at [FAQ.md](FAQ.md)

---

## 📦 Umum

### Apa itu Croaster?

Croaster adalah firmware open-source untuk mikrokontroler berbasis ESP8266 dan ESP32C3 yang mengubah hardware Anda menjadi monitor suhu sangrai kopi. Firmware ini membaca dua sensor termokopel (Suhu Biji dan Suhu Lingkungan), menghitung Rate of Rise (RoR), dan mengirimkan data melalui WiFi (WebSocket) atau BLE ke aplikasi yang kompatibel seperti Artisan Roaster Scope dan ICRM.

---

### Mikrokontroler apa saja yang didukung?

| Board | WiFi | BLE | OTA |
|:---|:---:|:---:|:---:|
| NodeMCU ESP8266 | ✅ | ❌ | ✅ |
| ESP32C3 Super Mini | ✅ | ✅ | ✅ |

---

### Aplikasi apa yang bisa digunakan bersama Croaster?

- **[Artisan Roaster Scope](https://artisan-scope.org/)** — terhubung via WebSocket (WiFi). Tersedia untuk Windows, macOS, dan Linux.
- **[Aplikasi ICRM](https://iiemb.github.io/#/icrm)** — terhubung via WebSocket (WiFi) atau BLE (khusus ESP32). Tersedia untuk Android.

---

### Apakah Croaster gratis digunakan?

Ya. Croaster dirilis di bawah **Lisensi MIT**, yang berarti gratis untuk penggunaan pribadi maupun komersial. Anda bebas memodifikasi, mendistribusikan, dan mengembangkannya.

---

## 🔌 Hardware

### Sensor apa yang digunakan Croaster?

Croaster menggunakan dua modul ADC termokopel **MAX6675** dengan **probe termokopel K-type**:
- **BT (Bean Temperature / Suhu Biji)** — ditempatkan di dalam drum sangrai untuk mengukur suhu biji kopi
- **ET (Environment Temperature / Suhu Lingkungan)** — ditempatkan di exhaust atau inlet untuk mengukur suhu aliran udara

---

### Bisakah saya menggunakan hanya satu sensor?

Firmware dirancang untuk membaca dua sensor. Jika hanya satu sensor yang terpasang, sensor yang tidak terpasang kemungkinan akan melaporkan `0` atau nilai yang tidak stabil. Anda dapat memodifikasi `CroasterCore.cpp` untuk setup satu sensor, namun ini tidak didukung secara resmi.

---

### Berapa tegangan operasi komponen-komponen tersebut?

Semua komponen (layar OLED, modul MAX6675, dan mikrokontroler) beroperasi pada **3.3V**. Jangan hubungkan komponen ini langsung ke 5V, karena dapat merusak komponen.

---

### Apakah kedua sensor menggunakan pin SPI yang sama?

Ya. Kedua sensor MAX6675 berbagi jalur **SCK** dan **SO** pada bus SPI. Keduanya dibedakan oleh pin **CS (Chip Select)** masing-masing:
- **CS sensor BT:** `D8` (ESP8266) / `GPIO7` (ESP32C3)
- **CS sensor ET:** `D6` (ESP8266) / `GPIO6` (ESP32C3)

---

## 📶 WiFi & Konektivitas

### Bagaimana cara menghubungkan Croaster ke jaringan WiFi saya?

Pada boot pertama, Croaster membuat access point WiFi sendiri dengan nama `[XXXX] Croaster-XXXX`. Hubungkan dari ponsel atau komputer Anda, dan halaman captive portal akan terbuka secara otomatis. Masukkan kredensial WiFi rumah Anda di sana — Croaster akan menyimpannya dan terhubung otomatis di boot berikutnya.

Lihat [Video Setup WiFi](https://www.youtube.com/watch?v=esNiudoCEcU&t=434s) untuk panduan visual.

---

### Saya lupa password WiFi atau ingin mengganti jaringan. Bagaimana cara mereset?

Kirim perintah `{"command": "erase"}` via WebSocket atau BLE (aplikasi ICRM), atau tekan tombol reset sambil menahan tombol boot pada ESP32C3. Ini akan menghapus kredensial WiFi yang tersimpan dan menempatkan Croaster kembali ke mode access point untuk konfigurasi ulang.

---

### Bisakah saya menggunakan Croaster tanpa jaringan WiFi?

Ya. Ketika Croaster tidak terhubung ke jaringan WiFi manapun, ia menyiarkan access point-nya sendiri. Anda dapat menghubungkan Artisan langsung ke access point tersebut. Lihat [Opsi 1 di README](README_ID.md#-opsi-1-koneksi-langsung-croaster-sebagai-access-point).

---

### Berapa port WebSocket yang digunakan?

Server WebSocket Croaster berjalan di **port 81**.

---

### Seberapa sering Croaster mengirim data?

Secara default, data di-broadcast setiap **3 detik**. Anda dapat mengubahnya dengan mengirim:
```json
{"command": {"interval": 5}}
```
Ganti `5` dengan interval yang Anda inginkan dalam satuan detik.

---

### Apakah BLE tersedia di ESP8266?

Tidak. BLE hanya tersedia di platform **ESP32**. ESP8266 hanya mendukung WiFi.

---

## 🌡️ Suhu & Sensor

### Satuan suhu apa saja yang didukung?

Croaster mendukung **Celsius (C)** dan **Fahrenheit (F)**. Ganti satuan dengan mengirim:
```json
{"command": {"tempUnit": "F"}}
```
atau
```json
{"command": {"tempUnit": "C"}}
```

---

### Apa itu Rate of Rise (RoR)?

Rate of Rise (RoR) mengukur seberapa cepat suhu meningkat, dinyatakan dalam **derajat per menit**. Croaster menghitung RoR untuk BT dan ET secara otomatis berdasarkan riwayat sensor terbaru. RoR adalah metrik kritis dalam sangrai kopi untuk memantau fase perkembangan.

---

### Pembacaan sensor saya terlihat tidak stabil. Apa yang harus saya lakukan?

Croaster menerapkan **filter penghalusan** (faktor: 5) untuk mengurangi noise dari sensor MAX6675. Jika pembacaan masih tidak stabil, periksa kabel Anda, pastikan probe termokopel terpasang dengan kuat di modul MAX6675, dan pastikan suplai VCC stabil di 3.3V.

---

### Bisakah saya menerapkan koreksi/offset suhu?

Ya. Kirim perintah koreksi:
```json
{"command": {"correctionBt": 1.5, "correctionEt": -0.5}}
```
Ini menerapkan offset `+1.5°` pada BT dan offset `-0.5°` pada ET. Koreksi diterapkan di atas pembacaan sensor yang sudah dihaluskan.

---

## 🔧 Build & Upload

### Metode build mana yang sebaiknya saya gunakan?

| Skenario | Metode yang Direkomendasikan |
|:---|:---|
| ESP8266 | PlatformIO atau Arduino IDE |
| ESP32C3 (dengan dukungan OTA) | Arduino IDE dengan partisi Custom SuperMini |
| ESP32C3 (tanpa OTA, ukuran sketch maksimal) | Arduino IDE dengan partisi Huge APP |

---

### Mengapa saya tidak bisa menggunakan PlatformIO untuk ESP32C3 Super Mini?

Definisi board **Makergo ESP32C3 SuperMini** tidak tersedia secara resmi di registry board PlatformIO. Anda dapat menambahkannya secara manual menggunakan konfigurasi komunitas dari [repositori ini](https://github.com/sigmdel/supermini_esp32c3_sketches.git), namun metode yang didukung secara resmi adalah menggunakan **Arduino IDE** dengan paket board Makergo.

---

### Apa itu `copy_to_ino.sh`?

Ini adalah shell script yang menyalin file sumber dari direktori `src/` (struktur PlatformIO) ke folder `croaster-arduino/` dengan konvensi penamaan sketch Arduino yang benar. Jalankan sebelum membuka project di Arduino IDE.

---

### Apa itu file `custom32c3sm.csv`?

Ini adalah **tabel partisi kustom** untuk ESP32C3 Super Mini. Tata letak partisi ini mengalokasikan lebih banyak penyimpanan untuk aplikasi (`1900544` byte) sekaligus menyisihkan ruang untuk update OTA. Tanpanya, update OTA tidak dapat berjalan berdampingan dengan binary firmware yang besar. Lihat [references.md](references.md) untuk langkah instalasi.

---

## ⬆️ Update OTA

### Bagaimana cara memperbarui firmware secara over the air?

Gunakan **aplikasi ICRM** di Android. Aplikasi akan mengirimkan binary firmware yang telah dikompilasi melalui WebSocket. Croaster menerimanya dalam potongan-potongan, menuliskannya ke flash, dan restart otomatis setelah selesai.

---

### OTA tidak bekerja di ESP32C3 saya. Mengapa?

Alasan paling umum adalah Anda menggunakan skema partisi **Huge APP**, yang tidak menyisihkan ruang untuk OTA. Ganti ke partisi **Custom SuperMini** seperti yang dijelaskan di [references.md](references.md). Setelah flash ulang dengan partisi yang benar, OTA akan berfungsi.

---

### Apakah OTA bekerja di ESP8266?

OTA via aplikasi ICRM didukung di ESP8266, selama Anda menggunakan skema partisi yang mencakup ruang OTA (skema NodeMCU default mendukung ini).

---

## 🧪 Pengembangan

### Bagaimana cara menguji Croaster tanpa sensor fisik?

Atur `dummyMode = true` di `Constants.h`:
```cpp
const bool dummyMode = true;
```
Dalam mode ini, Croaster menghasilkan data suhu simulasi, sehingga Anda dapat menguji koneksi WebSocket, tampilan OLED, dan BLE tanpa memasang hardware apapun.

---

### Bagaimana cara menambahkan perintah kustom?

Untuk **perintah dasar (string)**, tambahkan cabang `else if` baru di dalam `handleBasicCommand` di `CommandHandler.cpp`:
```cpp
else if (command == "perintahsaya") {
    // logika Anda di sini
    responseOut = "{\"status\": \"ok\"}";
}
```
Kirim sebagai: `{"command": "perintahsaya"}`

Untuk **perintah konfigurasi** (format key-value), tambahkan kondisi baru di dalam `handleJsonCommand`:
```cpp
if (json["kuncisaya"].is<String>()) {
    String nilai = json["kuncisaya"].as<String>();
    // logika Anda di sini
}
```
Kirim sebagai: `{"command": {"kuncisaya": "nilaitertentu"}}`

Kedua tipe tersedia melalui WebSocket maupun BLE secara otomatis.

---

### Format JSON apa yang di-broadcast oleh Croaster?

Croaster mengirimkan payload JSON melalui WebSocket dan BLE di setiap interval. Payload tersebut mencakup pembacaan suhu, nilai RoR, timer, dan versi firmware. Struktur lengkapnya dapat ditemukan di `CroasterCore.cpp` dan `WebSocketManager.cpp`.

---

## 🐛 Pemecahan Masalah

### Croaster tidak muncul di daftar WiFi saya.

- Tunggu hingga 30 detik setelah dinyalakan agar access point muncul.
- Jika Croaster sebelumnya terhubung ke jaringan WiFi, ia akan mencoba terhubung kembali terlebih dahulu. Tahan tombol boot saat startup untuk memaksa mode AP (atau hapus kredensial).
- Periksa apakah catu daya Anda menyediakan arus yang cukup untuk ESP32C3.

---

### Layar OLED kosong atau menampilkan karakter tidak jelas.

- Verifikasi bahwa kabel SDA/SCL tidak tertukar.
- Konfirmasi alamat I2C. SSD1306 biasanya menggunakan `0x3C`. Jika milik Anda menggunakan `0x3D`, perbarui `DisplayManager.cpp`.
- Periksa suplai 3.3V ke layar.

---

### Artisan menampilkan "no connection" meskipun saya berada di jaringan yang sama.

- Konfirmasi alamat IP yang ditampilkan di OLED Croaster sesuai dengan yang Anda masukkan di Artisan.
- Pastikan kedua perangkat berada di jaringan yang sama (router yang sama, bukan jaringan guest vs. utama).
- Periksa apakah port `81` tidak diblokir oleh firewall atau pengaturan router Anda.
- Coba restart Croaster dan hubungkan kembali.

---

### Pembacaan suhu macet di 0 atau menampilkan `-0`.

- Periksa kabel sensor MAX6675, terutama pin CS.
- Pastikan probe termokopel K-type terpasang kuat di modul MAX6675.
- Verifikasi suplai daya 3.3V Anda stabil.
- Coba tukar modul sensor untuk mengetahui apakah ada unit yang rusak.

---

_Ada pertanyaan yang belum terjawab di sini? Buka issue di [GitHub](https://github.com/IiemB/Croaster)._
