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
  pinMode(2, OUTPUT); // put it in and out of sleep mode to synchronize the output
  digitalWrite(2, LOW);
  delayMicroseconds(100);
  digitalWrite(2, HIGH);
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

// HP206C
#include <HP20x_dev.h>
#include <KalmanFilter.h>

void hp206c_setup() {
  HP20x.begin();
  delay(100);
  if( OK_HP20X_DEV != HP20x.isAvailable() ) {
    Serial.println("hp206C failed to setup");
  }
  return;
}

float hp206c_get_temperature() {
  return(HP20x.ReadTemperature()/100.0 ); // C
}

float hp206c_get_barometer() {
  return(HP20x.ReadPressure()/100.0 ); // hPa
}

float hp206c_get_altitude() {
  return(HP20x.ReadAltitude()/100.0 ); // m
}

