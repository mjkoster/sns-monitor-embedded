/*
 * Resource type specific functions and handlers
 */

// am2302
#include "DHT.h"
#define DHTPIN 8     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

void am2302_setup() {
  dht.begin();
  return;
}

float am2302_get_temperature() {
  return(dht.readTemperature()); // C
}

float am2302_get_humidity() {
  return(dht.readHumidity());  // %RH
}


