#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// --- WiFi & MQTT ---
const char* ssid = "HUAWEI-2.4G-e95T";
const char* password = "8apcSP2m";
const char* mqtt_server = "broker.hivemq.com";  // Public MQTT broker
const int mqtt_port = 1883;

// --- Topics ---
const char* topic_sensor = "smartgarden/sensor";
const char* topic_relay_control = "smartgarden/relay/control";
const char* topic_relay_status = "smartgarden/relay/status";

// --- DHT22 ---
#define DHTPIN D4       
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// --- Soil Moisture Sensor (Analog) ---
#define SOIL_PIN A0     

// --- Relay ---
#define RELAY_PIN D5    
bool relayState = false;

// --- Batas kelembaban tanah ---
const int soilThreshold = 40; // % kelembaban minimal

WiFiClient espClient;
PubSubClient client(espClient);

// --- Mode Relay ---
enum RelayMode { MANUAL, AUTO };
RelayMode modeRelay = AUTO;

unsigned long lastControl = 0;

// --- WiFi Connect ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- MQTT Callback ---
void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)message[i];

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == topic_relay_control) {
    if (msg == "AUTO") {
      modeRelay = AUTO;
      Serial.println("Mode relay: AUTO");
    } else {
      modeRelay = MANUAL;
      relayState = (msg == "1");
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
      client.publish(topic_relay_status, String(relayState ? "1" : "0").c_str(), true);
      Serial.println("Mode relay: MANUAL");
    }
  }
}

// --- Reconnect MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "WemosClient-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_relay_control);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// --- Loop ---
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // --- Baca sensor ---
  float h = dht.readHumidity();       // 0–100%
  float t = dht.readTemperature();    // °C
  int soilRaw = analogRead(SOIL_PIN); // 0–1023

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int soilPercent = map(soilRaw, 1023, 300, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  Serial.print("Suhu: "); Serial.print(t); Serial.println(" °C");
  Serial.print("Kelembaban Udara: "); Serial.print(h); Serial.println(" %");
  Serial.print("Kelembaban Tanah: "); Serial.print(soilPercent); Serial.println(" %");

  // --- Auto Relay berdasarkan soilThreshold (hanya di AUTO mode) ---
  if (millis() - lastControl > 1000) { // cek tiap 1 detik
    if (modeRelay == AUTO) {
      if (soilPercent < soilThreshold) relayState = true;
      else relayState = false;

      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
      // Publish status relay + mode ke MQTT
      String statusMsg = String(relayState ? "1" : "0") + (modeRelay == AUTO ? "-AUTO" : "-MANUAL");
      client.publish(topic_relay_status, statusMsg.c_str(), true);
    }
    lastControl = millis();
  }

  // --- Publish sensor data ---
  String payload = "{\"suhu\":" + String(t) + 
                   ",\"kelembaban_udara\":" + String(h) + 
                   ",\"kelembaban_tanah\":" + String(soilPercent) + "}";
  client.publish(topic_sensor, payload.c_str(), true);

  delay(5000);
}
