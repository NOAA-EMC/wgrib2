/** @file
 * @brief Routines that convert lat/lon to i, j of a Gaussian grid. Grid must be global 
 * in wesn order.
 * 
 * Based on lat2ij.c
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 7/2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "grb2.h"
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

/**  Conversion factor for longitude. */
static double from_lon;

/**
 * Initialize the Gaussian grid parameters to convert lat/lon to i,j.
 * 
 * @param sec Pointer to the section of the grid descriptor.
 * @param nx Number of grid points in the x-direction.
 * @param ny Number of grid points in the y-direction.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 7/2021
 */
int gaussian_init(unsigned char **sec, unsigned int nx, unsigned int ny) {

    if (code_table_3_1(sec) != 40) fatal_error("gaussian_init: not gaussian grid","");
    if (nx < 1 || ny < 1) fatal_error("gaussian_init: program error nx, ny","");
    if (lat == NULL || lon == NULL) fatal_error("gaussian_init: lat/lon undefined","");
    if (output_order != wesn) fatal_error("gaussian_init: order must be we:sn","");
    if (ny != 2*GDS_Gaussian_nlat(sec[3])) fatal_error("gaussian_init: regional Gaussian grid","");

    from_dlon = lon[1] - lon[0];
    from_lon = lon[0] - 0.5*from_dlon;
    from_nx = nx;
    from_ny = ny;

    return 0;
}

/* only for global gaussian grids, wesn order */

/**
 * Find the closest grid point (i,j) for a given latitude/longitude on a Gaussian grid.
 *
 * Only for global Gaussian grids in wesn order.
 * 
 * @param sec Pointer to the section of the grid descriptor.
 * @param plat Latitude of the point.
 * @param plon Longitude of the point.
 *
 * @return The index of the closest grid point, or -1 on error.
 */
long int gaussian_closest(unsigned char **sec, double plat, double plon) {

    double tmp, n_lat;
    long int ix;
    unsigned int iy;

    if (lat == NULL || lon == NULL) fatal_error("gaussian_closest: lat/lon undefined","");

    if (plon < from_lon) plon += 360.0;
    if (plon > from_lon + 360.0) plon -= 360.0;

    tmp = (plon - from_lon) / from_dlon;
    ix = floor(tmp);
    if (ix == from_nx && tmp <= from_nx+ERROR) ix--;
    if (ix < 0 || ix >= from_nx) return -1;

    if (plat < -90-ERROR || plat > 90 + ERROR) return -1;

    for (iy = 0; iy < from_ny; iy++) {
        /* not sure if n_lat = iy == from_ny-1 would short curcuit and avoid illegal mem access */
        if (iy == from_ny-1) {
            n_lat = 90 + ERROR;
        }
        else {
            n_lat =  (lat[iy*from_nx] + lat[iy*from_nx + from_nx]) * 0.5;
        }
        if (plat < n_lat) return (ix + iy*from_nx);
    }
    return -1;
}
