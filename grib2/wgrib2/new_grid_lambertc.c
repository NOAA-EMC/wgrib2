#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "proj.h"
#include "fnlist.h"

/*
 * Public Domain 8/2013 Wesley Ebisuzaki
 *
 *  WRF specifies the lambert conformal grid with the lat-lon of the center point.
 *
 *  grib requires the lat-lon of the 1st grid point
 *
 *  This routine calculates the lat-lon of the first grid point given
 *  the lat-lon of the center of the grid.
 *
 *  This routine is used by New_grid.c
 *
 *  The code requires gctpc
 *
 */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef DEG_TO_RAD
#define RAD_TO_DEG  (180.0/M_PI)
#define DEG_TO_RAD  (M_PI/180.0)
#endif

int new_grid_lambertc(int nx, int ny, double center_lon, double center_lat,
    double true_lat1, double true_lat2, double stand_lon, double stand_lat,
    double r_maj, double r_min, double dx,  double dy,
    double *lon_0, double *lat_0) {

    double x_0, y_0, rlon, rlat;

    double false_east, false_north;
    long long_i;

    /* gctpc */

    true_lat1 *= DEG_TO_RAD;
    true_lat2 *= DEG_TO_RAD;
    stand_lon *= DEG_TO_RAD;
    stand_lat *= DEG_TO_RAD;
    center_lon *= DEG_TO_RAD;
    center_lat *= DEG_TO_RAD;

    false_east = false_north = 0.0;
    long_i = lamccforint(r_maj,r_min,true_lat1, true_lat2, stand_lon, stand_lat, false_east,false_north);
    if (long_i) fatal_error_i("new_grid_lambertc lamccforint: returns %ld", long_i);

    /* convert center lat-lon to (x,y) */
    long_i = lamccfor(center_lon, center_lat, &x_0, &y_0);
    if (long_i) fatal_error_i("new_grid_lambertc lamccfor: returns %ld", long_i);

    /* find bottom left point (x,y) */
    x_0 = x_0 - dx * ( (nx - 1)*0.5 );
    y_0 = y_0 - dy * ( (ny - 1)*0.5 );

    /* convert (x,y) -> (lon,lat) */
    long_i = lamccinvint(r_maj,r_min,true_lat1,true_lat2,stand_lon,stand_lat,false_east,false_north);
    if (long_i) fatal_error_i("new_grid_lambertc lamccinvint: returns %ld", long_i);
    long_i = lamccinv(x_0, y_0, &rlon, &rlat);
    if (long_i) fatal_error_i("new_grid_lambertc lamccinv: returns %ld", long_i);
    rlon *= RAD_TO_DEG;
    rlat *= RAD_TO_DEG;
    if (rlon < 0.0) rlon += 360.0;
    if (rlon >= 360.0) rlon -= 360.0;
    *lon_0 = rlon;
    *lat_0 = rlat;

    return 0;
}
