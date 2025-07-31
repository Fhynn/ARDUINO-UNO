#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 3

// Button pins
#define BTN_AQUA 2     // Button untuk Aqua
#define BTN_KOPI 4     // Button untuk Kopi  
#define BTN_OREO 5     // Button untuk Oreo
#define BTN_TEH 6      // Button untuk Teh
#define BTN_MINT 7     // Button untuk Mint
#define BTN_TOPUP 8    // Button untuk Top Up
#define BTN_CEK 9      // Button untuk Cek Saldo

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

// Variables
bool buttonPressed = false;
int selectedItem = 0;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// Product data
String namaBarang[] = {"Aqua", "Kopi", "Oreo", "Teh", "Mint"};
long hargaBarang[] = {3000, 2000, 5000, 4000, 1500};

byte buffer[18];
byte blockNum = 5;
MFRC522::MIFARE_Key key;

// Utility functions
long bytesToLong(byte* bytes) {
  return ((long)bytes[0] << 24) | ((long)bytes[1] << 16) | ((long)bytes[2] << 8) | ((long)bytes[3]);
}

void longToBytes(long val, byte* bytes) {
  bytes[0] = (byte)(val >> 24);
  bytes[1] = (byte)(val >> 16);
  bytes[2] = (byte)(val >> 8);
  bytes[3] = (byte)val;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  lcd.init();
  lcd.backlight();
  mfrc522.PCD_Init();
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  
  // Setup buttons
  pinMode(BTN_AQUA, INPUT_PULLUP);
  pinMode(BTN_KOPI, INPUT_PULLUP);
  pinMode(BTN_OREO, INPUT_PULLUP);
  pinMode(BTN_TEH, INPUT_PULLUP);
  pinMode(BTN_MINT, INPUT_PULLUP);
  pinMode(BTN_TOPUP, INPUT_PULLUP);
  pinMode(BTN_CEK, INPUT_PULLUP);
  
  // Initialize key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  // Test servo
  myServo.write(90);
  delay(500);
  myServo.write(0);
  delay(500);
  
  showWelcome();
  Serial.println(F("RFID Vending Machine Ready!"));
}

void showWelcome() {
  lcd.clear();
  lcd.print(F("RFID VENDING"));
  lcd.setCursor(0,1);
  lcd.print(F("MACHINE READY"));
  delay(2000);
  showMainMenu();
}

void showMainMenu() {
  lcd.clear();
  lcd.print(F("Pilih Produk:"));
  lcd.setCursor(0,1);
  lcd.print(F("Tekan Tombol"));
}

void showProductMenu() {
  static byte menuIndex = 0;
  
  lcd.clear();
  
  // Tampilkan 2 produk bergantian
  if (menuIndex == 0) {
    lcd.print(F("1:Aqua-3k 2:Kopi-2k"));
    lcd.setCursor(0,1);
    lcd.print(F("3:Oreo-5k"));
  } else if (menuIndex == 1) {
    lcd.print(F("4:Teh-4k 5:Mint-1.5k"));
    lcd.setCursor(0,1);
    lcd.print(F("TopUp | CekSaldo"));
  }
  
  menuIndex = (menuIndex + 1) % 2;
}

bool scanCard() {
  lcd.clear();
  lcd.print(F("Tempelkan Kartu"));
  lcd.setCursor(0,1);
  lcd.print(F("Tunggu..."));
  Serial.println(F("Menunggu kartu..."));
  
  unsigned long start = millis();
  while (millis() - start < 15000) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      Serial.println(F("Kartu terdeteksi"));
      lcd.clear();
      lcd.print(F("Kartu OK"));
      delay(500);
      return true;
    }
    delay(50);
  }
  
  Serial.println(F("Timeout - kartu tidak terdeteksi"));
  lcd.clear();
  lcd.print(F("Timeout"));
  lcd.setCursor(0,1);
  lcd.print(F("Coba lagi"));
  delay(2000);
  return false;
}

bool authCard() {
  byte trailer = (blockNum / 4 * 4) + 3;
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailer, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
    Serial.println(F("Autentikasi gagal"));
    lcd.clear();
    lcd.print(F("Auth Gagal"));
    lcd.setCursor(0,1);
    lcd.print(F("Kartu Invalid"));
    delay(2000);
    return false;
  }
  return true;
}

long readSaldo() {
  byte size = 18;
  if (mfrc522.MIFARE_Read(blockNum, buffer, &size) != MFRC522::STATUS_OK) {
    Serial.println(F("Gagal membaca saldo"));
    lcd.clear();
    lcd.print(F("Baca Gagal"));
    delay(2000);
    return -1;
  }
  return bytesToLong(buffer);
}

bool writeSaldo(long saldo) {
  longToBytes(saldo, buffer);
  for(int i = 4; i < 16; i++) buffer[i] = 0x00;
  
  if (mfrc522.MIFARE_Write(blockNum, buffer, 16) != MFRC522::STATUS_OK) {
    Serial.println(F("Gagal menulis saldo"));
    lcd.clear();
    lcd.print(F("Tulis Gagal"));
    delay(2000);
    return false;
  }
  return true;
}

void openServo() {
  Serial.println(F("Mengeluarkan barang"));
  lcd.clear();
  lcd.print(F("Mengambil"));
  lcd.setCursor(0,1);
  lcd.print(F("Barang..."));
  
  myServo.write(90);
  delay(3000);
  myServo.write(0);
  
  lcd.clear();
  lcd.print(F("Ambil Barang"));
  lcd.setCursor(0,1);
  lcd.print(F("Anda"));
  delay(2000);
  
  Serial.println(F("Barang berhasil dikeluarkan"));
}

void beliBarang(int itemIndex) {
  if (itemIndex < 0 || itemIndex >= 5) return;
  
  Serial.print(F("Membeli: "));
  Serial.print(namaBarang[itemIndex]);
  Serial.print(F(" - Rp."));
  Serial.println(hargaBarang[itemIndex]);
  
  // Tampilkan info produk
  lcd.clear();
  lcd.print(namaBarang[itemIndex]);
  lcd.setCursor(0,1);
  lcd.print(F("Rp."));
  lcd.print(hargaBarang[itemIndex]);
  delay(2000);
  
  // Scan kartu
  if (!scanCard()) {
    resetSystem();
    return;
  }
  
  if (!authCard()) {
    resetSystem();
    return;
  }
  
  // Baca saldo
  long saldoLama = readSaldo();
  if (saldoLama == -1) {
    resetSystem();
    return;
  }
  
  Serial.print(F("Saldo saat ini: Rp."));
  Serial.println(saldoLama);
  
  // Cek saldo cukup
  if (saldoLama < hargaBarang[itemIndex]) {
    Serial.println(F("Saldo tidak mencukupi"));
    lcd.clear();
    lcd.print(F("Saldo Kurang"));
    lcd.setCursor(0,1);
    lcd.print(F("Rp."));
    lcd.print(saldoLama);
    delay(3000);
    
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    resetSystem();
    return;
  }
  
  // Potong saldo
  long saldoBaru = saldoLama - hargaBarang[itemIndex];
  
  if (!writeSaldo(saldoBaru)) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    resetSystem();
    return;
  }
  
  // Berhasil
  Serial.println(F("TRANSAKSI BERHASIL!"));
  Serial.print(F("Saldo baru: Rp."));
  Serial.println(saldoBaru);
  
  lcd.clear();
  lcd.print(F("BERHASIL!"));
  lcd.setCursor(0,1);
  lcd.print(F("Saldo: "));
  lcd.print(saldoBaru);
  delay(2000);
  
  // Keluarkan barang
  openServo();
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  resetSystem();
}

void topUp() {
  Serial.println(F("Proses Top Up Rp.10000"));
  
  lcd.clear();
  lcd.print(F("TOP UP"));
  lcd.setCursor(0,1);
  lcd.print(F("Rp.10,000"));
  delay(2000);
  
  if (!scanCard()) {
    resetSystem();
    return;
  }
  
  if (!authCard()) {
    resetSystem();
    return;
  }
  
  long saldoLama = readSaldo();
  if (saldoLama == -1) {
    resetSystem();
    return;
  }
  
  long saldoBaru = saldoLama + 10000;
  
  if (!writeSaldo(saldoBaru)) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    resetSystem();
    return;
  }
  
  Serial.println(F("Top Up berhasil"));
  Serial.print(F("Saldo baru: Rp."));
  Serial.println(saldoBaru);
  
  lcd.clear();
  lcd.print(F("TOP UP BERHASIL"));
  lcd.setCursor(0,1);
  lcd.print(F("Rp."));
  lcd.print(saldoBaru);
  delay(3000);
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  resetSystem();
}

void cekSaldo() {
  Serial.println(F("Mengecek saldo"));
  
  lcd.clear();
  lcd.print(F("CEK SALDO"));
  delay(1000);
  
  if (!scanCard()) {
    resetSystem();
    return;
  }
  
  if (!authCard()) {
    resetSystem();
    return;
  }
  
  long saldo = readSaldo();
  if (saldo == -1) {
    resetSystem();
    return;
  }
  
  Serial.print(F("Saldo: Rp."));
  Serial.println(saldo);
  
  lcd.clear();
  lcd.print(F("Saldo Anda:"));
  lcd.setCursor(0,1);
  lcd.print(F("Rp."));
  lcd.print(saldo);
  delay(5000);
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  resetSystem();
}

void resetSystem() {
  buttonPressed = false;
  selectedItem = 0;
  delay(1000);
  showMainMenu();
}

void checkButtons() {
  unsigned long currentTime = millis();
  
  // Debounce check
  if (currentTime - lastButtonPress < debounceDelay) {
    return;
  }
  
  if (digitalRead(BTN_AQUA) == LOW) {
    lastButtonPress = currentTime;
    beliBarang(0); // Aqua
  }
  else if (digitalRead(BTN_KOPI) == LOW) {
    lastButtonPress = currentTime;
    beliBarang(1); // Kopi
  }
  else if (digitalRead(BTN_OREO) == LOW) {
    lastButtonPress = currentTime;
    beliBarang(2); // Oreo
  }
  else if (digitalRead(BTN_TEH) == LOW) {
    lastButtonPress = currentTime;
    beliBarang(3); // Teh
  }
  else if (digitalRead(BTN_MINT) == LOW) {
    lastButtonPress = currentTime;
    beliBarang(4); // Mint
  }
  else if (digitalRead(BTN_TOPUP) == LOW) {
    lastButtonPress = currentTime;
    topUp();
  }
  else if (digitalRead(BTN_CEK) == LOW) {
    lastButtonPress = currentTime;
    cekSaldo();
  }
}

void loop() {
  static unsigned long lastMenuUpdate = 0;
  
  // Update menu display setiap 3 detik
  if (millis() - lastMenuUpdate > 3000) {
    if (!buttonPressed) {
      showProductMenu();
    }
    lastMenuUpdate = millis();
  }
  
  // Check button presses
  checkButtons();
  
  // Handle serial commands (untuk debugging/testing)
  if (Serial.available()) {
    String cmd = Serial.readString();
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "test") {
      Serial.println(F("Test servo"));
      myServo.write(90);
      delay(2000);
      myServo.write(0);
    }
    else if (cmd == "menu") {
      showMainMenu();
    }
  }
  
  delay(50);
}