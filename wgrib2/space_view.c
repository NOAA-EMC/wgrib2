#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * spaceview.c
 *
 * 2011-4-11 Public Domain Wesley Ebisuzaki
 *
 * this routine assigns lat/lon to the grid points of a space view 
 * perspective grid
 *
 * based on algorithms from
 *
 * LRIT/HRIT Global Specification, Coordination Group for Meteorological Satellites
 * Doc No CGMS 03 isssue 2.6 
 * date 12 August 1999
 *
 * This document assumed certain constants: satellite height
 *  radius (pole/equator) which differed from the values in the grib file
 *  I attempted to replace the constants with the grib-specified values.
 *
 * code can be speeded up x=-x and y=-y relationships
 *
 * code limited to orient == 0 and sat lat = 0
 *  v1.0 4-2011
 *
 * 4/2015: L. Majewski fixed defn of lop
 * 9/2017: W. Ebisuzaki lop does not need to be changed to radians
 */


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

extern enum output_order_type output_order;


int space_view2ll(unsigned char **sec, double **lat, double **lon) {
    double major, minor, r_eq, r_pol, sat_height;
    double lap, lop, orient_angle, angular_size;
    double xp, yp, dx, dy, rx, ry, x, y;
    double cos_x, cos_y, sin_x, sin_y;
    double factor_1, factor_2, tmp1, Sd, Sn, Sxy, S1, S2, S3;
    int x0, y0, i, ix, iy;
    double *llat, *llon;
    double *s_x, *c_x;

    int nnx, nny, nres, nscan;
    unsigned int nnpnts;

    get_nxny(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

//fprintf(stderr,"ALPHA: experimental space_view2ll scan=%d\n", nscan >> 4);

    axes_earth(sec, &major, &minor);
//fprintf(stderr,">> axes %lf minor %lf\n", major,minor);

    r_eq = major * 0.001;
    r_pol = minor * 0.001;

    angular_size = 2.0 * asin(1e6/ uint4(sec[3]+68));
    sat_height = uint4(sec[3]+68) * 1e-6 * r_eq;
//fprintf(stderr,">> sat height %lf\n",sat_height);

    lap = int4(sec[3]+38);
    lop = int4(sec[3]+42);

    /* apply default scaling factor */
    lap *= 1e-6;
    lop *= 1e-6;
//fprintf(stderr,">> lap  %lf lop %lf degrees\n",lap, lop);
    if (lap != 0.0) return 0;	// need to extend code

    /* convert to radians */
    lap *= (180.0/M_PI);
    // lop *= (180.0/M_PI);

    orient_angle = int4(sec[3]+64);
    /* apply default scaling factor */
    orient_angle *= 1e-6;
    if (orient_angle != 0.0) return 0;	// need to extend code

    xp = int4(sec[3]+55) * 0.001;
    yp = int4(sec[3]+59) * 0.001;

//fprintf(stderr,">> xp %lf yp %lf pixels\n",xp, yp);

    x0 = int4(sec[3]+72);
    y0 = int4(sec[3]+76);
//fprintf(stderr,">> origin x0 %d yo %d pixels\n",x0, y0);

    dx = int4(sec[3]+47);
    dy = int4(sec[3]+51);
//fprintf(stderr,">> dia: dx %lf dy %lf pixels\n",dx, dy);

    rx = angular_size / dx;
    ry = (r_pol/r_eq) * angular_size / dy;

//fprintf(stderr,">> factor %.17lg %.18lg, q: %.18lg %.18lg\n", 256*256.0/(-781648343.0), 
//	256*256.0/(-781648343.0), rx, ry);

    if (nnx < 1 || nny  < 1 || nnx*nny != nnpnts) {
        fprintf(stderr,"space_view2ll need rectangular grid\n");
        return 0;
    }

    if ((*lat = (double *) malloc(sizeof(double) * (size_t) nnpnts)) == NULL) {
        fatal_error("space_view2ll memory allocation failed","");
    }
    if ((*lon = (double *) malloc(sizeof(double) * (size_t) nnpnts)) == NULL) {
        fatal_error("space_view2ll memory allocation failed","");
    }
    llat = *lat;
    llon = *lon;

    /* find center point in we:sn coordinate */

    if (GDS_Scan_x(nscan)) {
        xp = xp - x0;
    } else {
        xp = (nnx-1) - (xp - x0);
    }
    if (GDS_Scan_y(nscan)) {
	yp = yp - y0;
    }
    else {
	yp = (nny-1) - (yp - y0);
    }
//fprintf(stderr,">> new center point point x/y=%lf %lf nnx=%d nny=%d\n", xp,yp,nnx, nny);
//fprintf(stderr,">> rx %lf ry %lf\n", rx,ry);

    i = 0;
    factor_2 = (r_eq/r_pol)*(r_eq/r_pol);
    factor_1 = sat_height * sat_height - r_eq * r_eq;
//fprintf(stderr," factor_1 %lf factor_2 %lf\n",factor_1,factor_2);

    s_x = (double *) malloc(sizeof(double) * (size_t) nnx);
    c_x = (double *) malloc(sizeof(double) * (size_t) nnx);
    if (s_x == NULL || c_x == NULL) fatal_error("space_view: memory allocation","");

    for (ix = 0; ix < nnx; ix++) {
	x = (ix - xp) * rx;
	s_x[ix] = sin(x);
	c_x[ix] = sqrt(1.0 - s_x[ix]*s_x[ix]);
    }
    
    for (iy = 0; iy < nny; iy++) {
	y = (iy - yp) * ry;
	sin_y = sin(y);
//	cos_y = cos(y);
	cos_y = sqrt(1.0 - sin_y*sin_y);
// printf("iy %d y %lf cos %lf sin %lf\n", iy, y, cos_y, sin_y);

// old	tmp1 = (cos_y*cos_y + factor_2*sin_y*sin_y);
	tmp1 = (1 + (factor_2-1.0)*sin_y*sin_y);

        for (ix = 0; ix < nnx; ix++, i++) {
	    x = (ix - xp) * rx;
//	    sin_x = sin(x);
////	    cos_x = cos(x);
//	    cos_x = sqrt(1.0 - sin_x*sin_x);
	    sin_x = s_x[ix];
	    cos_x = c_x[ix];

	    Sd = sat_height * cos_x * cos_y;
	    Sd = Sd * Sd - tmp1*factor_1;
	    if (Sd <= 0.0) {	// outside of view
		llat[i] = llon[i] = UNDEFINED_ANGLE;
	    }
	    else {
	        Sd = sqrt(Sd);
	        Sn = (sat_height*cos_x*cos_y - Sd) / tmp1;
	        S1 = sat_height - Sn * cos_x * cos_y;
	        S2 = Sn * sin_x * cos_y;
	        S3 = Sn * sin_y;
	        Sxy = sqrt(S1*S1 + S2*S2);
	        llon[i] = atan(S2/S1)*(180.0/M_PI) + lop;
		if (llon[i] > 360.0) llon[i] -= 360.0;
		// if (llon[i] < 0.0) llon[i] += 360.0;  should never happen as lop >= 0
	        llat[i] = atan(factor_2*S3/Sxy)*(180.0/M_PI);
/*
if ((iy == 1588 || iy == 300) && ix ==  1588) {
printf("ix %d iy %d x %lf y %lf\n",ix,iy,x,y);
printf("cos_x %lf sin_x %lf\n", cos_x, sin_x);
printf("cos_y %lf sin_y %lf\n", cos_y, sin_y);
printf("Sd=%lf Sn=%lf S1 %lf S2 %lf S3 %lf\n", Sd, Sn, S1 , S2, S3);
printf("Sxy=%lf S3/Sxy=%lf atan() %lf\n", Sxy, S3/Sxy, atan(S3/Sxy));
printf("lat %lf lon %lf i=%d\n", llat[i] , llon[i], i);
}
*/
	    }
	}
    }
    free(s_x);
    free(c_x);
//fprintf(stderr,">>return\n");
    return 0;
}
