#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Konfigurasi DHT Sensor
#define DHTPIN 33           // Pin DHT
#define DHTTYPE DHT22      // Tipe DHT (DHT22)
DHT dht(DHTPIN, DHTTYPE);

// Konfigurasi PIN Output
#define LED_HIJAU 27        // LED Hijau di Pin 5
#define LED_KUNING 26      // LED Kuning di Pin 10
#define LED_MERAH 14       // LED Merah di Pin 12
#define BUZZER 25           // Buzzer di Pin 9
#define RELAY_POMPA 32      // Relay Pompa di Pin 7

// Konfigurasi WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Konfigurasi MQTT
const char* mqtt_server = "broker.hivemq.com"; // Atau broker lainnya
const char* topic = "sensor/data";

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi koneksi WiFi
void setupWiFi() {
  delay(10);
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung!");
}

// Fungsi koneksi MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Menghubungkan ke MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("MQTT terhubung!");
    } else {
      Serial.print("Gagal, coba lagi dalam 5 detik. Error: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Setup awal
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Inisialisasi PIN
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY_POMPA, OUTPUT);

  // Matikan semua output awal
  digitalWrite(LED_HIJAU, LOW);
  digitalWrite(LED_KUNING, LOW);
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(BUZZER, LOW);
  digitalWrite(RELAY_POMPA, LOW);

  // Koneksi ke WiFi dan MQTT
  setupWiFi();
  client.setServer(mqtt_server, 1883);
}

// Loop utama
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Membaca data dari sensor
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  // Logika kontrol LED dan Buzzer berdasarkan suhu
if (suhu > 35) {
  digitalWrite(LED_MERAH, HIGH);
  digitalWrite(BUZZER, HIGH);
  digitalWrite(LED_KUNING, LOW);
  digitalWrite(LED_HIJAU, LOW);
} else if (suhu >= 30 && suhu <= 35) {
  digitalWrite(LED_KUNING, HIGH);
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_HIJAU, LOW);
} else {
  digitalWrite(LED_HIJAU, HIGH);
  digitalWrite(LED_KUNING, LOW);
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(BUZZER, LOW);
}

  // Menyalakan relay pompa berdasarkan suhu
  if (suhu > 30) {
    digitalWrite(RELAY_POMPA, HIGH);  // Menyalakan pompa
  } else {
    digitalWrite(RELAY_POMPA, LOW);   // Mematikan pompa
  }

  // Kirim data suhu dan kelembapan ke MQTT
  String payload = "{\"suhu\":" + String(suhu) + ",\"kelembapan\":" + String(kelembapan) + "}";
  client.publish(topic, payload.c_str());

  // Delay sebelum loop berikutnya
  delay(2000);  // Delay 2 detik
}
