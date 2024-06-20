/* 
 * resource template
 */
typedef enum valuetype_t { num_type, str_type, bool_type };
typedef enum gpiotype_t { no_gpio, ain_type, pwm_type, din_type, dout_type, ser_type, func_type };

typedef union value_t {
  float v;
  char* s;
  unsigned char vb;
};

typedef struct Resource {
  // LWM2M Object ID, Instance, and Resource ID
  short objid; 
  short objinst;
  short resid;

  valuetype_t type;

  value_t value;

  Resource_Extension *extension;
};

typedef struct Resource_Extension {
  float vmin; // Min and Max range values for conditionals and limiting
  float vmax;
  unsigned int max_strlen; // to limit string length
  unsigned char invert; // boolean for inverting the digital sense

  // value that was transmitted in the last (most recent past) report
  value_t last_rep_value;

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

  gpiotype_t gpio;
  unsigned short gpio_pin;
  // for scaling analog pins to vmin/vmax
  int vmin_counts; // a/d counts corresponding to vmin
  int vmax_counts; // a/d counts corresponding to vmax

  void (*init_function)(); // pointer to function to initialize the gpio driver (if needed)
  float (*sample_function)(); // pointer to a function to update resource->v 
  unsigned int (*int_sample_function)(); // pointer to a function to update resource->vs, vb
  unsigned int (*onupdate)(); // pointer to a function call on updates
  unsigned int (*onreport)( ); // pointer to a function call on reports
};

typedef struct Extended_Resource {
  // LWM2M Object ID, Instance, and Resource ID
  short objid; 
  short objinst;
  short resid;

  valuetype_t type;

  value_t value;

  float vmin; // Min and Max range values for conditionals and limiting
  float vmax;
  unsigned int max_strlen; // to limit string length
  unsigned char invert; // boolean for inverting the digital sense

  // value that was transmitted in the last (most recent past) report
  value_t last_rep_value;

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

  gpiotype_t gpio;
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
Extended_Resource R3303_0_5700 = {
  3303,  // objid
  0,  // objinst
  5700, // resid
  num_type, // type
  NULL, // value
  0, //vmin
  500, //vmax
  0, // max_strlen
  0, // invert
  0, // last_rep_value
  0, // last_rep_time
  1000, // pmin
  5000, // pmax
  0, // lt
  1000, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  device_setup, // *init_function
  device_get_value, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

Resource R3303_1_5700 = {
  3303,  // objid
  1,  // objinst
  5700, // resid
  num_type, // type
  NULL, // value
  &E3303_1_5700

};
Resource_Extension E3303_1_5700 = {
  0, //vmin
  500, //vmax
  0, // max_strlen
  0, // invert
  0, // last_rep_value
  0, // last_rep_time
  1000, // pmin
  5000, // pmax
  0, // lt
  1000, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  device_setup, // *init_function
  device_get_value, // *sample_function
  NULL, // *sample function for boolean or string
  NULL, // *onupdate
  NULL // *onreport
};

Resource * resource_list[] = {
  &R3303_0_5700
  &R3303_1_5700
};
