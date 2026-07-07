/** @file
 * @brief Routines that convert lat-lon to i,j.
 * @author Public Domain: Wesley Ebisuzaki @date 7/2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "wgrib2.h"

/** Error tolerance for floating point comparisons. */
#define ERROR 0.0001

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Current output order type. */
extern enum output_order_type output_order;

/** Conversion factor for nx. */
static unsigned int from_nx;

/** Conversion factor for ny. */
static unsigned int from_ny;

/** Conversion factor for dlon. */
static double from_dlon;

/** Conversion factor for dlat. */
static double from_dlat;

/** Conversion factor for lon. */
static double from_lon;

/** Conversion factor for lat. */
static double from_lat;

/**
 * Initialize the grid parameters to convert lat/lon to i,j.
 *
 * @param sec Pointer to the section of the grid descriptor.
 * @param nx Number of grid points in the x-direction.
 * @param ny Number of grid points in the y-direction.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 7/2021
 */
int latlon_init(unsigned char **sec, unsigned int nx, unsigned int ny) {

    if (code_table_3_1(sec) != 0) fatal_error("latlon_init: not lat-lon grid","");
    if (nx < 1 || ny < 1) fatal_error("latlon_init: program error nx, ny","");
    if (lat == NULL || lon == NULL) fatal_error("latlon_init: lat/lon undefined","");
    if (output_order != wesn) fatal_error("latlon_init: order must be we:sn","");

    from_dlon = lon[1] - lon[0];
    from_dlat = lat[nx] - lat[0];
    from_lon = lon[0] - 0.5*from_dlon;
    from_lat = lat[0] - 0.5*from_dlat;
    from_nx = nx;
    from_ny = ny;
    return 0;
}

/**
 * Find the closest grid point to a given latitude/longitude.
 *
 * @param sec Pointer to the section of the grid descriptor.
 * @param plat Latitude of the point to find.
 * @param plon Longitude of the point to find.
 *
 * @return Index of the closest grid point, or -1 if not found.
 *
 * @author Wesley Ebisuzaki @date 7/2021
 */
long int latlon_closest(unsigned char **sec, double plat, double plon) {

    double tmp;
    long int ix, iy;

    if (lat == NULL || lon == NULL) fatal_error("latlon_closest: lat/lon undefined","");

    if (plon < from_lon) plon += 360.0;
    if (plon > from_lon + 360.0) plon -= 360.0;

    tmp = (plon - from_lon) / from_dlon;
    ix = floor(tmp);
    if (ix == from_nx && tmp <= from_nx+ERROR) ix--;

    tmp = (plat - from_lat) / from_dlat;
    iy = floor(tmp);
    if (iy == from_ny && tmp <= from_ny+ERROR) iy--;
    if (ix >= 0 && ix < from_nx && iy >= 0 && iy < from_ny) return (ix+iy*from_nx);
    else return -1;
}
