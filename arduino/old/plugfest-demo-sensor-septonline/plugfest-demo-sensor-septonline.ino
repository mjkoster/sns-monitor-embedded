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

// dust sensor variables
int dust_pin = 4;
unsigned long duration;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

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
  pinMode(dust_pin,INPUT);
  
  // initial interval start times
  starttime = millis();//get the current time;
  dust_starttime = millis();//get the current time;

}

void loop() {
  
// check serial input


// check dust sensor
  duration = pulseIn(dust_pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-dust_starttime) >= dust_sampletime_ms)
  {
    ratio = lowpulseoccupancy/(dust_sampletime_ms*10.0);  // Integer percentage 0 to 100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("PM2.5 ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    lowpulseoccupancy = 0;
    dust_starttime = millis();
  }

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
