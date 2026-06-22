/** @file
 * @brief WRF specifies the lambert conformal grid with the lat-lon of the center point.
 * 
 * GRIB requires the lat-lon of the 1st grid point.
 * 
 * This routine calculates the lat-lon of the first grid point given the lat-lon of the center 
 * of the grid.
 * 
 * This routine is used by New_grid.c and requires gctpc.
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 8/2013
 */
#include <stdio.h>
#include <stdlib.h>

#include "wmath.h"
#include "grb2.h"
#include "wgrib2.h"
#include "cproj.h"

/**
 * Calculate the lat-lon of the first grid point given the lat-lon of the center of the grid.
 * 
 * @param nx Number of points in the x direction.
 * @param ny Number of points in the y direction.
 * @param center_lon Longitude of the center point.
 * @param center_lat Latitude of the center point.
 * @param true_lat1 Latitude of the first true scale.
 * @param true_lat2 Latitude of the second true scale.
 * @param stand_lon Longitude of the standard parallel.
 * @param stand_lat Latitude of the standard parallel.
 * @param r_maj Major radius of the earth.
 * @param r_min Minor radius of the earth.
 * @param dx Grid spacing in the x direction.
 * @param dy Grid spacing in the y direction.
 * @param lon_0 Pointer to store the longitude of the first grid point.
 * @param lat_0 Pointer to store the latitude of the first grid point.
 * 
 * @return 0 on success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 8/2013
 */
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
