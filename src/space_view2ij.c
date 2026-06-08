/** @file
 * @brief Find nearest-neighbor grid point for a given latitude/longitude.
 * 
 * Based on algorithms from:
 * LRIT/HRIT Global Specification, Coordination Group for Meteorological Satellites
 * Doc No CGMS 03 isssue 2.6
 * Date - 12 August 1999
 * 
 * This document assumed certain constants: satellite height radius (pole/equator) which 
 * differed from the values in the grib file. I attempted to replace the constants with the 
 * grib-specified values.
 * 
 * I used a different test to see if the grid point is visible than suggested by the code 
 * MSG_navigation_v1.01.c by EUMETSAT.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 4/2011 | W. Ebisuzaki | Initial
 * 9/2017 | W. Ebisuzaki | fix defintion of lop (was using lap = 0.0)
 *                         correct conversion of lap, lop to radians
 * @author Public Domain: Wesley Ebisuzaki @date 4/11/2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "wmath.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
// static double *lon;

/** Current output order type. */
extern enum output_order_type output_order;

/** Number of grid points in the x-direction. */
static unsigned int nnx;

/** Number of grid points in the y-direction. */
static unsigned int nny;

/** Satellite height. */
static double sat_height;

/** Satellite radius from pole. */
static double r_pol;

/** Satellite radius from equator. */
static double r_eq;

/** Effective radius (r_pol * r_eq). */
static double r_pol_eq;

/** Factor for converting between grid lengths and Earth-centered coordinates. */
static double factor_10;

/** Latitude of the sub-satellite point. */
static double lap;

/** Longitude of the sub-satellite point. */
static double lop;

/** Inverse of horizontal angular resolution of the sensor. */
static double inv_rx;

/** Inverse of vertical angular resolution of the sensor. */
static double inv_ry;

/** x-coordinate of the sub-satellite point. */
static double xp;

/** y-coordinate of the sub-satellite point. */
static double yp;

/** Apparent diameter of Earth in grid lengths, in x-direction. */
static double dx;

/** Apparent diameter of Earth in grid lengths, in y-direction. */
static double dy;

/**
 * Initialize the space view projection for nearest-neighbor grid point search.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2011
 */
int space_view_init(unsigned char **sec) {

    double major, minor, orient_angle, angular_size;
    int x0, y0;
    int nres, nscan;
    unsigned int nnpnts;

    fprintf(stderr,"ALPHA: experimental space_view2ij\n");
    if (sec == NULL || sec[3] == NULL) fatal_error("space_view_init: sec/sec[3] == NULL","");
 
    if (code_table_3_1(sec) != 90) fatal_error("space_view_init: not space view grid","");
    if (output_order != wesn) fatal_error("space_view_init: order must be we:sn","");

//printf("space_view_init >>>> table 3.1 %d\n", code_table_3_1(sec));

    get_nxny_(sec, &nnx, &nny, &nnpnts, &nres, &nscan);
    axes_earth(sec, &major, &minor, NULL);
//fprintf(stderr,">> axes %lf minor %lf\n", major,minor);

    r_eq = major * 0.001;
    r_pol = minor * 0.001;
    r_pol_eq = (r_pol * r_pol) / (r_eq * r_eq);

//    factor_10 = (r_eq*r_eq - r_pol*r_pol) / (r_eq * r_eq);
    factor_10 = 1.0 - r_pol_eq;

//fprintf(stderr,">> r_eq %lf r_pol %lf rad_factor %lf \n",r_eq, r_pol, factor_10);

    angular_size = 2.0 * asin(1e6/ uint4(sec[3]+68));
    sat_height = uint4(sec[3]+68) * 1e-6 * r_eq;
//fprintf(stderr,">> sat height %lf\n",sat_height);

    lap = int4(sec[3]+38);
    lop = int4(sec[3]+42);
    /* I am guessing that a scale factor has to be added */
    lap *= 1e-6;
    lop *= 1e-6;
// fprintf(stderr,">> lap  %lf lop %lf degrees\n",lap, lop);
    if (lap != 0.0) return 0;   // need to extend code

    /* convert to radians */
    lap *= (M_PI/180.0);
    lop *= (M_PI/180.0);

    orient_angle = int4(sec[3]+64);
    /* I am guessing that a scale factor has to be added */
    orient_angle *= 1e-6;
// fprintf(stderr,">> orientation angle %lf\n", orient_angle);
    if (orient_angle != 0.0) return 0;  // need to extend code

    xp = int4(sec[3]+55) * 0.001;
    yp = int4(sec[3]+59) * 0.001;

//fprintf(stderr,">> xp %lf yp %lf pixels\n",xp, yp);

    x0 = int4(sec[3]+72);
    y0 = int4(sec[3]+76);
//fprintf(stderr,">> origin x0 %d yo %d pixels\n",x0, y0);

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

    dx = int4(sec[3]+47);
    dy = int4(sec[3]+51);
//fprintf(stderr,">> dia: dx %lf dy %lf pixels\n",dx, dy);

    inv_rx = dx / angular_size;
    inv_ry = (r_eq/r_pol) * dy / angular_size;

// fprintf(stderr,">> factor %.18lf %.18lf, q: %.19lf %.18lf\n", 256*256.0/(-781648343.0), 256*256.0/(-781648343.0), 1/inv_rx, 1/inv_ry);


    return 0;
}

/**
 * Find the closest grid point in the space view projection using nearest-neighbor search.
 * 
 * Code limited to orientation == 0 and satellite location = 0N.
 *
 * @param sec Pointer to the GRIB section.
 * @param plat Latitude of the point to search for.
 * @param plon Longitude of the point to search for.
 * 
 * @return The index of the closest grid point, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 4/2011
 */
long int space_view_closest(unsigned char **sec, double plat, double plon) {

    double phi_e, cos_phi_e, r_e;
    double r1, r2, r3, rn, x, y;
    long int ix, iy;

// fprintf(stderr,"space_view_closest want lat %lf lon %lf\n",plat, plon);

    if (plat > 90.0 || plat < -90.0) return -1;

    plon *= (M_PI/180.0);
    plat *= (M_PI/180.0);

    /* preliminary check to see if point is visable */
    if (fmod( fabs(plat-lap), M_PI) > M_PI_2) return -1;

// printf(" calc clat/phi_e  r_pol_eq = %lf\n",r_pol_eq);
    phi_e = atan(r_pol_eq*tan(plat));
    cos_phi_e = cos(phi_e);

    r_e = r_pol / sqrt(1.0 - factor_10 * cos_phi_e*cos_phi_e);
// printf("r_e %lf factor_10 %lf sat_height %lf\n", r_e, factor_10, sat_height);
// printf("r_eq %lf r_pol %lf xp %lf yp %lf\n", r_eq, r_pol, xp, yp);

    r1 = sat_height - r_e*cos_phi_e*cos(plon - lop);
    r2 = -r_e * cos_phi_e * sin(plon - lop);
    r3 = r_e * sin(phi_e);
    rn = sqrt(r1*r1 + r2*r2 + r3*r3);

// printf("r1 %lg r2 %lg r3 %lg rn %lg\n", r1, r2, r3, rn);

    // test to see if the point is visible
    // barely visible if the triangle between the center of the earth
    // the satellite and the point on the earth is a right angle triangle
    // if rs*rs + re*re > sat_height*sat_height, then not visiable

// printf("test view %lf\n",sat_height*sat_height - rn*rn - r_e*r_e);
    if (sat_height*sat_height <= rn*rn + r_e*r_e)  return -1;

    x = atan(-r2/r1);
    y = asin(r3/rn);

    x *= inv_rx;
    y *= inv_ry;

// fprintf(stderr,"origin center values X %lf Y %lf nnx %u nny %u\n", x, y, nnx, nny);
    x += xp;
    y += yp;

// fprintf(stderr,"we:sn values X %lf Y %lf xp %lf yp %lf\n", x, y, xp, yp);

    ix = floor(x + 0.5);
    iy = floor(y + 0.5);
    if (ix < 0 || ix >= nnx || iy < 0 || iy >= nny) {
        return -1;
    }
    return (ix + iy*nnx);
}
