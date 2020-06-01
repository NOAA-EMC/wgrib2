#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "proj.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int latlon;
extern double *lon, *lat;
extern enum output_order_type output_order;

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


/* adapted from grib2ctl.pl */

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
    for (i = 0; i < n; i++) {
        pr = (M_PI/180.0) * *tlat;
        gr = -(M_PI/180.0) * *tlon;
        pm = asin(cos(pr)*cos(gr));
        gm = atan2(cos(pr)*sin(gr),-sin(pr));
        glat = (180.0/M_PI)*(asin(sin_a*sin(pm)-cos_a*cos(pm)*cos(gm-r)));
        glon = -(180.0/M_PI)*(-b+atan2(cos(pm)*sin(gm-r),sin_a*cos(pm)*cos(gm-r)+cos_a*sin(pm)) );
        *tlat++ = glat;
        *tlon++ = glon;
    }
   return 0;
}

