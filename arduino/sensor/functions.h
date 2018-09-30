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
