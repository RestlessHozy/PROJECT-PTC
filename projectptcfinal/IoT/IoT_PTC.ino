#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Informasi WiFi dan Firebase
#define SSID ""
#define PASSWORD ""
#define API_KEY ""
#define DATABASE_URL ""

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Pin konfigurasi
#define TRIG_PIN 19       // Pin Trig pada ESP32
#define ECHO_PIN 18       // Pin Echo pada ESP32
#define BUZZER_PIN 26     // Pin Buzzer pada ESP32
#define IR_SENSOR_PIN 25  // Pin Sensor IR pada ESP32

// Variabel untuk penghitungan
unsigned long startTime = 0; // Waktu mulai pengisian
unsigned long durationToFull = 0; // Durasi hingga penuh
bool isMeasuring = false;  // Status apakah sedang mengukur
int weeklyFrequency = 0;  // Frekuensi mingguan tingkat kepenuhan penuh
unsigned long weekStartTime = 0; // Waktu mulai periode mingguan
const unsigned long weekDuration = 604800000; // Durasi satu minggu dalam milidetik

void setup() {
  // Serial dan WiFi setup
  Serial.begin(115200);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Menghubungkan ke Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(">");
    delay(500);
  }
  Serial.println("\nWi-Fi Terhubung!");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  // Konfigurasi Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Terhubung ke Firebase!");
  } else {
    Serial.printf("Gagal: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);

  // Konfigurasi pin
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);

  // Inisialisasi waktu mulai minggu
  weekStartTime = millis();
}

void sendWeeklyFrequency() {
  if (Firebase.RTDB.setInt(&fbdo, "/TITIK1/WeeklyFrequency", weeklyFrequency)) {
    Serial.println("Frekuensi mingguan berhasil dikirim!");
  } else {
    Serial.printf("Gagal mengirim frekuensi mingguan: %s\n", fbdo.errorReason().c_str());
  }
}

void resetWeeklyFrequency() {
  weeklyFrequency = 0;
  Serial.println("Frekuensi mingguan direset.");
}

void playMusic() {
  tone(BUZZER_PIN, 262, 500); // Nada C
  delay(500);
  tone(BUZZER_PIN, 294, 500); // Nada D
  delay(500);
  tone(BUZZER_PIN, 330, 500); // Nada E
  delay(500);
  tone(BUZZER_PIN, 349, 500); // Nada F
  delay(500);
  noTone(BUZZER_PIN);
}

void loop() {
  long duration, distance;
  int irStatus = digitalRead(IR_SENSOR_PIN);

  // Hitung jarak ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; // Kecepatan suara: 343 m/s

  // Batasi jarak maksimum menjadi 30 cm
  if (distance > 19) {
    distance = 19;
  }

  // Hitung tingkat kepenuhan (0-100%)
  int fullness = map(distance, 19, 3, 0, 100);
  if (fullness < 0) fullness = 0;
  if (fullness > 100) fullness = 100;

  // Logika durasi pengisian
  if (fullness >= 5 && fullness < 100 && !isMeasuring) {
    // Mulai penghitungan waktu saat tingkat kepenuhan mulai meningkat
    startTime = millis();
    isMeasuring = true;
  } else if (fullness == 100 && irStatus == LOW && isMeasuring) {
    // Hitung durasi saat kedua sensor mendeteksi penuh
    durationToFull = millis() - startTime;
    isMeasuring = false;

    // Tambahkan frekuensi mingguan
    weeklyFrequency++;

    // Kirim frekuensi mingguan setiap kali dihitung
    sendWeeklyFrequency();

    // Kirim durasi ke Firebase
    if (Firebase.RTDB.setInt(&fbdo, "/TITIK1/DurationToFull", durationToFull / 1000)) {
      Serial.println("Durasi pengisian berhasil dikirim!");
    } else {
      Serial.printf("Gagal mengirim durasi: %s\n", fbdo.errorReason().c_str());
    }
  }

  // Tampilkan data ke serial monitor
  Serial.print("Ultrasonik - Jarak: ");
  Serial.print(distance);
  Serial.print(" cm - Kepenuhan: ");
  Serial.print(fullness);
  Serial.println("%");

  // Logika aktivasi buzzer: hanya aktif jika kedua sensor mendeteksi penuh
  if (fullness == 100 && irStatus == LOW) {
    Serial.println("Kedua sensor mendeteksi objek - Buzzer aktif.");
    playMusic(); // Mainkan musik
  } else {
    Serial.println("Salah satu sensor tidak mendeteksi objek - Buzzer mati.");
    noTone(BUZZER_PIN); // Matikan buzzer
  }

  // Kirim data ke Firebase
  if (Firebase.RTDB.setInt(&fbdo, "/TITIK1/Fullness", fullness)) {
    Serial.println("Data kepenuhan berhasil dikirim!");
  } else {
    Serial.printf("Gagal mengirim kepenuhan: %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.RTDB.setInt(&fbdo, "/TITIK1/IR", irStatus)) {
    Serial.println("Data IR berhasil dikirim!");
  } else {
    Serial.printf("Gagal mengirim IR: %s\n", fbdo.errorReason().c_str());
  }

  // Periksa apakah waktu minggu telah habis
  if (millis() - weekStartTime >= weekDuration) {
    resetWeeklyFrequency();
    weekStartTime = millis(); // Reset waktu minggu
  }

  delay(1000); // Tunggu 1 detik sebelum pembacaan berikutnya
}