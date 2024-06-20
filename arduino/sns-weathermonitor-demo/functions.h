/*
 * Resource type specific functions and handlers
 */
// 

#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht20;

void aht20_setup() {
  aht20.begin();
  return;
}

float aht20_get_temperature() {
  sensors_event_t temp, humidity;
  aht20.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  return(temp.temperature); // C
}

float aht20_get_humidity() {
  sensors_event_t temp, humidity;
  aht20.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  return(humidity.relative_humidity);  // %RH
}

/*
// am2315
#include <Wire.h>
#include <Adafruit_AM2315.h>

Adafruit_AM2315 am2315;

void am2315_setup() {
  am2315.begin();
  return;
}

float am2315_get_temperature() {
  return(am2315.readTemperature()); // C
}

float am2315_get_humidity() {
  return(am2315.readHumidity());  // %RH
}

// Si1145
#include <Wire.h>

#include "Arduino.h"
#include "SI114X.h"
SI114X SI1145 = SI114X();

void si1145_setup() {
  SI1145.Begin();
  return;
}

float si1145_get_visible() {
  return(SI1145.ReadVisible()); 
}

float si1145_get_ir() {
  return(SI1145.ReadIR()); 
}

float si1145_get_uv() {
  return((float)SI1145.ReadUV()/100); 
}
*/

// weather rack
#include <Wire.h>
#include <Time.h>

#include "SDL_Weather_80422.h"

#define pinLED     13   // LED connected to digital pin 13
#define pinAnem    2  // Anenometer connected to pin 18 - Int 5 - Mega   / Uno pin 2
#define pinRain    3  // Anenometer connected to pin 2 - Int 0 - Mega   / Uno Pin 3 
#define intAnem    0  // int 0 (check for Uno)
#define intRain    1  // int 1

SDL_Weather_80422 weather_rack(pinAnem, pinRain, intAnem, intRain, A1, SDL_MODE_INTERNAL_AD);

void weather_rack_setup() {
  weather_rack.setWindMode(SDL_MODE_SAMPLE, 5.0);
}

float weather_rack_get_wind_direction() {
  return(weather_rack.current_wind_direction());
}

float weather_rack_get_wind_speed() {
  return(weather_rack.current_wind_speed());
}

float weather_rack_get_wind_gust() {
  return(weather_rack.get_wind_gust());
}

float weather_rack_get_rain_total() {
  return(weather_rack.get_current_rain_total());
}





  

