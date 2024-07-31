#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include <Wire.h>
//#include "Adafruit_ADS1X15.h"
//#include <Adafruit_GFX.h>
//#include <Adafruit_SH110X.h>
//#include <Fonts/FreeSans9pt7b.h>

#define LOOPTIME 1000 // 10 second measurement period
#define METER_PIN A9

const char* ssid = "ABRACADABRA-2G";
const char* password = "OPENSESAME";

const char* mqtt_server = "10.0.0.111";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char msg[256];

JsonDocument report;

long lastMillis=0;

long counter=0;
long this_counts=0;
long last_counts=0;
long interval_counts=0;

void count_meter_pulse() {
counter++;
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("PressureMonitor")) {
      Serial.println("connected");
      // Subscribe
      mqttClient.subscribe("tankLevel");
      mqttClient.subscribe("flowRate");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
attachInterrupt(METER_PIN, count_meter_pulse, FALLING);
Serial.begin(9600);
Serial.println("Flowmeter start");
setup_wifi();
mqttClient.setServer(mqtt_server, 1883);
}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMillis > LOOPTIME) {
    lastMillis += LOOPTIME;
    
    this_counts=counter; // sample the counter
    interval_counts=this_counts - last_counts;
    last_counts=this_counts;

    Serial.print(interval_counts);
    Serial.print("    ");
    Serial.println(counter);

    // encode as JSON
    report["flowCounts"] = interval_counts;
    serializeJson(report, msg);
    // publish to flowmeter topic
    mqttClient.publish("flowmeter", msg);

  }
}
