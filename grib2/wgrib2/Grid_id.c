#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "grid_id.h"

/* Grib_id
   6/2012 Public Domain Wesley Ebisuzaki

   identify grid
  
*/


/*
 * HEADER:100:grid_id:inv:0:show values from grid_id
 */

extern enum output_order_type output_order;
extern int nx, ny;
extern int inv_no;

int f_grid_id(ARG0) {
    int i;
    double r_major;                           /* major axis                   */
    double r_minor;                           /* minor axis                   */
    enum projection_type proj_id;
    double proj_args[N_proj_args];
    struct grid_type grid_defn;

    if (mode < 0) return 0;
    i = grid_id(sec, &r_major,&r_minor, &proj_id, proj_args, N_proj_args, &grid_defn);

    sprintf(inv_out,"grid_id err=%d r_major=%.1lf m r_minor=%.1lf m proj_id=%d n=%d nx=%d ny=%d", i, r_major, r_minor, 
          (int) proj_id,grid_defn.n,grid_defn.nx,grid_defn.ny);
    return 0;
}

