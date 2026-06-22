/** @file
 * @brief GRIB2 specific  routine to identify a grid.
 * @author Public Domain: Wesley Ebisuzaki @date 6/2012
 */

#include <stdio.h>
#include "wgrib2.h"
#include "grid_id.h"

/*
  r_major = earth shape: major axis in m
  r_minor = earth shape: manor axis in m

  projection_type:  lambert_conic, etc

  n = number of grid points
  nx = number of x
  ny = number of y
        use -1 for variable or not rectangular grid
*/

/**
 * Identify the grid type and parameters from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * @param r_major Pointer to the major axis of the Earth.
 * @param r_minor Pointer to the minor axis of the Earth.
 * @param proj_id Pointer to the projection type.
 * @param proj_args Pointer to the projection arguments.
 * @param n_proj_args Number of projection arguments.
 * @param grid_defn Pointer to the grid definition structure.
 *
 * @return 0 for success, error code otherwise.
 *
 * @author Wesley Ebisuzaki @date 6/2012
 */
int grid_id(GRID_ID_ARGS) {

    int gdt, res, scan, nx, ny;
    unsigned int npnts;

    if (axes_earth(sec, r_major, r_minor, NULL)) fatal_error("grid_id: axes undefined","");

    gdt = code_table_3_1(sec);
    switch (gdt) {
        case 0: *proj_id = p_latlon; break;
        case 1: *proj_id = p_rotated_latlon; break;
        case 10: *proj_id = p_mercator; break;
        case 20: *proj_id = p_polar_stereographic; break;
        case 30: *proj_id = p_lambert_conic; break;
        default: *proj_id = p_unknown; break;
    }

    /* get nx, ny */
    get_nxny(sec, &nx, &ny, &npnts, &res, &scan);

    grid_defn->nx = nx;
    grid_defn->ny = ny;
    grid_defn->n = npnts;

    /* get xy_list */
    grid_defn->valid_xy_list = 0;

    return 0;
}
