/** @file
 * @brief Convert between (i,j) and (lon, lat) coordinates.
 *
 * Adapted from grib2ctl.pl
 *
 * As of 7/2021, this routine is not used by wgrib2.
 *
 * However, want code that goes from (i,j) -> (lon, lat) and (lon, lat) -> (i,j)
 * for all the projections. This is good for 
 *
 * 1) speedup closest()   - find nearest neighbor
 * 2) future interpolation that doesn't depend on ipolates
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2005 | W. Ebisuzaki | Initial
 * 7/2021 | W. Ebisuzaki | Added OpenMP
 * @author Public Domain: Wesley Ebisuzaki @date 2005
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "wmath.h"
#include "grb2.h"
#include "wgrib2.h"

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Pointer to array of longitude values. */
extern double *lon;

/** Pointer to array of latitude values. */
extern double *lat;

/** Current output order type. */
extern enum output_order_type output_order;

/**
 * Convert from (i,j) to (lon,lat) and (lon,lat) to (i,j) for regular grids.
 * 
 * As of 7/2021, this routine is not used by wgrib2.
 * 
 * However, want code that goes from (i,j) -> (lon, lat) and (lon, lat) -> (i,j)
 * for all the projections. This is good for 
 *
 * 1) speedup closest()   - find nearest neighbor
 * 2) future interpolation that doesn't depend on ipolates
 *
 * Adapted from grib2ctl.pl
 * 
 * @param sec Pointer to GRIB sections.
 * @param lat Pointer to array of latitude values.
 * @param lon Pointer to array of longitude values.
 * @param n Number of points.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
int rot_regular2ij(unsigned char **sec, double **lat, double **lon, int n) {


    unsigned char *gds;
    double units, *tlon, *tlat;
    double sp_lat, sp_lon, angle_rot;
    double sin_a, cos_a;
    int basic_ang, sub_ang, i;
    double a, b, r, pr, gr, pm, gm, glat, glon;

    gds = sec[3];
    // npnts = GB2_Sec3_npts(sec);

    basic_ang = GDS_LatLon_basic_ang(gds);
    sub_ang = GDS_LatLon_sub_ang(gds);
    if (basic_ang != 0) {
        units = (double) basic_ang / (double) sub_ang;
    }
    else {
        units = 0.000001;
    }

    sp_lat = GDS_RotLatLon_sp_lat(gds) * units;
    sp_lon = GDS_RotLatLon_sp_lon(gds) * units;
    angle_rot = GDS_RotLatLon_rotation(gds) * units;

    // inverse transformation, reverse rotation angle
    angle_rot = -angle_rot;

    a = (M_PI/180.0) * (90.0+sp_lat);
    b = (M_PI/180.0) * sp_lon;
    r = (M_PI/180.0) * angle_rot;

    sin_a = sin(a);
    cos_a = cos(a);

    tlat = *lat;
    tlon = *lon;

#ifdef USE_OPENMP
#pragma omp parallel for private(i,pr,gr,pm,gm,glat,glon)
#endif
    for (i = 0; i < n; i++) {
        pr = (M_PI/180.0) * tlat[i];
        gr = -(M_PI/180.0) * tlon[i];
        pm = asin(cos(pr)*cos(gr));
        gm = atan2(cos(pr)*sin(gr),-sin(pr));
        glat = (180.0/M_PI)*(asin(sin_a*sin(pm)-cos_a*cos(pm)*cos(gm-r)));
        glon = -(180.0/M_PI)*(-b+atan2(cos(pm)*sin(gm-r),sin_a*cos(pm)*cos(gm-r)+cos_a*sin(pm)) );
        tlat[i] = glat;
        tlon[i] = glon;
    }
    return 0;
}
