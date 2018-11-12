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

Resource R3325_15_5700 = {
  3325,  // objid
  15,  // objinst
  5700, // resid
  num_type, // type
  50, // v
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
  1000, // pmin
  5000, // pmax
  0, // lt
  500, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  setup_pms5003, // *init_function
  pms5003_get_pm1_0, // *sample_function
  NULL, // *onupdate
  NULL // *onreport
};

Resource R3325_16_5700 = {
  3325,  // objid
  16,  // objinst
  5700, // resid
  num_type, // type
  50, // v
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
  1000, // pmin
  5000, // pmax
  0, // lt
  500, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  pms5003_get_pm2_5, // *sample_function
  NULL, // *onupdate
  NULL // *onreport
};

Resource R3325_17_5700 = {
  3325,  // objid
  17,  // objinst
  5700, // resid
  num_type, // type
  50, // v
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
  1000, // pmin
  5000, // pmax
  0, // lt
  500, // gt
  1, // st
  false, // band
  1000, // sample_interval
  0, // last_sample_time
  func_type, // type
  0, // gpio_pin
  0, // vmin_counts
  0, // vmax_counts
  NULL, // *init_function
  pms5003_get_pm10_0, // *sample_function
  NULL, // *onupdate
  NULL // *onreport
};


Resource * resource_list[] = {
  &R3325_15_5700,
  &R3325_16_5700,
  &R3325_17_5700
};
