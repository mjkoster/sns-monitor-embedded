/*
WoT Plugfest Arduino Sensors demo
 */

// gpio sensor variables
int spl_pin = A0;
int illuminance_pin = A1;
int motion_pin = 7;
int illuminance, motion, spl, last_illuminance, last_motion, last_spl = 0;
int illuminance_step = 50; 
int spl_step = 10;
float illuminance_scale = .12;
float spl_scale = 1;

// DHT humidity and temperature sensor
int dht_pin = 2;

// sampling interval parameters
unsigned long starttime;
unsigned long sampletime = 200;
unsigned long dust_starttime;
unsigned long dust_sampletime_ms = 30000;

void setup() {
  Serial.begin(9600);

 
  // Pin modes
  pinMode(motion_pin,INPUT);
  pinMode(spl_pin,INPUT);
  pinMode(illuminance_pin,INPUT);
 
}

void loop() {
  
// check serial input

// check for end of sample interval
  if ((millis()-starttime) >= sampletime)
  {
    
// read current sensor state from pins
    motion = digitalRead(motion_pin);
    illuminance = analogRead(illuminance_pin);
    spl = analogRead(spl_pin);

// only report changes >= step
    if (motion != last_motion)
    {
      Serial.print("Motion ");
      Serial.print(motion);
      Serial.println(" ");
      last_motion = motion;
    }

    if ( (illuminance > last_illuminance + illuminance_step) || (illuminance < last_illuminance - illuminance_step) )
    {
      Serial.print("Illuminance ");
      Serial.print(float(illuminance*illuminance_scale));
      Serial.println(" ");
      last_illuminance = illuminance;
    }
    
    if ( (spl > last_spl + spl_step) || (spl < last_spl - spl_step) )
    {
      Serial.print("SPL ");
      Serial.print( float(spl*spl_scale));
      Serial.println(" ");
      last_spl = spl;
    }    
    starttime = millis();
  } 
}
