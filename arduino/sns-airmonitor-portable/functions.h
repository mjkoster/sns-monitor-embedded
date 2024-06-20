/*
 * Resource type specific functions and handlers
 */

// pms5003
#define PMS5003_BUFSIZE 32
uint8_t pms5003_buffer[PMS5003_BUFSIZE];
uint8_t pms5003_buffer_index;

// integer buffer for endian correction
uint16_t pms5003_buffer_u16[15];

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
 
struct pms5003data pms5003_data;

void pms5003_setup() {
  //setup the input buffer
  pms5003_buffer_index = 0;
  Serial3.begin(9600);
  //pinMode(2, OUTPUT); // put it in and out of sleep mode to synchronize the output
  //digitalWrite(2, LOW);
  //delayMicroseconds(100);
  //digitalWrite(2, HIGH);
}

void pms5003_process_buffer() {
  
  // checksum
  uint16_t sum = 0;
  for (uint8_t i=0; i<30; i++) {
    sum += pms5003_buffer[i];
  }
   
  // this works on any byte order platform
  for (uint8_t i=0; i<15; i++) {
    pms5003_buffer_u16[i] = pms5003_buffer[2 + i*2 + 1];
    pms5003_buffer_u16[i] += (pms5003_buffer[2 + i*2] << 8);
  }
  if (sum == pms5003_buffer_u16[14]) {
    // load the struct
    memcpy((void *)&pms5003_data, (void *)pms5003_buffer_u16, 30);
  }
  else {
    Serial.println("Checksum failure");
  }
  return;
}

void serialEvent3() {
  // PMS5003 serial data handler
  // skip over any leading bytes that are not the 0x42 start flag; then get bytes and put them in the buffer
  // when the buffer is full to the expected size, check the contents and process the buffer
  // 
  while (Serial3.available()) {
  // get the new byte:
  pms5003_buffer[pms5003_buffer_index++] = Serial3.read();
  // packet is aligned when the first byte is 0x42
  if ( 1 == pms5003_buffer_index && pms5003_buffer[0] != 0x42 ) {
    pms5003_buffer_index = 0;
  }
  // check to see if the expected number of bytes have been recieved
  if ( pms5003_buffer_index >= PMS5003_BUFSIZE) {
    pms5003_process_buffer();
    pms5003_buffer_index = 0;
    break;
    }
  }
}

float pms5003_get_pm1_0(){
  return(pms5003_data.pm10_standard); // ug/m3
}

float pms5003_get_pm2_5(){
  return(pms5003_data.pm25_standard); // ug/m3
}

float pms5003_get_pm10_0(){
  return(pms5003_data.pm100_standard); // ug/m3
}

///*
// sgp30
#include "Adafruit_SGP30.h"
Adafruit_SGP30 sgp30;
  
void sgp30_setup() {
    if (! sgp30.begin()){
    Serial.println("Sensor not found :(");
  }
  return;
}

float sgp30_get_coeq() {
  if (! sgp30.IAQmeasure()) {
    Serial.println("Measurement failed");
    return(-1);
  }
  return(sgp30.eCO2); // ppm
}

float sgp30_get_tvoc() {
  if (! sgp30.IAQmeasure()) {
    Serial.println("Measurement failed");
    return(-1);
  }
  return(sgp30.TVOC); // ppb
}

//*/ /*
// mics 6814
#include "MutichannelGasSensor.h"

void mics6814_setup() {
  gas.begin(0x04);//the default I2C address of the slave is 0x04
  gas.powerOn();
  return;
}

float mics6814_get_co() {
  return(gas.measure_CO());
}

float mics6814_get_no2() {
  return(gas.measure_NH3());
}

float mics6814_get_c2h5oh() {
  return(gas.measure_C2H5OH());
}

float mics6814_get_h2() {
  return(gas.measure_H2());
}

float mics6814_get_nh3() {
  return(gas.measure_NH3());
}

float mics6814_get_ch4() {
  return(gas.measure_CH4());
}

float mics6814_get_c3h8() {
  return(gas.measure_C3H8());
}

float mics6814_get_c4h10() {
  return(gas.measure_C4H10());
}
//*/

// temp, humidity, barometer, gas

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

Adafruit_BME680 bme; // I2C

void bme680_setup() {
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    return;
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(100, 1); // 320*C for 150 ms // don't use gas measurement 
}

float bme680_get_temperature() {
  return(bme.temperature);
}

float bme680_get_humidity() {
  return(bme.humidity);
}

float bme680_get_barometer() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
  }
  return(bme.pressure / 100.0);
}

// air quality sensor
void ccs811_setup() {
  
}

float ccs811_get_eCO2() {
  
}

float ccs811_get_tvoc() {
  
}

// luminance sensor
#include <Wire.h>
#include <Digital_Light_TSL2561.h>
//#include <Digital_Light_ISL29035.h>

void tsl2561_setup() {
  Wire.begin();
  TSL2561.init();
  //ISL29035.init();
}

float tsl2561_get_illuminance() {
  return(TSL2561.readVisibleLux());
  //return(ISL29035.readVisibleLux());
}


// RGB LED on frontpanel

#include <ChainableLED.h>
ChainableLED rgb_led (4, 5, 1);

void rgb_led_setup() {
  rgb_led.setColorRGB(0,0,0,0);
}

float rgb_led_set_color() {
  rgb_led.setColorRGB(0,0,0,0);
}


// 10 segment LED bargraph
#include <Grove_LED_Bar.h>
Grove_LED_Bar led_bar(7, 6, 0);

void bargraph_led_setup() {
  led_bar.begin();
  led_bar.setLevel(0);
}

float bargraph_led_set_level() {
  led_bar.setLevel(0);
}


