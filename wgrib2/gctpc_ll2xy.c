#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "proj.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* gctpc_aux.c  interface routines to the gctpc library
   2/2012 Public Domain Wesley Ebisuzaki

  gctpc_get_latlon: fill grid with lat/lon values

  mercator
  polar stereographic
  lambert conformal
  Albers equal area
  10/2015 lambert azimuthal equal area
*/

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

/*
 * given vectors of lon-lat
 * return i-j vectors
 *
 * step 1: find (x,y) of 1st point
 * step 2: find (x,y) of input point
 *
 * input:
 *        **sec = grib sections of grid
 *        **grid_lon = longitues of grid
 *        **grid_lat = latitudes of grid
 *        n = number of point to convert
 *        *lon = longitudes of points to convert to (i,j) 
 *        *lat = longitudes of points to convert to (i,j) 
 *
 * output:
 *        x[n]    coordinates on grid
 *        y[n]
 *
 * assumption: grid_lon, grid_lat is in wesn order
 *
 * to use:
 *
 * setup
 *      int gctpc_ll2xy_init(unsigned char **sec, double *grid_lon, double *grid_lat);
 *
 * get (x,y)
 *      int gctpc_ll2xy(int n, double *lon, double *lat, double *x, double *y);
 * get nearest neighbor
 *      int gctpc_ll2i(int n, double *lon, double *lat, unsigned int *i);
 *
 */

static long int (*forward_fn)();
static double dx, dy, inv_dx, inv_dy, x_0, y_0, x00, xN;
static unsigned int gdt;
static int nx, ny;

int gctpc_ll2xy_init(unsigned char **sec, double *grid_lon, double *grid_lat) {

    unsigned char *gds;

    double r_maj;                           /* major axis                   */
    double r_min;                           /* minor axis                   */
    double lat1;                            /* first standard parallel      */
    double lat2;                            /* second standard parallel     */
    double c_lon;                           /* center longitude             */
    double c_lat;                           /* center latitude              */
    double false_east;                      /* x offset in meters           */
    double false_north;

    int nres, nscan;
    unsigned int npnts;
    long long_i;
    double rlon, rlat;

    gdt = code_table_3_1(sec);
    gds = sec[3];

    /* only process certain grids */
    forward_fn = NULL;

    if (grid_lon == NULL || grid_lat == NULL) return 1;
    if (gdt != 0 && !(gdt == 10 && GDS_Mercator_ori_angle(gds) == 0.0) && gdt != 20 && gdt != 30 && gdt != 140) return 1;
    get_nxny(sec, &nx, &ny, &npnts, &nres, &nscan);
    if (nx == -1 || ny == -1 || nx*ny != npnts)   return 1;

    /* only process certain grids */

    x_0 = y_0 = x00 = xN = inv_dx = inv_dy = 0.0;

    if (gdt == 0) {	/* lat-lon grid */
	dx = grid_lon[1] - grid_lon[0];
        dy = grid_lat[nx] - grid_lat[0];
        inv_dx = 1.0 / dx;
        inv_dy = 1.0 / dy;
        x_0 = grid_lon[0];
        x00 = grid_lon[0] - 0.5*dx;
        xN = grid_lon[nx-1] + 0.5*dx;
        y_0 = grid_lat[0];
        return 0;
    }
    else if (gdt == 10 && (GDS_Mercator_ori_angle(gds) == 0.0) ) {            // mercator no rotation
        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);

	dx = GDS_Mercator_dx(gds);
	dy = GDS_Mercator_dy(gds);
        inv_dx = 1.0 / dx;
        inv_dy = 1.0 / dy;

        /* central point */
        c_lon = 0.0;
        c_lat = GDS_Mercator_latD(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        long_i = merforint(r_maj,r_min,c_lon,c_lat,false_east,false_north);
	if (long_i) fatal_error_i("gctpc_ll2xy_init merforint: return %ld", long_i);

        rlat = GDS_Mercator_lat1(gds) * (M_PI/180.0);
        rlon = GDS_Mercator_lon1(gds) * (M_PI/180.0);

        long_i = merfor(rlon, rlat, &x_0, &y_0);

        x00 = x_0 - 0.5*dx;
        xN =  x_0 + (nx-0.5)*dx;

        forward_fn = &merfor;
    }
    else if (gdt == 20) {            // polar stereographic

        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);
        dy      = GDS_Polar_dy(gds);
        dx      = GDS_Polar_dx(gds);
        inv_dx = 1.0 / dx;
        inv_dy = 1.0 / dy;

        /* central point */
        c_lon = GDS_Polar_lov(gds) * (M_PI/180.0);
        c_lat = GDS_Polar_lad(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        long_i = psforint(r_maj,r_min,c_lon,c_lat,false_east,false_north);
	if (long_i) fatal_error_i("gctpc_ll2xy_init psforint: return %ld", long_i);

        rlon   = grid_lon[0] * (M_PI/180.0);
        rlat   = grid_lat[0] * (M_PI/180.0);

        long_i = psfor(rlon, rlat, &x_0, &y_0);
        x00 = x_0 - 0.5*dx;
        xN =  x_0 + (nx-0.5)*dx;
        forward_fn = &psfor;
    }

    else if (gdt == 30) {            // lambert conformal conic

        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);
        dx      = GDS_Lambert_dx(gds);
        dy      = GDS_Lambert_dy(gds);
        inv_dx = 1.0 / dx;
        inv_dy = 1.0 / dy;

        /* latitudes of tangent/intersection */
        lat1 = GDS_Lambert_Latin1(gds) * (M_PI/180.0);
        lat2 = GDS_Lambert_Latin2(gds) * (M_PI/180.0);

        /* central point */
        c_lon = GDS_Lambert_Lov(gds) * (M_PI/180.0);
        c_lat = GDS_Lambert_LatD(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        long_i = lamccforint(r_maj,r_min,lat1,lat2,c_lon,c_lat,false_east,false_north);
	if (long_i) fatal_error_i("gctpc_ll2xy_init lamccforint: return %ld", long_i);

        rlon   = grid_lon[0] * (M_PI/180.0);
        rlat   = grid_lat[0] * (M_PI/180.0);

        long_i = lamccfor(rlon, rlat, &x_0, &y_0);

        x00 = x_0 - 0.5*dx;
        xN =  x_0 + (nx-0.5)*dx;
        forward_fn = &lamccfor;
    }
    else if (gdt == 140) {            // lambert azimuthal equal area

	/* get earth axis */
	axes_earth(sec, &r_maj, &r_min);
	r_maj = 0.5 * (r_maj + r_min);

        dx      = GDS_Lambert_Az_dx(gds);
        dy      = GDS_Lambert_Az_dy(gds);
        inv_dx = 1.0 / dx;
        inv_dy = 1.0 / dy;

        /* central point */
        c_lon = GDS_Lambert_Az_Cen_Lon(gds) * (M_PI/180.0);
        c_lat = GDS_Lambert_Az_Std_Par(gds) * (M_PI/180.0);

        false_east = false_north = 0.0;
        long_i = lamazforint(r_maj,c_lon,c_lat,false_east,false_north);
	if (long_i) fatal_error_i("gctpc_ll2xy_init lamazforint: return %ld", long_i);

        rlon   = grid_lon[0] * (M_PI/180.0);
        rlat   = grid_lat[0] * (M_PI/180.0);

        long_i = lamazfor(rlon, rlat, &x_0, &y_0);

        x00 = x_0 - 0.5*dx;
        xN =  x_0 + (nx-0.5)*dx;
        forward_fn = &lamazfor;
    }

    return forward_fn != NULL ? 0 : 1;
}


int gctpc_ll2xy(int n, double *lon, double *lat, double *x, double *y) {

    int i;
    double rlon, rlat;

    if (gdt == 0) {		// lat-lon
//   #pragma omp parallel for schedule(static) private(i,rlon,rlat)
        for (i = 0; i < n; i++) {
            rlon = lon[i];
            if (rlon > xN) rlon -= 360.0;
            if (rlon < x00) rlon += 360.0;
            rlat = lat[i];
            x[i] = (rlon - x_0) * inv_dx;
            y[i] = (rlat - y_0) * inv_dy;
        }
        return 0;
    }

    if (forward_fn == NULL) return 1;

//  #pragma omp parallel for private(i,rlon,rlat)
    for (i = 0; i < n; i++) {
        rlon = lon[i];
        if (rlon > xN) rlon -= 360.0;
        if (rlon < x00) rlon += 360.0;
        rlon *= (M_PI/180.0);
        rlat = lat[i] * (M_PI/180.0);

        forward_fn(rlon, rlat, x+i, y+i);
        x[i] = (x[i] - x_0)*inv_dx;
        y[i] = (y[i] - y_0)*inv_dy;
    }
    return 0;
}

/* iptr[] == 0 for out of bounds */

int gctpc_ll2i(int n, double *lon, double *lat, unsigned int *ipnt) {
    int i;
    unsigned int ix, iy;
    double rlon, rlat, x, y;

    if (gdt == 0) {             // lat-lon
//  #pragma omp parallel for schedule(static) private(i,rlon,rlat,ix,iy, x, y)
        for (i = 0; i < n; i++) {
            rlon = lon[i];
            if (rlon > xN) rlon -= 360.0;
            if (rlon < x00) rlon += 360.0;
            rlat = lat[i];
            ix = x = floor((rlon - x_0) * inv_dx + 0.5);
            iy = y = floor((rlat - y_0) * inv_dy + 0.5);
	    if (x < 0 || x >= nx || y < 0 || y >= ny) {
		ipnt[i] = 0;
	    }
	    else {
		ipnt[i] = ix + nx*iy + 1;
	    }
        }
        return 0;
    }
    if (forward_fn == NULL) return 1;

//   #pragma omp parallel for schedule(static) private(i,rlon,rlat,ix,iy, x, y)
    for (i = 0; i < n; i++) {
        rlon = lon[i];
        if (rlon > xN) rlon -= 360.0;
        if (rlon < x00) rlon += 360.0;
        rlon *= (M_PI/180.0);
        rlat = lat[i] * (M_PI/180.0);

        forward_fn(rlon, rlat, &x, &y);
        ix = x = floor((x - x_0)*inv_dx + 0.5);
        iy = y = floor((y - y_0)*inv_dy + 0.5);
	if (x < 0 || x >= nx || y < 0 || y >= ny) {
	    ipnt[i] = 0;
	}
	else {
	    ipnt[i] = ix + nx*iy + 1;
	}
    }
    return 0;
}
