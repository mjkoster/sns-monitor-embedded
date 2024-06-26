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

    void (*init_function)(); // pointer to function to initialize the gpio driver (if needed)
    float (*sample_function)(); // pointer to a function to update resource->v 
    unsigned int (*int_sample_function)(); // pointer to a function to update resource->vs, vb
    unsigned int (*onupdate)(); // pointer to a function call on updates
    unsigned int (*onreport)(); // pointer to a function call on reports
};

/*
 * resource instances
 */

// bme680 temperature
Resource R3303_0_5700 = {
  3303,  // objid
  0,  // objinst
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
  60000, // pmax
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
  bme680_setup, // *init_function
  bme680_get_temperature, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// bme680 humidity
Resource R3304_0_5700 = {
  3304,  // objid
  0,  // objinst
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
  60000, // pmax
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
  bme680_get_humidity, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};


///*
// SGP30 COeq
Resource R3325_3_5700 = {
  3325,  // objid
  3,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  60000, //vmax
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
  60000, // gt
  50, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  sgp30_setup, // *init_function
  sgp30_get_coeq, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// SGP30 TVOC
Resource R3325_4_5700 = {
  3325,  // objid
  4,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  60000, //vmax
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
  60000, // gt
  20, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  sgp30_get_tvoc, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};
//*/ /*
// MiCS-6814 CO
Resource R3325_5_5700 = {
  3325,  // objid
  5,  // objinst
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
  1000, // gt
  20, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  mics6814_setup, // *init_function
  mics6814_get_co, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 N02
Resource R3325_6_5700 = {
  3325,  // objid
  6,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  10, //vmax
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
  10, // gt
  0.2, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_no2, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 C2H6OH
Resource R3325_7_5700 = {
  3325,  // objid
  7,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  0, //vmin
  10, //vmax
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
  10, // gt
  1, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_c2h5oh, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 H2
Resource R3325_8_5700 = {
  3325,  // objid
  8,  // objinst
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
  1000, // gt
  2, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_h2, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 NH3
Resource R3325_9_5700 = {
  3325,  // objid
  9,  // objinst
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
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_nh3, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 CH4
Resource R3325_10_5700 = {
  3325,  // objid
  10,  // objinst
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
  5000000, // gt
  100000, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_ch4, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 C3H8
Resource R3325_11_5700 = {
  3325,  // objid
  11,  // objinst
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
  1000, // gt
  300, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_c3h8, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-6814 C4H10
Resource R3325_12_5700 = {
  3325,  // objid
  12,  // objinst
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
  10000, // gt
  200, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  1023, // vmax_counts
  NULL, // *init_function
  mics6814_get_c4h10, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// MiCS-5524
Resource R3325_2_5700 = {
  3325,  // objid
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
  1000, // gt
  20, // st
  false, // band
  5000, // sample_interval
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

// pms5003 PM1.0
Resource R3325_15_5700 = {
  3325,  // objid
  15,  // objinst
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
  10, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  pms5003_setup, // *init_function
  pms5003_get_pm1_0, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// pms5003 PM2.5
Resource R3325_16_5700 = {
  3325,  // objid
  16,  // objinst
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
  10, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  pms5003_get_pm2_5, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// pms5003 PM10
Resource R3325_17_5700 = {
  3325,  // objid
  17,  // objinst
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
  10, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  pms5003_get_pm10_0, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// bme680 barometer
Resource R3315_0_5700 = {
  3315,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  0, // v
  900, //vmin
  1100, //vmax
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
  .5, // st
  false, // band
  5000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  bme680_get_barometer, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// motion sensor
Resource R3302_0_5500 = {
  3302,  // objid
  0,  // objinst
  5500, // resid
  bool_type, // type
  0, // v
  0, //vmin
  0, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  1000, // pmin
  30000, // pmax
  0, // lt
  0, // gt
  0, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  din_type, // type
  10, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  NULL, // *sample_function
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
  A0, // gpio_pin
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
  A1, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  NULL, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

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
  tsl2561_setup, // *init_function
  tsl2561_get_illuminance, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// led color
Resource R3311_0_5706 = {
  3311,  // objid
  0,  // objinst
  5706, // resid
  str_type, // type
  0, // v
  0, //vmin
  0, //vmax
  "000000", // vs
  6, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  1000, // pmin
  30000, // pmax
  0, // lt
  0, // gt
  0, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  no_gpio, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  rgb_led_setup, // *init_function
  NULL, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

// led bargraph 10 segment
Resource R3306_0_5851 = {
  3306,  // objid
  0,  // objinst
  5851, // resid
  num_type, // type
  0, // v
  0, //vmin
  10, //vmax
  "", // vs
  0, // max_strlen
  0, // vb
  0, // invert
  0, // last_rep_v
  "", // last_rep_vs
  0, // last_rep_vb
  0, // last_rep_time
  1000, // pmin
  30000, // pmax
  0, // lt
  10, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  no_gpio, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  bargraph_led_setup, // *init_function
  NULL, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

Resource * resource_list[] = {
  &R3303_0_5700,
  &R3304_0_5700,
//  &R3325_0_5700,
//  &R3325_1_5700,
  &R3325_2_5700,
  &R3325_3_5700,
  &R3325_4_5700,
  &R3325_5_5700,
  &R3325_6_5700,
  &R3325_7_5700,
  &R3325_8_5700,
  &R3325_9_5700,
  &R3325_10_5700,
  &R3325_11_5700,
  &R3325_12_5700,
//  &R3325_13_5700,
//  &R3325_14_5700,
  &R3325_15_5700,
  &R3325_16_5700,
  &R3325_17_5700,
  &R3315_0_5700,
  &R3302_0_5500,
  &R3324_0_5700,
  &R3200_0_5500,
  &R3301_0_5700,
  &R3311_0_5706,
  &R3306_0_5851
};
