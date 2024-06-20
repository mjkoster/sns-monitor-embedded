/* 
 * resource template
 */
#include "Arduino.h"

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

    void (*init_function)(); // pointer to function to initialize the gpio driver (if needed)
    float (*sample_function)(); // pointer to a function to update resource->v 
    unsigned int (*int_sample_function)(); // pointer to a function to update resource->vs, vb
    unsigned int (*onupdate)(); // pointer to a function call on updates
    unsigned int (*onreport)(); // pointer to a function call on reports
};

/*
 * resource instances
 */

// am2315 temperature
Resource R3303_1_5700 = {
  3303,  // objid
  1,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  500, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  30000, // pmax
  0, // lt
  500, // gt
  1, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  aht20_setup, // *init_function
  aht20_get_temperature, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// am2315 humidity
Resource R3304_1_5700 = {
  3304,  // objid
  1,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  500, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  30000, // pmax
  0, // lt
  500, // gt
  1, // st
  false, // band
  4000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  aht20_get_humidity, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};
/*
// si1145 visible light
Resource R3301_0_5700 = {
  3301,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  1000, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  30000, // pmax
  0, // lt
  500, // gt
  10, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  si1145_setup, // *init_function
  si1145_get_visible, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// si1145 IR light
Resource R3301_1_5700 = {
  3301,  // objid
  1,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  1000, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  30000, // pmax
  0, // lt
  500, // gt
  10, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  si1145_get_ir, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// si1145 UV factor
Resource R3301_2_5700 = {
  3301,  // objid
  2,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  1000, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  30000, // pmax
  0, // lt
  500, // gt
  1, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  si1145_get_uv, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// sound level
Resource R3324_0_5700 = {
  3324,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  160, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  100, // pmin
  30000, // pmax
  0, // lt
  160, // gt
  10, // st
  false, // band
  100, // sample_interval
  0, // last_sample_time
  ain_type, // type
  A2, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  NULL, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// sound detect
Resource R3200_0_5500 = {
  3200,  // objid
  0,  // objinst
  5500, // resid
  bool_type, // type
  0, // v
  0, //vmin
  0, //vmax
  "", // vs
  0, // max_strlen
  false, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  100, // pmin
  30000, // pmax
  0, // lt
  0, // gt
  0, // st
  false, // band
  100, // sample_interval
  0, // last_sample_time
  din_type, // type
  A3, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  NULL, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};
*/
// depth mm (rain total)
Resource R3319_0_5700 = {
  3319,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  1000, //vmax
  "", // vs
  0, // max_strlen
  false, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  10000, // pmin
  10000, // pmax
  0, // lt
  1000, // gt
  1, // st
  false, // band
  10000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  weather_rack_setup, // *init_function
  weather_rack_get_rain_total, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// direction (wind direction)
Resource R3332_0_5700 = {
  3332,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  359, //vmax
  "", // vs
  0, // max_strlen
  false, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  30000, // pmax
  0, // lt
  360, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  weather_rack_get_wind_direction, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

//rate m/s (wind speed)
Resource R3346_0_5700 = {
  3346,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  100, //vmax
  "", // vs
  0, // max_strlen
  false, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  500, // pmin
  30000, // pmax
  0, // lt
  100, // gt
  1, // st
  false, // band
  500, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  weather_rack_get_wind_speed, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

//rate m/s (wind gust)
Resource R3346_1_5700 = {
  3346,  // objid
  1,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  100, //vmax
  "", // vs
  0, // max_strlen
  false, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  5000, // pmin
  5000, // pmax
  0, // lt
  100, // gt
  1, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  weather_rack_get_wind_gust, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

Resource * resource_list[] = {
  &R3303_1_5700,
  &R3304_1_5700,
  //&R3301_0_5700,
  //&R3301_1_5700,
  //&R3301_2_5700,
  //&R3324_0_5700,
  //&R3200_0_5500,
  &R3319_0_5700,
  &R3332_0_5700,
  &R3346_0_5700, 
  &R3346_1_5700  
};
