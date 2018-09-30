
#include <stdio.h>
#include <stdlib.h>
#include <Time.h>
#include <math.h>

#define true 1
#define false 0


/* 
 * resource template
 */
typedef enum value_t { num_type, str_type, bool_type };
typedef enum gpio_t { no_gpio, ain_type, pwm_type, din_type, dout_type, ser_type, func_type };

typedef struct Resource {
    // LWM2M Object ID, Instance, and Resource ID
    short objid; 
    short objinst;
    short resid;

    value_t type;

    float v;
    float vmin; // Min and Max range values for conditionals and limiting
    float vmax;

    char* vs;
    unsigned int max_strlen; // to limit string length

    unsigned char vb;
    unsigned char invert; // boolean for inverting the digital sense

    // values that were transmitted in the last (most recent past) report
    float last_rep_v;
    char* last_rep_vs;
    unsigned char last_rep_vb;

    time_t last_rep_time; // timestamp of the value from the last report
    // Conditionals for report timing, see IETF CoRE Dynlink
    time_t pmin; // minimum interval between reports
    time_t pmax; // maximum interval between reports
    // Conditionals for scalar values, see IETF CoRE Dynlink
    float lt; // upper limit, reports samples less than lt in band mode
    float gt; // lower limit, reports samples greater than gt in band mode
    float st; // step limit, amount sample must change to generate a report
    unsigned char band; // set true to notify when sample is between gt and lt

    // synchronous updater, gpio or input function
    time_t sample_interval; // inter-sample timing for gpio and function samplers
    time_t last_sample_time; // when the last inter-sample interval ended

    gpio_t gpio;
    unsigned short gpio_pin;
    // for scaling analog pins to vmin/vmax
    int vmin_counts; // a/d counts corresponding to vmin
    int vmax_counts; // a/d counts corresponding to vmax

    void (*init_function)(Resource *); // pointer to function to initialize the gpio driver (if needed)
    unsigned int (*sample_function)(Resource *); // pointer to a function to update resource->v
    unsigned int (*onupdate)(Resource *); // pointer to a function call on updates
    unsigned int (*onreport)(Resource *); // pointer to a function call on reports
};

/* 
 * resource instances
 */
 
unsigned int run_hot_cold (Resource * r);
 
Resource R3300_0_5700 = {
  3300,  // objid 
  0,  // objinst 
  5700, // resid 
  num_type, // type
  50, // v
  0, //vmin
  100, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  50, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  1000, // pmin
  5000, // pmax
  20, // lt
  80, // gt
  1, // st
  true, // band
  1000, // sample_interval
  0, // last_sample_time
  ain_type, // gpio
  14, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  NULL, // *sample_function
  NULL, // *onupdate
  NULL // *onreport
};

Resource R3300_1_5700 = {
  3300,  // objid 
  1,  // objinst 
  5700, // resid 
  str_type, // type
  0, // v
  0, //vmin
  0, //vmax
  "", // vs
  10, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  10000, // pmax
  0, // lt
  0, // gt
  0, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // gpio
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  run_hot_cold, // *sample_function
  NULL, // *onupdate
  NULL // *onreport
};

Resource R3300_2_5700 = {
  3300,  // objid 
  2,  // objinst 
  5700, // resid 
  bool_type, // type
  0, // v
  0, //vmin
  0, //vmax
  "", // vs
  10, // max_strlen
  false, // vb
  false, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  2000, // pmin
  10000, // pmax
  0, // lt
  0, // gt
  0, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  din_type, // gpio
  5, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  NULL, // *sample_function
  NULL, // *onupdate
  NULL // *onreport  
};

Resource * resource_list[] = {
  &R3300_0_5700,
  &R3300_1_5700,
  &R3300_2_5700
};

/*
 * Resource type specific functions and handlers
 */
unsigned int run_hot_cold (Resource * r) {
  if ( (int) millis() & 1 ) {
    r->vs = "cold";
  }
  else {
    r->vs = "hot";
  }
  return(true);
}


/* 
 *  resource processing
 */
unsigned int report_resource (Resource * resource) {

  Serial.print (resource->objid);
  Serial.print ("/");
  Serial.print (resource->objinst);
  Serial.print ("/");
  Serial.print (resource->resid);
  Serial.print (": ");

  if ( num_type == resource->type )
    Serial.println(resource->v);
  else if ( str_type == resource->type )
    Serial.println(resource->vs);
  else if ( bool_type == resource->type )
    Serial.println(resource->vb ? "true" : "false");

  resource->last_rep_time = resource->last_sample_time;

  return(true);
}


unsigned int process_sample (Resource * resource) {
/*
drives sampling of resources based on a GPIO pin or function call.
A/D counts ace scaled to vmin/vmax based on vmin_coiuunts and vmax_counts
parameters. Normally vmin_counts will be 0 and vmax_couts will be 1023
but this can be adapted to various sensors.
*/
  if ( ( dout_type == resource->gpio ) ||
  ( pwm_type == resource->gpio ) ||
  ( no_gpio == resource->gpio ) ) {
    return(true);
  }

  else if ( ain_type == resource->gpio ) {
    // read analog input gpio
    int counts = round ( ( (float) rand() / (float) RAND_MAX ) * 1023 );
    // int counts = analogRead(resource->gpio_pin);
    // pre-calculate the counts scale and the value scale
    int count_scale = resource->vmax_counts - resource->vmin_counts;
    float v_scale = resource->vmax - resource->vmin;
    // limit the counts before scaling
    counts = (counts < resource->vmin_counts ? resource->vmin_counts : counts);
    counts = (counts > resource->vmax_counts ? resource->vmax_counts : counts);
    counts -= resource->vmin_counts; // re-zero the counts, then scale, then add offset
    resource->v = (float) counts * (v_scale / (float) count_scale) + resource->vmin;
    return(true);
  }
  else if ( din_type == resource->gpio ) {
    // read gpio pin and apply the inversion parameter
    resource->vb = resource->invert ^ (unsigned char) round ( (float) rand() / (float) RAND_MAX );
    // resource->vb = resource->invert ^ digitalRead(resource->gpio_pin);
    return(true);
  }
  else if ( ser_type == resource->gpio ) {
    return(true); // handle serial interface
  }
  else if ( func_type == resource->gpio ) {
    resource->sample_function(resource);
    return(true); // handle function call interface
  }
  else {
    return(true); // if there was no GPIO update
  }
}


unsigned int apply_conditionals (Resource * resource) {

  // if pmin is not yet exceeded, return indicating no notification
  if (resource->last_sample_time - resource->last_rep_time < resource->pmin) {
    return(false);
  }
  if (num_type == resource->type) {
    // notify on gt crossings if not in band mode, gt == vmax to disable gt limiting
    if (  (false == resource->band) && (resource->gt < resource->vmax) &&
    ( (resource->v > resource->gt && resource->last_rep_v <= resource->gt) ||
    (resource->v <= resource->gt && resource->last_rep_v > resource->gt) ) )
    {
      resource->last_rep_v = resource->v;
      return(true);
    }
    // notify on lt crossings if not in band mode, lt == vmin to disable lt limiting
    else if ( (false == resource->band) && (resource->lt > resource->vmin) &&
    ( (resource->v < resource->lt && resource->last_rep_v >= resource->lt) ||
    (resource->v >= resource->lt && resource->last_rep_v < resource->lt) ) )
    {
      resource->last_rep_v = resource->v;
      return(true);
    }
    // notify on either step or pmax exceeded if not in band mode
    // set step == 0 to disable value filtering
    else if ( false == resource->band &&
      ( ( resource->v - resource->last_rep_v >= resource->st
      || resource->last_rep_v - resource->v >= resource->st )
      || (resource->last_sample_time - resource->last_rep_time >= resource->pmax) ) )
    {
      resource->last_rep_v = resource->v;
      return(true);
    }
    // notify on either step or pmax exceeded if in band mode and in band
    // set step == 0 to disable value filtering
    else if ( true == resource->band && (
      ( (resource->v >= resource->gt) && (resource->v <= resource->lt) )
      ||( (resource->v >= resource->gt) && (resource->gt >= resource->lt) )
      || ( (resource->v <= resource->lt) && (resource->gt >= resource->lt) ) ) // in band
      && ( ( resource->v - resource->last_rep_v >= resource->st
        || resource->last_rep_v - resource->v >= resource->st )
        || (resource->last_sample_time - resource->last_rep_time >= resource->pmax) ) )
    {
      resource->last_rep_v = resource->v;
      return(true);
    }
    else
      // no numeric value condition to notify
      return(false);
  }
  else {
    // evaluate value change and pmax for boolean and string types
    if ( resource->vb != resource->last_rep_vb ) {
      resource->last_rep_vb = resource->vb;
      return(true);
    }
    else if ( resource->vs != resource->last_rep_vs ) {
      resource->last_rep_vs = resource->vs;
      return(true);
    }
    else if (resource->last_sample_time - resource->last_rep_time >= resource->pmax) {
      resource->last_rep_time = resource->last_sample_time;
      return(true); // no value change
    }
    else
      // no notifiable value type
      return(false);
  }
}


unsigned int process_resource (Resource * resource, time_t timestamp) {
  if (timestamp - resource->last_sample_time >= resource->sample_interval) {
    resource->last_sample_time += resource->sample_interval;
    process_sample(resource);
    if (apply_conditionals(resource)) {
      report_resource(resource);
    }
  }
  return(true);
}


unsigned int init_resource (Resource * resource) {

  time_t init_time = millis();
  resource->last_sample_time = init_time - resource->sample_interval;

  if ( ain_type == resource->gpio ) {
    pinMode(resource->gpio_pin, INPUT);
    return(true);
  }
  else if ( din_type == resource->gpio ) {
    pinMode(resource->gpio_pin, INPUT);
    return(true);
  }
  else if ( pwm_type == resource->gpio ) {
    return(true);
  }
  else if ( dout_type == resource->gpio ) {
    return(true);
  }
  if (NULL != resource->init_function) {
    resource->init_function(resource);
  }
  return(true);
};

// serial input
#define CR 0x0C
#define LF 0x0A
#define BUFSIZE 50

unsigned char cr_received = false;
unsigned char input_buffer[BUFSIZE];

void process_input_byte(unsigned char in_byte) {
  if ( CR == in_byte ) {
    cr_received = true;
    return;
  }
}

void receive_input() {
  while (Serial.available() > 0) {
    process_input_byte(Serial.read());
  }
}

void setup() {
  
  Serial.begin(9600);
  for ( int i = 0; i < ( sizeof(resource_list) / sizeof(resource_list[0]) ); i++) {
    init_resource (resource_list[i]);
  };
  return(1);
}


void loop() {
  time_t last_wakeup = millis();
  time_t wakeup_interval = 1;

  while (true) {

    for ( int i = 0; i < ( sizeof(resource_list) / sizeof(resource_list[0]) ); i++) {
      process_resource (resource_list[i], last_wakeup);
    };

    while (millis() - last_wakeup < wakeup_interval) {
      receive_input();
    }; // busy wait for the next wakeup interval to pass
    last_wakeup += wakeup_interval;
  }
  return(1);
}
