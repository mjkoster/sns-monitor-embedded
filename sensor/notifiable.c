//notifiable.c
bool notifiable() {

  time_t t; // when v was sampled
  time_t t_last; // last time this resource was reported
  double v; // sampled values
  char* vs;
  bool vb;
  double v_last; // last reported values
  char* vs_last;
  bool vb_last;
  time_t pmin; // min time between report notifications
  time_t pmax; // max time allowed without a notificaiton
  double lt; // limit for sample less than
  double gt; // limit for sample greater than
  double st; // limit of sample change (delta) since last report
  bool band; // only notify when gt <= sample <= lt

  #define BAND band
  #define SCALAR_TYPE ( NUMBER_TYPE == type )
  #define STRING_TYPE ( STRING_TYPE == type )
  #define BOOLEAN_TYPE ( BOOLEAN_TYPE == type )
  #define PMIN_EX ( t - t_last >= pmin )
  #define PMAX_EX ( t - t_last > pmax )
  #define LT_EX ( v < lt ^ v_last < lt )
  #define GT_EX ( v > gt ^ v_last > gt )
  #define ST_EX ( abs( v - v_last ) >= st )
  #define IN_BAND ( ( gt <= v && v <= lt ) || \
                  ( lt <= gt && gt <= v ) || \
                  ( v <= lt && lt <= gt ) )
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
