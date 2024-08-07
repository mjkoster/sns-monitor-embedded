#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_ADS1X15.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
#include <ArduinoJson.h>

#define oled_Address 0x3c 
#define adc_Address 0x48 

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO

#define LOOPTIME 1000 // 1 second 

// parameteric scaling for A/D measurement
#define VOLTS_ZERO 0.5
#define VOLTS_SCALE 4.5
#define AD_VOLTS 2.048
#define AD_SCALE 65536.0
#define COUNTS_ZERO ( (VOLTS_ZERO / AD_VOLTS) * AD_SCALE) // 16000.0
#define COUNTS_SCALE ( (VOLTS_SCALE / AD_VOLTS) * AD_SCALE)// 144000.0
#define UNIT_ZERO 0.0
#define UNIT_SCALE 100.0 // 100 psig sensor

// dynamic data x,y position on screen
#define TIME_POS 65,1
#define FLOW_POS 0,50
#define PRES_POS 0,25
#define LEVEL_POS 65,25
#define VOLUME_POS 55,50

const char* ssid = "ABRACADABRA-2G";
const char* password = "OPENSESAME";

const char* mqtt_server = "10.0.0.111";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

Adafruit_ADS1115 adc;
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

long lastMsgTime = 0;
char msg[256];
uint16_t counts = 0;

float pressure=0, flowrate=0, level=0, volume=0;

JsonDocument report;
JsonDocument update;

void setup()
{
  Serial.begin(9600);
  Serial.println("Pressure Monitor Start");

  Wire.begin(20,21); // (20,21) for ESP32-S3

  display.begin(oled_Address, true); // Address 0x3C default
  display.clearDisplay();
  display.display();
  display_static();

  adc.begin(adc_Address);
  adc.setGain(GAIN_TWO);

  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  char pb[10];

  if (String(topic) == "tankLevel") {
  // parse the messageTemp JSON and extract time string and level + volume estimates
  deserializeJson(update, messageTemp);
  const char* time = update["time"];
  level = update["level"];
  volume = update["volume"];
  Serial.print("Time: ");
  Serial.println(time);

  // update the display

  display.setCursor(TIME_POS);
  display.print(time);

  display.setCursor(LEVEL_POS);
  display.setFont(&FreeSans9pt7b);
  display.fillRect(60,15,38,20,SH110X_BLACK);
  sprintf(pb, "%3.0f", level);
  display.print(pb);
  display.setFont();  

  display.setCursor(VOLUME_POS);
  display.setFont(&FreeSans9pt7b);
  display.fillRect(53,40,45,20,SH110X_BLACK);
  sprintf(pb, "%4.0f", volume);
  display.print(pb);
  display.setFont();  
  display.display();
  }
  else if (String(topic) == "flowRate") {
  // parse the JSON and extract flow rate value
  deserializeJson(update, messageTemp);
  flowrate = update["flowrate"];
  // update the display
  display.setCursor(FLOW_POS);
  display.setFont(&FreeSans9pt7b);
  display.fillRect(0,40,28,20,SH110X_BLACK);
  sprintf(pb, "%01.1f", flowrate);
  display.print(pb);
  display.setFont(); 
  display.display();
  }
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

void display_static() {
  display.setFont(); // default small text font
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);

  display.setCursor(5, 1);
  display.print("current");

  display.setCursor(PRES_POS);
  display.setFont(&FreeSans9pt7b);
  display.print("--.-");
  display.setFont();  

  display.setCursor(40, 25);
  display.print("psi");

  display.setCursor(FLOW_POS);
  display.setFont(&FreeSans9pt7b);
  display.print("-.-");
  display.setFont();  

  display.setCursor(30, 50);
  display.print("gpm");

  display.setCursor(TIME_POS);
  display.print("--.--.--");

  display.setCursor(LEVEL_POS);
  display.setFont(&FreeSans9pt7b);
  display.print("---");
  display.setFont();  

  display.setCursor(100, 25);
  display.print("%");

  display.setCursor(VOLUME_POS);
  display.setFont(&FreeSans9pt7b);
  display.print("----");
  display.setFont();  

  display.setCursor(100, 50);
  display.print("gal");

  display.display();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsgTime > LOOPTIME) {
    lastMsgTime = now;
    // read the A/D converter
    counts = adc.readADC_SingleEnded(0);
    Serial.print("Counts = ");
    Serial.println(counts);
    // convert counts to psi value
    pressure = UNIT_ZERO + ( ( (float)counts - COUNTS_ZERO ) / ( COUNTS_SCALE - COUNTS_ZERO ) ) * ( UNIT_SCALE - UNIT_ZERO);
    Serial.print("Pressure = ");
    Serial.println(pressure);
    // update display
    char pb[10];
    sprintf(pb, "%02.1f", pressure);
    display.setCursor(PRES_POS);
    display.setFont(&FreeSans9pt7b);
    display.fillRect(0,15,35,20,SH110X_BLACK);
    display.print(pb);
    display.setFont();
    display.display();

    // encode as JSON
    report["pressure"] = pressure;
    serializeJson(report, msg);
    // publish to pressure topic
    mqttClient.publish("pressure", msg);
  }
}
