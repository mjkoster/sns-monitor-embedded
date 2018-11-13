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

void setup_pms5003() {
  //setup the input buffer
  pms5003_buffer_index = 0;
  Serial3.begin(9600);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delayMicroseconds(100);
  digitalWrite(2, HIGH);
}

void process_pms5003_buffer() {
  
  // get checksum ready
  uint16_t sum = 0;
  for (uint8_t i=0; i<30; i++) {
    sum += pms5003_buffer[i];
  }
 
  /* debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(pms5003_buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
  */
  
  // The data comes in endian'd, this solves it so it works on all platforms
  for (uint8_t i=0; i<15; i++) {
    pms5003_buffer_u16[i] = pms5003_buffer[2 + i*2 + 1];
    pms5003_buffer_u16[i] += (pms5003_buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&pms5003_data, (void *)pms5003_buffer_u16, 30);
 
  if (sum != pms5003_data.checksum) {
    Serial.println("Checksum failure");
  }
  else {
    /*
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(pms5003_data.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(pms5003_data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(pms5003_data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(pms5003_data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(pms5003_data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(pms5003_data.pm100_env);
    Serial.println("---------------------------------------");
    Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(pms5003_data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(pms5003_data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(pms5003_data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(pms5003_data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(pms5003_data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(pms5003_data.particles_100um);
    Serial.println("---------------------------------------");    
    */
  }
}

void serialEvent3() {
  //PMS5003 serial data handler
  // get all the bytes and put them in the buffer
  // when the buffer is full to the expected size, check the content and update the resource state variables
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
    process_pms5003_buffer();
    pms5003_buffer_index = 0;
    break;
    }
  }
}

//pms5003 accessors

uint16_t pms5003_get_pm1_0(){
  return(pms5003_data.pm10_standard);
}

uint16_t pms5003_get_pm2_5(){
  return(pms5003_data.pm25_standard);
}

uint16_t pms5003_get_pm10_0(){
  return(pms5003_data.pm100_standard);
}

float sgp30_get_coeq() {
  return(0);
}

float sgp30_get_tvoc() {
  return(0);
}

float mics_6814_get_co() {
  return(0);
}

float mics_6814_get_no2() {
  return(0);
}

float mics_6814_get_c2h6oh() {
  return(0);
}

float mics_6814_get_h2() {
  return(0);
}

float mics_6814_get_nh3() {
  return(0);
}

float mics_6814_get_ch4() {
  return(0);
}

float mics_6814_get_c3h8() {
  return(0);
}

float mics_6814_get_c4h10() {
  return(0);
}

float hp206c_get_barometer() {
  return(0);
}


