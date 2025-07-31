# RFID Vending Machine

## 📋 Deskripsi Project

Vending machine otomatis menggunakan RFID card sebagai sistem pembayaran elektronik. Dilengkapi dengan LCD display, servo motor dispenser, dan sistem button untuk memilih produk. Saldo tersimpan langsung di RFID card menggunakan teknologi MIFARE Classic.

## 🛠️ Komponen Hardware

### Mikrokontroler
- **Arduino Uno/Nano** - 1 unit

### Sensor & Aktuator
- **MFRC522 RFID Reader** - 1 unit
- **LCD I2C 16x2** - 1 unit
- **Servo Motor SG90** - 1 unit
- **Push Button** - 7 unit
- **RFID Card MIFARE Classic** - minimal 1 unit

### Komponen Tambahan
- **Resistor 10kΩ** - 7 unit (pull-up resistor untuk button)
- **Jumper Wire** - secukupnya
- **Breadboard** - 1 unit
- **Power Supply 5V** - 1 unit

## 🔌 Wiring Diagram

### RFID MFRC522
```
MFRC522    →  Arduino
VCC        →  3.3V
RST        →  Pin 9
GND        →  GND
MISO       →  Pin 12
MOSI       →  Pin 11
SCK        →  Pin 13
SDA(SS)    →  Pin 10
```

### LCD I2C
```
LCD I2C    →  Arduino
VCC        →  5V
GND        →  GND
SDA        →  Pin A4
SCL        →  Pin A5
```

### Servo Motor
```
Servo      →  Arduino
VCC        →  5V
GND        →  GND
Signal     →  Pin 3
```

### Push Buttons
```
Button     →  Arduino Pin
Aqua       →  Pin 2
Kopi       →  Pin 4
Oreo       →  Pin 5
Teh        →  Pin 6
Mint       →  Pin 7
Top Up     →  Pin 8
Cek Saldo  →  Pin 9
```

*Semua button menggunakan INPUT_PULLUP, sehingga tidak perlu resistor eksternal.*

## 📚 Library Dependencies

Install library berikut melalui Arduino IDE Library Manager:

```cpp
#include <Servo.h>              // Built-in Arduino
#include <SPI.h>                // Built-in Arduino
#include <MFRC522.h>            // by GithubCommunity
#include <Wire.h>               // Built-in Arduino
#include <LiquidCrystal_I2C.h>  // by Frank de Brabander
```

## 💰 Menu Produk & Harga

| No | Produk | Harga    | Button Pin |
|----|--------|----------|------------|
| 1  | Aqua   | Rp 3,000 | Pin 2      |
| 2  | Kopi   | Rp 2,000 | Pin 4      |
| 3  | Oreo   | Rp 5,000 | Pin 5      |
| 4  | Teh    | Rp 4,000 | Pin 6      |
| 5  | Mint   | Rp 1,500 | Pin 7      |

### Fungsi Tambahan
- **Top Up**: +Rp 10,000 (Pin 8)
- **Cek Saldo**: Lihat saldo kartu (Pin 9)

## 🚀 Cara Penggunaan

### 1. Setup Awal
1. Upload kode ke Arduino
2. Pastikan semua koneksi hardware benar
3. Buka Serial Monitor (115200 baud)

### 2. Inisialisasi Kartu RFID
```
Ketik di Serial Monitor: init
Tempelkan kartu RFID
Kartu akan terisi saldo awal Rp 50,000
```

### 3. Cara Membeli Produk
1. **Tekan button** produk yang diinginkan
2. **LCD menampilkan** nama produk dan harga
3. **Tempelkan kartu RFID** saat diminta
4. **Sistem memverifikasi** saldo
5. **Jika saldo cukup**: 
   - Saldo dipotong otomatis
   - Servo mengeluarkan produk
   - LCD menampilkan saldo baru
6. **Jika saldo kurang**: LCD menampilkan pesan error

### 4. Top Up Saldo
1. **Tekan button Top Up** (pin 8)
2. **Tempelkan kartu RFID**
3. **Saldo bertambah** Rp 10,000 otomatis

### 5. Cek Saldo
1. **Tekan button Cek Saldo** (pin 9)
2. **Tempelkan kartu RFID**
3. **LCD menampilkan** saldo saat ini

## 💾 Sistem Penyimpanan Saldo

- **Teknologi**: MIFARE Classic 1K
- **Block Storage**: Block 5 (sektor 1)
- **Format Data**: 4 bytes long integer
- **Autentikasi**: Key A (default: 0xFFFFFFFFFFFF)
- **Kapasitas**: Saldo maksimal ±2 miliar rupiah

## 🔧 Troubleshooting

### LCD Tidak Menyala
1. Cek alamat I2C LCD (0x27, 0x3F, 0x26)
2. Pastikan koneksi SDA/SCL benar
3. Ganti kabel jumper

### RFID Tidak Terdeteksi
1. Pastikan jarak kartu <3cm dari reader
2. Cek koneksi SPI (MISO, MOSI, SCK, SS)
3. Pastikan power supply stabil

### Servo Tidak Bergerak
1. Cek koneksi signal wire ke pin 3
2. Pastikan power supply 5V cukup
3. Test dengan serial command: `test`

### Button Tidak Responsif
1. Pastikan menggunakan INPUT_PULLUP
2. Cek koneksi button ke ground
3. Periksa debounce delay

## 📖 Penjelasan Kode

### Fungsi Utama

#### `setup()`
- Inisialisasi semua komponen
- Setup pin button dengan pull-up
- Test servo motor
- Tampilkan welcome message

#### `loop()`
- Update display menu setiap 3 detik
- Monitoring semua button press
- Handle serial commands untuk debugging

#### `beliBarang(int itemIndex)`
- Proses pembelian produk
- Verifikasi saldo di kartu RFID
- Potong saldo dan update kartu
- Aktivasi servo dispenser

#### `scanCard()`
- Menunggu kartu RFID (timeout 15 detik)
- Return true jika kartu terdeteksi

#### `readSaldo()` & `writeSaldo()`
- Baca/tulis data saldo ke RFID card
- Menggunakan MIFARE Classic protocol

## 🔒 Security Features

- **RFID Authentication** menggunakan Key A
- **Timeout protection** untuk scan kartu
- **Debounce protection** untuk button
- **Error handling** untuk transaksi gagal
- **Auto reset** sistem setelah transaksi

## 📝 Serial Commands (Debug)

| Command | Fungsi |
|---------|--------|
| `test`  | Test servo motor |
| `menu`  | Tampilkan main menu |

## 🎯 Future Improvements

- [ ] Multiple servo untuk banyak slot
- [ ] Network connectivity (WiFi/Ethernet)
- [ ] Database logging transaksi
- [ ] Admin panel untuk management
- [ ] Receipt printer integration
- [ ] Multiple payment methods
- [ ] Inventory tracking system

## 👨‍💻 Author

**Project**: RFID Vending Machine  
**Platform**: Arduino C++  
**Version**: 1.0  
**License**: MIT  

## 📄 File Structure

```
rfid_vending_machine/
├── rfid_vending_machine.ino    # Main Arduino sketch
├── README.md                   # Project documentation
└── wiring_diagram.png          # Hardware connection diagram
```

---

*Dibuat dengan ❤️ menggunakan Arduino C++*
