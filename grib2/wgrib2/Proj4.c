#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* proj4.c  interface routines to the Proj.4 library
   6/2012 Public Domain Dusan Jovic
   8/2014 Public Domain Wesley Ebisuzaki


  latlon
  lambert conformal
  ncep rotated latlon B grid
  10/2015 lambert azimuthal equal area
*/

#ifdef USE_PROJ4

#include "proj_api.h"
#include "proj4_wgrib2.h"

extern int latlon;
extern enum output_order_type output_order;
extern int use_proj4;


#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif


extern double *lat, *lon;

static double dx, dy, x_0, y_0, x00, xN;
static unsigned int gdt;
static int nx, ny;
static unsigned int nx_, ny_;

static projPJ pj_grid, pj_latlon;

int proj4_init(unsigned char **sec, double *grid_lon, double *grid_lat) {

    unsigned char *gds;

    double r_maj;                           /* major axis                   */
    double r_min;                           /* minor axis                   */
    double latsp1;                          /* first standard parallel      */
    double latsp2;                          /* second standard parallel     */
    double c_lon;                           /* center longitude             */
    double c_lat;                           /* center latitude              */
    double lon1;
//    double lon2;
    double lat1;
//    double lat2;

    int nres, nscan,has_np, center;
    unsigned int npnts;
    char proj4_def[1000];

    if (grid_lat == NULL || grid_lon == NULL) return 1;
    gdt = code_table_3_1(sec);
    gds = sec[3];
    center = GB2_Center(sec);

    get_nxny(sec, &nx, &ny, &npnts, &nres, &nscan);
    get_nxny_(sec, &nx_, &ny_, &npnts, &nres, &nscan);
    if (nx_ < 1 || ny_ < 1 || nx_*ny_ != npnts)   return 1;

    /* only process certain grids */
    pj_grid = NULL;
    pj_latlon = NULL;

    x_0 = y_0 = x00 = xN = 0.0;

    if (gdt == 0) {     /* lat-lon grid */
        dx = grid_lon[1] - grid_lon[0];
        dy = grid_lat[nx] - grid_lat[0];
        x_0 = grid_lon[0];
        x00 = grid_lon[0] - 0.5*dx;
        xN = grid_lon[nx-1] + 0.5*dx;
        y_0 = grid_lat[0];
        return 0;
    }
    else if (gdt == 10 && (GDS_Mercator_ori_angle(gds) == 0.0) ) {            // mercator no rotation
        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);
        dx = abs(GDS_Mercator_dx(gds));
        dy = abs(GDS_Mercator_dy(gds));

        /* central point */
        c_lon = 0.0;
        c_lat = GDS_Mercator_latD(gds);

        sprintf(proj4_def,"+proj=merc +lat_ts=%lf +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=%lf +b=%lf", 
	    c_lat, r_maj, r_min);

        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ( (pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lat1 = GDS_Mercator_lat1(gds);
        lon1 = GDS_Mercator_lon1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
                  fatal_error("proj4_init: Proj4 transform to lat-lon","");
    }
    else if (gdt == 20) {            // polar stereographic

        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);
        dy      = fabs(GDS_Polar_dy(gds));
        dx      = fabs(GDS_Polar_dx(gds));

        /* central point */
        c_lon = GDS_Polar_lov(gds);
        c_lat = GDS_Polar_lad(gds);

	/* strange but np/sp flag is used by proj4 but not gctpc */
	has_np = ((flag_table_3_5(sec) & 128) == 0);

        sprintf(proj4_def,"+proj=stere +lat_ts=%lf +lat_0=%s +lon_0=%lf +k_0=1 +x_0=0 +y_0=0 +a=%lf +b=%lf", 
		c_lat, has_np ? "90" : "-90", c_lon, r_maj,r_min);

        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ( (pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lon1 = GDS_Polar_lon1(gds);
        lat1 = GDS_Polar_lat1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
                     fatal_error("proj4_init: Proj4 transform to lat-lon","");

    }
    else if (gdt == 30) {            // lambert conformal conic

        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);
        dx      = fabs(GDS_Lambert_dx(gds));
        dy      = fabs(GDS_Lambert_dy(gds));

        /* latitudes of tangent/intersection */
        latsp1 = GDS_Lambert_Latin1(gds);
        latsp2 = GDS_Lambert_Latin2(gds);

        /* central point */
        c_lon = GDS_Lambert_Lov(gds);
        c_lat = GDS_Lambert_LatD(gds);

        sprintf(proj4_def,"+proj=lcc +lon_0=%lf +lat_0=%lf +lat_1=%lf +lat_2=%lf +a=%lf +b=%lf",c_lon,
                   c_lat,latsp1,latsp2,r_maj,r_min);

        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

 
        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ((pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lon1 = GDS_Lambert_Lo1(gds);
        lat1 = GDS_Lambert_La1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;
        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) fatal_error("proj4_init: Proj4 transform to lat-lon","");
    }
    else if (gdt == 140) {            // lambert azimuthal equal area
        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min);
        dx      = fabs(GDS_Lambert_Az_dx(gds));
        dy      = fabs(GDS_Lambert_Az_dy(gds));

        /* central point */
        c_lon = GDS_Lambert_Az_Cen_Lon(gds);
        c_lat = GDS_Lambert_Az_Std_Par(gds);

        sprintf(proj4_def,"+proj=laea +lon_0=%lf +lat_0=%lf +a=%lf +b=%lf",c_lon,c_lat,r_maj,r_min);
        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ((pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lon1 = GDS_Lambert_Az_Lo1(gds);
        lat1 = GDS_Lambert_Az_La1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;
        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
		fatal_error("proj4_init: Proj4 transform to lat-lon","");
    }
    else if (center == NCEP && gdt == 32769) {         // ncep rotated latlon Non-E 

        /* get earth axis */ 
        axes_earth(sec, &r_maj, &r_min);

        /* dx, dy */
        dx = fabs(GDS_NCEP_B_LatLon_dlon(gds) * 0.000001);
        dy = fabs(GDS_NCEP_B_LatLon_dlat(gds) * 0.000001);
        dx *= DEG_TO_RAD;
        dy *= DEG_TO_RAD;

        /* central point */
        c_lon = GDS_NCEP_B_LatLon_tlm0d(gds) * 0.000001;
        c_lat = GDS_NCEP_B_LatLon_tph0d(gds) * 0.000001;

        lon1 = GDS_NCEP_B_LatLon_lon1(gds) * 0.000001;
        lat1 = GDS_NCEP_B_LatLon_lat1(gds) * 0.000001;

        sprintf(proj4_def,"+proj=ob_tran +o_proj=latlon +o_lon_p=%f +o_lat_p=%f",c_lon,90.0+c_lat);
        if ((pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlon");
        if ((pj_grid = pj_init_plus(proj4_def)) == NULL ) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;
        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) fatal_error("proj4_init: Proj4 transform to lat-lon","");
 
    }
    else {
       return 1;
    }
    return 0;

}

int Proj4_ll2xy(int n, double *lon, double *lat, double *x, double *y) {

    int i;
    double rlon, rlat, inv_dx, inv_dy;

    inv_dx = 1.0 / dx;
    inv_dy = 1.0 / dy;
    if (gdt == 0) {             // lat-lon 
#pragma omp parallel for schedule(static) private(i,rlon,rlat)
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


#pragma omp parallel for schedule(static) private(i,rlon,rlat)
    for (i = 0; i < n; i++) {
        rlon = lon[i] * DEG_TO_RAD;
        rlat = lat[i] * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &rlon, &rlat, NULL) != 0 ) {
	    x[i] = y[i] = UNDEFINED;
	}
	else {
            x[i] = (rlon - x_0)*inv_dx;
            y[i] = (rlat - y_0)*inv_dy;
	}
    }
    return 0;
}

int Proj4_ll2i(int n, double *lon, double *lat, unsigned int *ipnt) {
    int error;
    unsigned int i;
    double rlon, rlat, inv_dx, inv_dy, x, y; 

    inv_dx = 1.0 / dx;
    inv_dy = 1.0 / dy;
    if (gdt == 0) {             // lat-lon
#pragma omp parallel for schedule(static) private(i,rlon,rlat,x,y)
        for (i = 0; i < n; i++) {
            rlon = lon[i];
            if (rlon > xN) rlon -= 360.0;
            if (rlon < x00) rlon += 360.0;
            rlat = lat[i];
	    
            x = floor((rlon - x_0) * inv_dx + 0.5);
            y = floor((rlat - y_0) * inv_dy + 0.5);

            if (x < 0 || x >= nx || y < 0 || y >= ny) {
                ipnt[i] = 0;
            }   
            else {
                ipnt[i] = (unsigned int) x + nx* ((unsigned int) y) + 1;
            }
        }
        return 0;
    }

    error = 0;

    for (i = 0; i < n; i++) {
        rlon = lon[i] * DEG_TO_RAD;
        rlat = lat[i] * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &rlon, &rlat, NULL) != 0 ) error = 1;

        x = floor((rlon - x_0)*inv_dx + 0.5);
        y = floor((rlat - y_0)*inv_dy + 0.5);

        if (x < 0 || x >= nx || y < 0 || y >= ny) {
            ipnt[i] = 0;
	}
        else {
            ipnt[i] = (unsigned int) x + nx*(unsigned int) y + 1;
        }
    }
    return error;
}

/*
 * HEADER:100:proj4_ll2ij:inv:2:x=lon y=lat, converts lon-lat (i,j) using proj.4 (experimental)
 */
int f_proj4_ll2ij(ARG2) {

    double x[1], y[1], to_lat[1], to_lon[1];
    int i;

    if (mode == -1) {
        latlon = 1;
    }
    if (mode >= 0) {
        if (output_order != wesn)  return 1;
        to_lon[0] = atof(arg1);
        to_lat[0] = atof(arg2);
        i = proj4_init(sec, lon, lat);
        if (i == 0)  {
            i = Proj4_ll2xy(1, to_lon, to_lat, x , y);
	    if (i) x[0] = y[0] = -1.0;
            sprintf(inv_out,"%lf %lf -> (%lf,%lf)",to_lon[0], to_lat[0], x[0]+1.0, y[0]+1.0);
        }
    }
    return 0;
}

int Proj4_ij2ll(unsigned char **sec, int n, double *x, double *y, double *lon, double *lat) {

    int i, error;
    double xx, yy;

    if (gdt == 0) {
#pragma omp parallel for schedule(static)
        for (i = 0; i < n; i++) {
            lon[i] = dx * x[i] + x_0;
            lat[i] = dy * y[i] + y_0;
	    if (lon[i] < 0.0) lon[i] += 360.0;
	}
        return 0;
    }
    error = 0;

#pragma omp parallel for schedule(static) private(i,xx,yy)
    for (i = 0; i < n; i++) {
        xx = x[i] + x_0;
        yy = y[i] + y_0;
/* test */
        xx = dx*(x[i] -1.0) + x_0;
        yy = dy*(y[i] -1.0) + y_0;
        if ( pj_transform(pj_grid, pj_latlon, 1, 1, &xx, &yy, NULL) != 0 ) {
	    error = 1;
            lon[i] = 999.0;
            lat[i] = 999.0;
	}
	else {
            lon[i] = xx * RAD_TO_DEG;
            lat[i] = yy * RAD_TO_DEG;
            if (lon[i] < 0.0) lon[i] += 360.0;
	}
    }
    return error;

}


int Proj4_xy2ll(int n, double *x, double *y, double *lon, double *lat) {

    int i, error;
    double xx, yy;

    if (gdt == 0) {
#pragma omp parallel for schedule(static)
        for (i = 0; i < n; i++) {
            lon[i] = dx * x[i] + x_0;
            lat[i] = dy * y[i] + y_0;
            if (lon[i] < 0.0) lon[i] += 360.0;
        }
        return 0;
    }
    error = 0;
    for (i = 0; i < n; i++) {
        xx = x[i] + x_0;
        yy = y[i] + y_0;
        if ( pj_transform(pj_grid, pj_latlon, 1, 1, &xx, &yy, NULL) != 0 ) error = 1;
        lon[i] = xx * RAD_TO_DEG;
        lat[i] = yy * RAD_TO_DEG;
        if (lon[i] < 0.0) lon[i] += 360.0;
    }
    return error;
}


/*
 * HEADER:100:proj4_ij2ll:inv:2:X=x Y=y, converts to (i,j) to lon-lat using proj.4  (experimental) we:sn
 */
int f_proj4_ij2ll(ARG2) {
    int i;
    double x, y, rlon, rlat;

    if (mode == -1) {
	latlon = 1;
    }
    else if (mode >= 0) {
	x = atof(arg1);
	y = atof(arg2);
        i = proj4_init(sec, lon, lat);
        if (i == 0)  {
	    i = Proj4_ij2ll(sec, 1, &x, &y, &rlon, &rlat);
	    if (i == 0) {
		sprintf(inv_out,"x=%lf y=%lf lon=%lf lat=%lf", x, y, rlon, rlat);
	    }
        }
    }
    return 0;
}




/*
 * HEADER:100:proj4_ll2i:inv:2:x=lon y=lat, converts to (i) using proj.4  (experimental) 1..ndata
 */
int f_proj4_ll2i(ARG2) {

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
        i = proj4_init(sec, lon, lat);
	if (i) iptr = 0;
	else {
            i = Proj4_ll2i(1, to_lon, to_lat, &iptr);
	    if (i) iptr = 0;
        }
        sprintf(inv_out,"%lf %lf -> (%u)",to_lon[0], to_lat[0], iptr);
    }
    return 0;
}

/*
 * HEADER:100:proj4:misc:1:X=0,1 use proj4 library for geolocation (testing)
 */

int f_proj4(ARG1) {
   use_proj4 = (strcmp(arg1,"1") == 0);
   return 0;
}

int proj4_get_latlon(unsigned char **sec, double **lon, double **lat) {

    int nnx, nny, nres, nscan, error;
    unsigned int i, nnpnts;
    double *llat, *llon;

    llat = *lat;
    llon = *lon;

    if (proj4_init(sec, llon, llat) != 0) return 1;

    get_nxny(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

    /* potentially staggered */

    if (llat != NULL) {
        free(llat);
        free(llon);
        *lat = *lon = llat = llon = NULL;
    }

    if ((*lat = llat = (double *) malloc(sizeof(double) * (size_t) nnpnts)) == NULL) {
        fatal_error("proj4_get_latlon memory allocation failed","");
    }
    if ((*lon = llon = (double *) malloc(sizeof(double) * (size_t) nnpnts)) == NULL) {
        fatal_error("proj4_get_latlon memory allocation failed","");
    }

    /* put x[] and y[] values in lon and lat */
    if (stagger(sec, nnpnts, llon, llat)) fatal_error("proj4: stagger problem","");

    /* handle lat-lon grid differently */

    if (gdt == 0) {
#pragma omp parallel for private(i)
        for (i = 0; i < nnpnts; i++) {
            llon[i] = dx * llon[i] + x_0;
            llat[i] = dy * llat[i] + y_0;
            if (llon[i] < 0.0) llon[i] += 360.0;
            if (llon[i] > 360.0) llon[i] -= 360.0;
        }
        return 0;
    }

    /* proj4 projections */

#pragma omp parallel for private(i)
    for (i = 0; i < nnpnts; i++) {
        llon[i] = llon[i] * dx + x_0;
        llat[i] = llat[i] * dy + y_0;
    }

    error = pj_transform(pj_grid, pj_latlon, (long) nnpnts, (long) 1, llon, llat, NULL);

#pragma omp parallel for private(i)
    for (i = 0; i < nnpnts; i++) {
        llon[i] = llon[i] * RAD_TO_DEG;
        llat[i] = llat[i] * RAD_TO_DEG;
	if (llon[i] < 0.0) llon[i] += 360.0;
    }        
    return error;
}


#else
int f_proj4(ARG1) {
   if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
	return 1;
}
int f_proj4_ll2ij(ARG2) {
   if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
   return 1;
}
int f_proj4_ij2ll(ARG2) {
   if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
   return 1;
}
int f_proj4_ll2i(ARG2) {
   if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
   return 1;
}
#endif
