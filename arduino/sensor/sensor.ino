
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
  s->setTimeout(100);
  process_input( s );
}

void process_input ( Stream *s ) {
  uint8_t input_buffer[BUFSIZE];
  unsigned int length = s->readBytes( input_buffer, BUFSIZE );
  Serial.print("length "); 
  Serial.println(length);
  CborBuffer buffer(BUFSIZE);
  CborVariant cbor_pack = buffer.decode( input_buffer, length );
  if ( ! cbor_pack.isArray() ) {
    s->print("Not array type pack 0x");
    s->println(cbor_pack.isValid(), HEX);
    return;
  }
  else {
    unsigned int length = cbor_pack.length();
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
  Serial.print("item ");
  int objid = item.get("bn").asArray().get(0).asInteger() ;
  int objinst = item.get("bn").asArray().get(1).asInteger() ;
  int resid = item.get("bn").asArray().get(2).asInteger() ;
  
  if ( item.get("vb").isInteger() ) {
    boolean vb = item.get("vb").asInteger();
    if ( 0 == vb || 1 == vb ) {
      Serial.print("vb=integer ");
      Serial.println( item.get("vb").asInteger() );
    }
  }
  else if ( item.get("vs").isString() ) {
    Serial.print("vs=string ");
    Serial.println( item.get("vs").asString() );
  }
  else if ( item.get("v").isInteger() ) {
    Serial.print("v=integer ");
    Serial.println( item.get("v").asInteger() );
  }
  else if ( item.get("v").isFloat() ) {
    Serial.print("v=float ");
    Serial.println( item.get("v").asFloat() );
  }
  else if ( item.get("vb").isBoolean() ) {
    Serial.print("vb=boolean ");
    Serial.println( item.get("vb").asBoolean() );
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

  if ( num_type == resource->type ) {
    s->println(resource->v);
    resource->last_rep_v = resource->v;
  }
  else if ( str_type == resource->type ) {
    s->println(resource->vs);
    resource->last_rep_vs = resource->vs;
  }

  else if ( bool_type == resource->type ) {
    s->println(resource->vb ? "true" : "false");
    resource->last_rep_vb = resource->vb;
  }
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

bool notifiable( Resource * r ) {

#define BAND r->band
#define SCALAR_TYPE ( num_type == r->type )
#define STRING_TYPE ( str_type == r->type )
#define BOOLEAN_TYPE ( bool_type == r->type )
#define PMIN_EX ( r->last_sample_time - r->last_rep_time >= r->pmin )
#define PMAX_EX ( r->last_sample_time - r->last_rep_time > r->pmax )
#define LT_EX ( r->v < r->lt ^ r->last_rep_v < r->lt )
#define GT_EX ( r->v > r->gt ^ r->last_rep_v > r->gt )
#define ST_EX ( abs( r->v - r->last_rep_v ) >= r->st )
#define IN_BAND ( ( r->gt <= r->v && r->v <= r->lt ) || ( r->v >= r->gt && r->gt >= r->lt ) || ( r->v <= r->lt && r->lt <= r->gt ) )
#define VB_CHANGE ( r->vb != r->last_rep_vb )
#define VS_CHANGE ( r->vs != r->last_rep_vs )

  return (
    PMIN_EX &&
    ( SCALAR_TYPE ?
      ( ( !BAND && ( GT_EX || LT_EX || ST_EX || PMAX_EX ) ) ||
        ( BAND && IN_BAND && ( ST_EX || PMAX_EX) ) )
    : STRING_TYPE ?
      ( VS_CHANGE || PMAX_EX )
    : BOOLEAN_TYPE ?
      ( VB_CHANGE || PMAX_EX )
    : false )
  );
}

unsigned int process_resource (Resource * resource, time_t timestamp) {
  if (timestamp - resource->last_sample_time >= resource->sample_interval) {
    resource->last_sample_time += resource->sample_interval;
    process_sample(resource);
    if ( notifiable(resource) ) {
      report_resource( resource, &Serial );
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
  time_t wakeup_interval = 1000;

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
