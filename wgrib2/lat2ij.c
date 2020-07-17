/*
 * routines that convert lat-lon to ij
 *
 * I expect things to change
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* M_PI, M_PI_2, M_PI_4, and M_SQRT2 are not ANSI C but are commonly defined */
/* values from GNU C library version of math.h copyright Free Software Foundation, Inc. */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif
#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif
#define ERROR 0.0001

extern double *lat, *lon;
extern enum output_order_type output_order;

static unsigned int from_nx, from_ny;
static double from_dlon, from_dlat;
static double from_lon, from_lat;

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


