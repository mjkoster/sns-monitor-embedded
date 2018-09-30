
#include <stdio.h>
#include <stdlib.h>
#include <Time.h>
#include <math.h>
#include <ArduinoCbor.h>

#define true 1
#define false 0

/*
 * the resources.h file contains the resource struct definition and the instance definitions
 */
#include "resources.h"

/*
 * the functions file contains resource specific functions and handlers
 */
#include "functions.h"


/* 
 * serial input and resource update processing
 */
#define BUFSIZE 200

boolean receive_input(Stream *s) {
  if ( ! s->available() ) {
    return(false);
  }
  s->setTimeout(10);
  process_input( s );
}

void process_input ( Stream *s ) {
  uint8_t input_buffer[BUFSIZE];
  unsigned int length = s->readBytes( input_buffer, BUFSIZE );
  CborBuffer buffer(BUFSIZE);
  CborVariant cbor_pack = buffer.decode( input_buffer, length );
  if ( ! cbor_pack.isArray() ) {
    s->print("Not array type pack 0x");
    s->println(cbor_pack.isValid(), HEX);
    return;
  }
  else {
    int length = cbor_pack.length();
    CborArray outer = cbor_pack.asArray();
    for ( int i=0; i<length; i++) {
      if (! outer.get(i).isObject()) {
        Serial.println("Not Object");
        return;
      }
      else
        process_item( outer.get(i).asObject() );
    }
  }
  return;
}

void process_item( CborObject item ) {
  Serial.print("process_item ");
  int objid = item.get("bn").asArray().get(0).asInteger() ;
  int objinst = item.get("bn").asArray().get(1).asInteger() ;
  int resid = item.get("bn").asArray().get(2).asInteger() ;
  
  if ( item.get("vb").isInteger() ) {
    boolean vb = item.get("vb").asInteger();
    if ( vb != 0 || vb != 1 ) {
      return;
    }
    else {
      
    }
    Serial.println("vb=integer");
  }
  else if ( item.get("vs").isString() ) {
    Serial.println("vs=string");
  }
  else if ( item.get("v").isInteger() ) {
    Serial.println("v=integer");
  }
  else if ( item.get("v").isValid() ) {
    Serial.println("v=other");
  }
  else {
    Serial.println("?");
  }
  return;
}


/* 
 *  resource sampling and report processing
 */
unsigned int report_resource (Resource * resource, Stream *s) {

  s->print (resource->objid);
  s->print ("/");
  s->print (resource->objinst);
  s->print ("/");
  s->print (resource->resid);
  s->print (": ");

  if ( num_type == resource->type )
    s->println(resource->v);
  else if ( str_type == resource->type )
    s->println(resource->vs);
  else if ( bool_type == resource->type )
    s->println(resource->vb ? "true" : "false");

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
      report_resource(resource, &Serial);
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
      receive_input(&Serial);
    }; // busy wait for the next wakeup interval to pass
    last_wakeup += wakeup_interval;
  }
  return(1);
}
