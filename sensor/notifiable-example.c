//notifiable.c
bool notifiable( Resource * r ) {

  bool scalar_type = ( num_type == r->type ); // number, string. or boolean
  bool string_type = ( str_type == r->type );
  bool boolean_type = ( bool_type == r->type );
  time_t t = r->last_sample_time; // when v was sampled
  time_t t_last = r->last_rep_time; // last time this resource was reported
  double v = r->v; // sampled value
  char* vs = r->vs;
  bool vb = r->vb;
  double v_last = r->last_rep_v; // last reported value
  char* vs_last = r->last_rep_vs;
  bool vb_last = r->last_rep_vb;
  time_t pmin = r->pmin; // min time between report notifications
  time_t pmax = r->pmax; // max time allowed without a notificaiton
  double lt = r->lt; // limit for sample less than
  double gt = r->gt; // limit for sample greater than
  double st = r->st; // limit of sample change (delta) since last report
  bool band = r->band; // only notify when gt <= sample <= lt

  bool pmin_ex = ( t - t_last >= pmin );
  bool pmax_ex = ( t - t_last > pmax );
  bool lt_ex = ( v < lt ^ v_last < lt );
  bool gt_ex = ( v > gt ^ v_last > gt );
  bool st_ex = ( abs( v - v_last ) >= st );
  bool in_band = ( ( gt <= v && v <= lt ) ||
                ( v >= gt && gt >= lt ) ||
                ( v <= lt && lt <= gt ) );

  return (
    pmin_ex &&
    ( scalar_type ?
      ( ( !band && ( gt_ex || lt_ex || st_ex || pmax_ex ) ) ||
        ( band && in_band && ( st_ex || pmax_ex) ) )
    : string_type ?
      ( ( vs != vs_last) || pmax_ex )
    : boolean_type ?
      ( ( vb != vb_last) || pmax_ex )
    : false )
  );
}
