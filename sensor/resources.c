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

unsigned int R3300_1_5700_sample () {
  if ( (int) time(NULL) & 1 ) {
    R3300_1_5700.vs = "cold";
  }
  else {
    R3300_1_5700.vs = "hot";
  }
  return(true);
}

void bind_functions () {
  R3300_1_5700.sample_function = R3300_1_5700_sample;
};
