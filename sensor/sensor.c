#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define true 1
#define false 0

typedef enum value_t { num_type, str_type, bool_type } value_t;
typedef enum gpio_t { no_gpio, ain_type, pwm_type, din_type, dout_type, ser_type } gpio_t;

typedef struct Resource {
    short objid; // LWM2M Object ID, Instance, and Resource ID
    short objinst;
    short resid;

    value_t type;
    float v;
    char* vs;
    unsigned char vb;

    float last_rep_v;
    char* last_rep_vs;
    unsigned char last_rep_vb;

    float vmin; // Min and Max range values for conditionals and limiting
    float vmax;

    unsigned int max_strlen; // to limit string length
    unsigned char invert; // boolean for inverting the digital sense

    time_t last_rep_time;
    time_t pmin; // Conditionals for report timing, see IETF CoRE Dynlink
    time_t pmax;

    float lt; // Conditionals for scalar values, see IETF CoRE Dynlink
    float gt;
    float st;

    gpio_t gpio;
    unsigned short gpio_pin;
    int vmin_counts; // for scaling analog pins to vmin/vmax
    int vmax_counts; // a/d counts corresponding to vmin and vmax

    time_t sample_interval; // for sample timing of gpio and function samplers
    time_t last_sample_time;

} Resource;

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
  .lt = 60,
  .gt = 40,
  .st = 1,
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
  .vs = "hot",
  .last_rep_vs = "hot",
  .last_rep_time = 0,
  .pmin = 5,
  .pmax = 10,
};

Resource R3300_2_5700 = {
  .objid = 3300,
  .objinst = 2,
  .resid = 5700,
  .type = bool_type,
  .invert = false,
  .vb = false,
  .last_rep_vb = false,
  .last_rep_time = 0,
  .pmin = 1,
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

unsigned int report_resource (Resource * resource) {

  printf ( "%i/%i/%i: ", resource->objid, resource->objinst, resource->resid );

  if ( num_type == resource->type )
    printf("%.2f\n", resource->v);
  else if ( str_type == resource->type )
    printf("%-10s\n", resource->vs);
  else if ( bool_type == resource->type )
    printf(resource->vb ? "true\n" : "false\n");

  return(true);

};

unsigned int process_gpio (Resource * resource) {

  if ( ain_type == resource->gpio ) {
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
  return(true); // if there was no GPIO update
};

unsigned int apply_conditionals (Resource * resource, time_t timestamp) {
  if (timestamp - resource->last_rep_time >= resource->pmin) {

    if (num_type == resource->type) {
      // notify on gt crossings
      if ( (resource->gt < resource->vmax) && // gt == vmax to disable gt limiting
      ( (resource->v > resource->gt && resource->last_rep_v <= resource->gt)
      || (resource->v <= resource->gt && resource->last_rep_v > resource->gt) ) )
      {
        resource->last_rep_v = resource->v;
        return(true);
      }
      // notify on lt crossing
      else if ( (resource->lt > resource->vmin) && // lt == vmin to disable lt limiting
      ( (resource->v < resource->lt && resource->last_rep_v >= resource->lt)
      || (resource->v >= resource->lt && resource->last_rep_v < resource->lt) ) )
      {
        resource->last_rep_v = resource->v;
        return(true);
      }

      // notify if in band and if step exceeded
      else if (
        ( (resource->v >= resource->gt) && (resource->v <= resource->lt) ) // in band
        && ( resource->v - resource->last_rep_v >= resource->st || resource->last_rep_v - resource->v >= resource->st )
      ) // note step = 0 disables change filtering and notifies every time
      {
        resource->last_rep_v = resource->v;
        return(true);
      }

      // notify if in band and pmax is exceeded
      else if ( ( (resource->v >= resource->gt) && (resource->v <= resource->lt) )
      && (timestamp - resource->last_rep_time >= resource->pmax) )
      {
        resource->last_rep_v = resource->v;
        return(true);
      }

      // else - no numeric value condition to notify this time
      else
        return(false);
    }
    else { // boolean or string type
      if ( resource->vb != resource->last_rep_vb ) {
        resource->last_rep_vb = resource->vb;
        return(true);
      }
      else if ( resource->vs != resource->last_rep_vs ) {
        resource->last_rep_vs = resource->vs;
        return(true);
      }
      else if (timestamp - resource->last_rep_time >= resource->pmax) {
        return(true); // no value change
      }
    }
  }

  // pmin not yet satisfied
  return(false);

};

unsigned int process_resource (Resource * resource, time_t timestamp) {
  if (timestamp - resource->last_sample_time > resource->sample_interval) {
    if (process_gpio(resource)) {
      if (apply_conditionals(resource, timestamp)) {
        resource->last_rep_time = timestamp;
        report_resource(resource);
      }
    }
  }
  return(true);
};

unsigned int init_resource (Resource * resource) {

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
  for ( int i = 0; i < ( sizeof(resource_list) / sizeof(resource_list[0]) ); i++) {
    init_resource (resource_list[i]);
  };
  return(1);
};

unsigned int loop (void) {
  time_t timestamp = time(NULL);
  time_t wakeup_interval = 1;
  time_t last_timestamp = timestamp;

  while (true) {

    for ( int i = 0; i < ( sizeof(resource_list) / sizeof(resource_list[0]) ); i++) {
      process_resource (resource_list[i], timestamp);
    };

    last_timestamp = timestamp;

    while (timestamp - last_timestamp < wakeup_interval) {
      timestamp = time(NULL);
    }; // busy wait for the next wakeup interval to pass

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
