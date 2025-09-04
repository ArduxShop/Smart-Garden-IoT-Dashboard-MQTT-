#include <WiFi.h>
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

// --- DHT11 ---
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Rain Sensor (Analog) ---
#define RAIN_PIN 34

// --- Relay ---
#define RELAY_PIN 15
bool relayState = false;

WiFiClient espClient;
PubSubClient client(espClient);

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

// --- MQTT Callback (Handle Messages) ---
void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == topic_relay_control) {
    if (msg == "1") {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
    } else {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
    }
    client.publish(topic_relay_status, relayState ? "1" : "0");
  }
}

// --- Reconnect MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int rainValue = analogRead(RAIN_PIN);

  // Kalau error baca DHT
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Publish data sensor
  String payload = "{\"suhu\":" + String(t) + ",\"kelembaban\":" + String(h) + ",\"rain\":" + String(rainValue) + "}";
  client.publish(topic_sensor, payload.c_str());

  delay(5000); // kirim tiap 5 detik
}
