#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define true 1
#define false 0

// resource template
typedef enum value_t { num_type, str_type, bool_type } value_t;
typedef enum gpio_t { no_gpio, ain_type, pwm_type, din_type, dout_type, ser_type, func_type } gpio_t;

typedef struct Resource {
    short objid; // LWM2M Object ID, Instance, and Resource ID
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
    float lt; // upper limit, indicates sample is less than
    float gt; // lower limit, indicates sample is greater than
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

    unsigned int (*sample_function)(); // pointer to a function to update resource->v
    unsigned int (*onupdate)(); // pointer to a function call on updates
    unsigned int (*onreport)(); // pointer to a function call on reports

} Resource;

// resource instances
Resource R3300_0_5700 = {
  .objid = 3300,
  .objinst = 0,
  .resid = 5700,
  .type = num_type,
  .v = 50,
  .vmin = 0,
  .vmax = 100,
  .last_rep_v = 50,
  .last_rep_time = 0,
  .pmin = 1,
  .pmax = 5,
  .lt = 20,
  .gt = 80,
  .st = 1,
  .band = true,
  .gpio = ain_type,
  .gpio_pin = 3,
  .vmin_counts = 0,
  .vmax_counts = 1023,
  .sample_interval = 1,
  .last_sample_time = 0
};

Resource R3300_1_5700 = {
  .objid = 3300,
  .objinst = 1,
  .resid = 5700,
  .type = str_type,
  .max_strlen = 10,
  .vs = "",
  .last_rep_vs = "",
  .last_rep_time = 0,
  .pmin = 5,
  .pmax = 10,
  .gpio = func_type,
  .sample_interval = 1,
  .last_sample_time = 0,
  .sample_function = NULL
};

unsigned int R3300_1_5700_sample () {
  if ( (int) time(NULL) & 1 ) {
    R3300_1_5700.vs = "cold";
  }
  else {
    R3300_1_5700.vs = "hot";
  }
  return(true);
}

Resource R3300_2_5700 = {
  .objid = 3300,
  .objinst = 2,
  .resid = 5700,
  .type = bool_type,
  .invert = false,
  .vb = false,
  .last_rep_vb = false,
  .last_rep_time = 0,
  .pmin = 2,
  .pmax = 10,
  .gpio = din_type,
  .gpio_pin = 5,
  .sample_interval = 1,
  .last_sample_time = 0
};

Resource * resource_list[] = {
  &R3300_0_5700,
  &R3300_1_5700,
  &R3300_2_5700
};

void bind_functions () {
  R3300_1_5700.sample_function = R3300_1_5700_sample;
};

// resource processing
unsigned int report_resource (Resource * resource) {

  printf ( "%i/%i/%i: ", resource->objid, resource->objinst, resource->resid );

  if ( num_type == resource->type )
    printf("%.2f\n", resource->v);
  else if ( str_type == resource->type )
    printf("%-10s\n", resource->vs);
  else if ( bool_type == resource->type )
    printf(resource->vb ? "true\n" : "false\n");

  resource->last_rep_time = resource->last_sample_time;

  return(true);
};


unsigned int process_sample (Resource * resource) {
/*
drives sampling of resources based on a GPIO pin or function call.
A/D counts ace scaled to vmin/vmax based on vmin_coiuunts and vmax_counts
parameters. Normallt vmin_counts will be 0 and vmax_couts will be 1023
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
    return(true);
  }
  else if ( ser_type == resource->gpio ) {
    return(true); // handle serial interface
  }
  else if ( func_type == resource->gpio ) {
    resource->sample_function();
    return(true); // handle function call interface
  }
  else {
    return(true); // if there was no GPIO update
  }
};


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
};


unsigned int process_resource (Resource * resource, time_t timestamp) {
  if (timestamp - resource->last_sample_time >= resource->sample_interval) {
    resource->last_sample_time += resource->sample_interval;
    process_sample(resource);
    if (apply_conditionals(resource)) {
      report_resource(resource);
    }
  }
  return(true);
};


unsigned int init_resource (Resource * resource) {

  time_t init_time = time(NULL);
  resource->last_sample_time = init_time - resource->sample_interval;

  if ( ain_type == resource->gpio ) {
    return(true);
  }
  else if ( din_type == resource->gpio ) {
    return(true);
  }
  else if ( pwm_type == resource->gpio ) {
    return(true);
  }
  else if ( dout_type == resource->gpio ) {
    return(true);
  }
  return(true);
};


unsigned int init (void) {
  bind_functions();
  for ( int i = 0; i < ( sizeof(resource_list) / sizeof(resource_list[0]) ); i++) {
    init_resource (resource_list[i]);
  };
  return(1);
};


unsigned int loop (void) {
  time_t last_wakeup = time(NULL);
  time_t wakeup_interval = 1;

  while (true) {

    for ( int i = 0; i < ( sizeof(resource_list) / sizeof(resource_list[0]) ); i++) {
      process_resource (resource_list[i], last_wakeup);
    };

    while (time(NULL) - last_wakeup < wakeup_interval) {
    }; // busy wait for the next wakeup interval to pass
    last_wakeup += wakeup_interval;
  }
  return(1);
};


int main (void) {

    if (!init()) {
      return(false);
    };

    if (!loop() ) {
      return(false);
    };

    return(true);
}
