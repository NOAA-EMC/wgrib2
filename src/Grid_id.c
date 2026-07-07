/** @file
 * @brief Identify grid.
 * @author Public Domain: Wesley Ebisuzaki @date 6/2012
 */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"
#include "grid_id.h"

/*
 * HEADER:100:grid_id:inv:0:show values from grid_id
 */

/** Current output order type. */
extern enum output_order_type output_order;

/** Number of grid points in the x direction. */
extern int nx;

/** Number of grid points in the y direction. */
extern int ny;

/** Inventory number. */
extern int inv_no;

/**
 * Prints values from grid_id.
 * 
 * The option, -grid_id, is for the development of the Proj4 interface. This option may change 
 * or disappear in future versions of wgrib2. 
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 6/2012
 */
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

