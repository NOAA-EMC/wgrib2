/* Public Domain 7/2021 Wesley Ebisuaki
 *
 * routines that convert lat/lon to i, j of a Gaussian grid
 * grid must be global in wesn order
 *
 * based on lat2ij.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#define ERROR 0.0001

extern double *lat, *lon;
extern enum output_order_type output_order;

static unsigned int from_nx, from_ny;
static double from_dlon, from_lon;

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
