/*
WoT Plugfest Arduino Sensors demo
 */

// sampling interval parameters
unsigned long starttime;
unsigned long sampletime = 200;

int spl_pin = A3;
int illuminance_pin = A0;
int motion_pin = 8;
int illuminance, motion, spl, last_illuminance, last_motion, last_spl = 0;
int illuminance_step = 20; 
int spl_step = 100;
float illuminance_scale = 2;
float spl_scale = 0.1;

void setup() {
  Serial.begin(9600);
  
// interval start time
  starttime = millis();//get the current time;

// Pin modes
  pinMode(motion_pin,INPUT);
  pinMode(spl_pin,INPUT);
  pinMode(illuminance_pin,INPUT);
}

void loop() {

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
