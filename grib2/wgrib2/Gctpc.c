#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proj.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Gctpc.c  interface routines to the gctpc library
   2/2012 Public Domain Wesley Ebisuzaki

  gctpc_get_latlon: fill grid with lat/lon values

  mercator 10
  polar stereographic 20
  lambert conformal 30
  Albers equal area 31

  5/2014 added staggering
  10/2015 added Lambert Azimuthal Equal Area Projection
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

extern int use_gctpc;
extern int latlon;
extern double *lon, *lat;
extern enum output_order_type output_order;


/*
 * HEADER:100:gctpc:misc:1: X=0,1 use gctpc library  (default=1)
 */

int f_gctpc(ARG1) {
   use_gctpc = (strcmp(arg1,"1") == 0);
   return 0;
}


/* get lat-lon for grid
 *
 * step 1: initialize to center point
 * step 2: find (x,y) of lon1/lat1 (1st grid point)  (x0,y0)
 * step 3  find (x,y) of grid
 * step 4  find lat-lon of (x,y)
 */

static int warning_LamAZ = 0;

int gctpc_get_latlon(unsigned char **sec, double **lon, double **lat) {

    int gdt;
    unsigned char *gds;

    double r_maj;                           /* major axis                   */
    double r_min;                           /* minor axis                   */
    double lat1;                            /* first standard parallel      */
    double lat2;                            /* second standard parallel     */
    double c_lon;                           /* center longitude             */
    double c_lat;                           /* center latitude              */
    double false_east;                      /* x offset in meters           */
    double false_north;
    double dx, dy;
    double x0, y0;
    long int (*inv_fn)();
    double *llat, *llon, rlon, rlat;

    int nnx, nny, nres, nscan;
    unsigned int i, nnpnts;
    long g_error;

    gdt = code_table_3_1(sec);
    gds = sec[3];

    /* only process certain grids */

    if (gdt != 10 && gdt != 20 && gdt != 30 && gdt != 31 && gdt != 140) return 1;
    get_nxny(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

    /* potentially staggered */
// 8/2014    if (nnx < 1 || nny < 1) return 1;

    llat = *lat;
    llon = *lon;

    if (llat != NULL) {
	free(llat);
	free(llon);
        *lat = *lon = llat = llon = NULL;
    }

    inv_fn = NULL;
    dx = dy = 0.0;

    if (gdt == 10) {            // mercator

       /* get earth axis */
       axes_earth(sec, &r_maj, &r_min);
       dy      = GDS_Mercator_dy(gds);
       dx      = GDS_Mercator_dx(gds);

       /* central point */
       c_lon = GDS_Mercator_ori_angle(gds) * (M_PI/180.0);
       c_lat = GDS_Mercator_latD(gds) * (M_PI/180.0);

       /* find the eastling and northing of of the 1st grid point */

       false_east = false_north = 0.0;
       g_error = merforint(r_maj,r_min,c_lon,c_lat,false_east,false_north);
       if (g_error) fatal_error_i("merforint %d", g_error);

       rlon   = GDS_Mercator_lon1(gds) * (M_PI/180.0);
       rlat   = GDS_Mercator_lat1(gds) * (M_PI/180.0);

       g_error = merfor(rlon, rlat, &x0, &y0);
       if (g_error) fatal_error_i("merfor %d", g_error);

       /* initialize for 1st grid point */
       x0 = -x0;
       y0 = -y0;
       g_error = merinvint(r_maj,r_min,c_lon,c_lat,x0,y0);
       if (g_error) fatal_error_i("merinvint %d", g_error);
       inv_fn = &merinv;
    }

    else if (gdt == 20) {            // polar stereographic

       /* get earth axis */
       axes_earth(sec, &r_maj, &r_min);
       dy      = GDS_Polar_dy(gds);
       dx      = GDS_Polar_dx(gds);

       /* central point */
       c_lon = GDS_Polar_lov(gds) * (M_PI/180.0);
       c_lat = GDS_Polar_lad(gds) * (M_PI/180.0);

       /* find the eastling and northing of of the 1st grid point */

       false_east = false_north = 0.0;
       g_error = psforint(r_maj,r_min,c_lon,c_lat,false_east,false_north);
       if (g_error) fatal_error_i("psforint %d", g_error);

       rlon   = GDS_Polar_lon1(gds) * (M_PI/180.0);
       rlat   = GDS_Polar_lat1(gds) * (M_PI/180.0);

       g_error = psfor(rlon, rlat, &x0, &y0);
       if (g_error) fatal_error_i("psfor %d", g_error);

       /* initialize for 1st grid point */
       x0 = -x0;
       y0 = -y0;
       g_error = psinvint(r_maj,r_min,c_lon,c_lat,x0,y0);
       if (g_error) fatal_error_i("psinvint %d", g_error);
       inv_fn = &psinv;
    }

    else if (gdt == 30) {            // lambert conformal conic

       /* get earth axis */
       axes_earth(sec, &r_maj, &r_min);
       dy      = GDS_Lambert_dy(gds);
       dx      = GDS_Lambert_dx(gds);
//printf(">>> gctpc dx %lf, dy %lf\n", dx, dy);
       /* latitudes of tangent/intersection */
       lat1 = GDS_Lambert_Latin1(gds) * (M_PI/180.0);
       lat2 = GDS_Lambert_Latin2(gds) * (M_PI/180.0);

       /* central point */
       c_lon = GDS_Lambert_Lov(gds) * (M_PI/180.0);
       c_lat = GDS_Lambert_LatD(gds) * (M_PI/180.0);

       /* find the eastling and northing of of the 1st grid point */

       false_east = false_north = 0.0;
       g_error = lamccforint(r_maj,r_min,lat1,lat2,c_lon,c_lat,false_east,false_north);
       if (g_error) fatal_error_i("lamccforint %d", g_error);

       rlon   = GDS_Lambert_Lo1(gds) * (M_PI/180.0);
       rlat   = GDS_Lambert_La1(gds) * (M_PI/180.0);

       g_error = lamccfor(rlon, rlat, &x0, &y0);
       if (g_error) fatal_error_i("lamccfor %d", g_error);

       /* initialize for 1st grid point */
       x0 = -x0;
       y0 = -y0;
       g_error = lamccinvint(r_maj,r_min,lat1,lat2,c_lon,c_lat,x0,y0);
       if (g_error) fatal_error_i("lamccinvint %d", g_error);
       inv_fn = &lamccinv;
    }
    else if (gdt == 31) {			// albers equal area
       /* get earth axis */
       axes_earth(sec, &r_maj, &r_min);
       dy      = GDS_Albers_dy(gds);
       dx      = GDS_Albers_dx(gds);

       /* latitudes of tangent/intersection */
       lat1 = GDS_Albers_Latin1(gds) * (M_PI/180.0);
       lat2 = GDS_Albers_Latin2(gds) * (M_PI/180.0);

       /* central point */
       c_lon = GDS_Albers_Lov(gds) * (M_PI/180.0);
       c_lat = GDS_Albers_LatD(gds) * (M_PI/180.0);

       /* find the eastling and northing of of the 1st grid point */

       false_east = false_north = 0.0;
       g_error = alberforint(r_maj,r_min,lat1,lat2,c_lon,c_lat,false_east,false_north);
       if (g_error) fatal_error_i("alberforint %d", g_error);

       rlon   = GDS_Albers_Lo1(gds) * (M_PI/180.0);
       rlat   = GDS_Albers_La1(gds) * (M_PI/180.0);

       g_error = alberfor(rlon, rlat, &x0, &y0);
       if (g_error) fatal_error_i("alberfor %d", g_error);

       /* initialize for 1st grid point */
       x0 = -x0;
       y0 = -y0;
       g_error = alberinvint(r_maj,r_min,lat1,lat2,c_lon,c_lat,x0,y0);
       if (g_error) fatal_error_i("alberinvint %d", g_error);
       inv_fn = &alberinv;
    }
    else if (gdt == 140) {            // lambert azimuthal equal area

       /* get earth axis */
       axes_earth(sec, &r_maj, &r_min);
       if (r_maj != r_min && warning_LamAZ< 3) {
	    warning_LamAZ++;
	    fprintf(stderr,"**WARNING gctpc only does spherical Lambert Azimuthal Equal Area.\n"
		"**WARNING wgrib2 with Proj4 does aspherical earth:  -proj4 1\n");
       }
       r_maj = (r_maj + r_min)/2.0;		/* only for spherical earth */

       dy      = GDS_Lambert_Az_dy(gds);
       dx      = GDS_Lambert_Az_dx(gds);

       /* central point */
       c_lon = GDS_Lambert_Az_Cen_Lon(gds) * (M_PI/180.0);
       c_lat = GDS_Lambert_Az_Std_Par(gds) * (M_PI/180.0);

       /* find the eastling and northing of of the 1st grid point */

       false_east = false_north = 0.0;
       g_error = lamazforint(r_maj,c_lon,c_lat,false_east,false_north);
       if (g_error) fatal_error_i("lamazforint %d", g_error);

       rlon  = GDS_Lambert_Az_Lo1(gds) * (M_PI/180.0);
       rlat  = GDS_Lambert_Az_La1(gds) * (M_PI/180.0);
       g_error = lamazfor(rlon, rlat, &x0, &y0);
       if (g_error) fatal_error_i("lamazfor %d", g_error);

       /* initialize for 1st grid point */
       x0 = -x0;
       y0 = -y0;
       g_error = lamazinvint(r_maj,c_lon,c_lat,x0,y0);
       if (g_error) fatal_error_i("lamazinvint %d", g_error);
       inv_fn = &lamazinv;
    }

    if (inv_fn == NULL)  return 1;

    if ((*lat = llat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("gctpc_get_latlon memory allocation failed","");
    }
    if ((*lon = llon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("gctpc_get_latlon memory allocation failed","");
    }

    /* put x[] and y[] values in lon and lat */
    if (stagger(sec, nnpnts, llon, llat)) fatal_error("gctpc: stagger problem","");

//    printf(">> stagger gctpc x00 %lf y00 %lf\n",llon[0], llat[0]);
#pragma omp parallel for private(i)
    for (i = 0; i < nnpnts; i++) {
        inv_fn(llon[i]*dx, llat[i]*dy, llon+i, llat+i);
	llat[i] *= (180.0 / M_PI);
	llon[i] *= (180.0 / M_PI);
	if (llon[i] < 0.0) llon[i] += 360.0;
    }
    return 0;
}


/*
 * HEADER:100:ll2ij:inv:2:x=lon y=lat, converts lon-lat to (i,j) using gctpc
 */
int f_ll2ij(ARG2) {

    double x[1], y[1], to_lat[1], to_lon[1];
    int i;

    if (mode == -1) {
	latlon = 1;
    }
    if (mode >= 0) {
	if (output_order != wesn) return 1;
	to_lon[0] = atof(arg1);
	to_lat[0] = atof(arg2);
        i = gctpc_ll2xy_init(sec, lon, lat);
	if (i == 0)  {
            i = gctpc_ll2xy(1, to_lon, to_lat, x , y);
	    sprintf(inv_out,"%lf %lf -> (%lf,%lf)",to_lon[0], to_lat[0], x[0]+1.0, y[0]+1.0);
	}
    }
    return 0;
}

/*
 * HEADER:100:ll2i:inv:2:x=lon y=lat, converts to (i), 1..ndata
 */
int f_ll2i(ARG2) {

    double to_lat[1], to_lon[1];
    int i;
    unsigned int iptr;

    if (mode == -1) {
        latlon = 1;
    }
    if (mode >= 0) {
        if (output_order != wesn) return 1;
        to_lon[0] = atof(arg1);
        to_lat[0] = atof(arg2);
        i = gctpc_ll2xy_init(sec, lon, lat);
	if (i != 0) {
	    iptr = 0;	
	}
	else {
            i = gctpc_ll2i(1, to_lon, to_lat, &iptr);
	    if (i != 0) iptr = 0;
	}
        sprintf(inv_out,"%lf %lf -> (%u)",to_lon[0], to_lat[0], iptr);
    }
    return 0;
}
