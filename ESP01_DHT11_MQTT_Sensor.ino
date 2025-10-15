/*
  ESP-01 Temperature & Humidity Sensor Publisher with Remote Reset
  - Publishes three MQTT topics:
      MQTT/Outside/Temp
      MQTT/Outside/Humidity
      MQTT/Outside/Heartbeat
  - Listens for "MQTT/Outside/RESET" → triggers software reset

  rfesler, rfesler@gmail.com (OCT2025)
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ====== Configuration ======
#define WIFI_SSID       "WIFI_SSID"
#define WIFI_PASS       "WIFI_PASS"
#define MQTT_SERVER     "192.168.n.n"
#define MQTT_PORT       1883
#define MQTT_USER       ""
#define MQTT_PASS       ""
#define TOPIC_TEMP      "MQTT/Outside/Temp"
#define TOPIC_HUMID     "MQTT/Outside/Humidity"
#define TOPIC_HEART     "MQTT/Outside/Heartbeat"
#define TOPIC_RESET     "MQTT/Outside/RESET"
#define DHT_PIN         2
#define DHT_TYPE        DHT11
#define PUBLISH_INTERVAL 10000

// ====== Globals ======
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastPublish = 0;
uint32_t heartbeat = 0;

// ====== Callback for incoming MQTT messages ======
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Copy payload to a null-terminated buffer
  char msg[32];
  if (length >= sizeof(msg)) length = sizeof(msg) - 1;
  memcpy(msg, payload, length);
  msg[length] = '\0';

  // Check for reset command
  if (strcmp(topic, TOPIC_RESET) == 0) {
    if (strcasecmp(msg, "RESET") == 0 || strcasecmp(msg, "REBOOT") == 0) {
      mqttClient.publish(TOPIC_HEART, "Remote reset received");
      delay(1000);
      ESP.restart();
    }
  }
}

// ====== Wi-Fi Connection ======
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

// ====== MQTT Connection ======
void connectMQTT() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("Sensor:Outside", MQTT_USER, MQTT_PASS)) {
      mqttClient.subscribe(TOPIC_RESET);
      mqttClient.publish(TOPIC_HEART, "Sensor:Outside connected");
    } else {
      delay(500);
    }
  }
}

// ====== Setup ======
void setup() {
  connectWiFi();
  connectMQTT();
  dht.begin();
}

// ====== Main Loop ======
void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;
    heartbeat++;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(false);

    if (isnan(humidity) || isnan(temperature)) return;

    char msg[16];

    // Temperature
    dtostrf(temperature, 0, 1, msg);
    mqttClient.publish(TOPIC_TEMP, msg);

    // Humidity
    dtostrf(humidity, 0, 1, msg);
    mqttClient.publish(TOPIC_HUMID, msg);

    // Heartbeat
    snprintf(msg, sizeof(msg), "%lu", heartbeat);
    mqttClient.publish(TOPIC_HEART, msg);
  }
}
