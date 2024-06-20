/*
WoT Plugfest Arduino Sensors demo
 */

// gpio sensor variables
int pm2_5_pin = A4;
int led_pin = A3;
int illuminance_pin = A1;
int motion_pin = 7;
int illuminance, motion, pm2_5_counts, last_illuminance, last_motion, last_pm2_5 = 0;
int illuminance_step = 10; 
int pm2_5_step = 2;
float illuminance_scale = .12;
float pm2_5_scale = 10.74;
int pm2_5_mincounts = 7;
int pm2_5_maxcounts = int(3600.0/11.0 * 1024.0/5.0);

// DHT humidity and temperature sensor
int dht_pin = 2;

// sampling interval parameters
unsigned long starttime;
unsigned long sampletime = 1000;

void setup() {
  Serial.begin(9600);

 
  // Pin modes
  pinMode(motion_pin,INPUT);
  pinMode(pm2_5_pin,INPUT);
  pinMode(illuminance_pin,INPUT);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
 
}

void loop() {
  
// check serial input

// check for end of sample interval
  if ((millis()-starttime) >= sampletime)
  {
    
// read current sensor state from pins
    motion = digitalRead(motion_pin);
    illuminance = analogRead(illuminance_pin);
    digitalWrite(led_pin, HIGH);
    delayMicroseconds(280);
    pm2_5_counts = analogRead(pm2_5_pin);
    digitalWrite(led_pin, LOW);
 
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
    
    if ( (pm2_5_counts > last_pm2_5 + pm2_5_step) || (pm2_5_counts < last_pm2_5 - pm2_5_step) )
    {
      Serial.print("PM2.5 ");
      Serial.print( float((pm2_5_counts-pm2_5_mincounts)*pm2_5_scale));
      Serial.println(" ");
      last_pm2_5 = pm2_5_counts;
    }    
    starttime = millis();
  } 
}
