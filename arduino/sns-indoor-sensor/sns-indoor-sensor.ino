/*
Grove - Dust Sensor Demo v1.0
 Interface to Shinyei Model PPD42NS Particle Sensor
 Program by Christopher Nafis
 Written April 2012

 http://www.seeedstudio.com/depot/grove-dust-sensor-p-1050.html
 http://www.sca-shinyei.com/pdf/PPD42NS.pdf

 JST Pin 1 (Black Wire)  =&gt; //Arduino GND
 JST Pin 3 (Red wire)    =&gt; //Arduino 5VDC
 JST Pin 4 (Yellow wire) =&gt; //Arduino Digital Pin 8
 */


int pin = 4;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

int illuminance_pin = A0;
int motion_pin = 7;
unsigned long motion_starttime;
unsigned long motion_sampletime = 200;
int motion = 0;
int ill_signal;
int last_motion = 0;

void setup() {
  Serial.begin(9600);
  
// dust sensor setup
  pinMode(pin,INPUT);
  starttime = millis();//get the current time;
  
// motion sensor setup
  pinMode(motion_pin,INPUT);
  pinMode(illuminance_pin,INPUT);
  motion_starttime = starttime;
}

void loop() {

// check dust sensor
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-starttime) >= sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=&gt;100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("PM2.5 ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    lowpulseoccupancy = 0;
    starttime = millis();
  }

// check motion sensor
  if ((millis()-motion_starttime) >= motion_sampletime)
  {
//    motion = digitalRead(motion_pin);
    ill_signal = analogRead(illuminance_pin);
    motion = 0;
    if (ill_signal > 50) 
      motion = 1;
      
    if (motion != last_motion)
    {
      Serial.print("Motion ");
      Serial.print(motion);
      Serial.println(" ");
      last_motion = motion;
    }
    motion_starttime = millis();
  }
  
}
