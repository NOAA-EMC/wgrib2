/* proj4_transform.c Public domain 10/2012 Wesley Ebisuzaki */
/* proj4_ll2xy   convert lat/lon to X,Y */
/* proj4_xy2ll   convert lat/lon to X,Y */

#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_PROJ4

#include "proj_api.h"
#include "proj4_wgrib2.h"


int proj4_ll2xy(struct proj4_struct *projection, int n, double *lon, double *lat, double *x, double *y) {

    int i;
    double rlon, rlat;


    if (projection->proj_is_nop == 1) {				// lat-lon
#pragma omp parallel for schedule(static) private(i,rlon,rlat)
        for (i = 0; i < n; i++) {
            rlon = lon[i];
            rlat = lat[i];
            x[i] = (rlon - projection->x_0);
            y[i] = (rlat - projection->y_0);
        }
        return 0;
    }

#pragma omp parallel for schedule(static) private(i,rlon,rlat)
    for (i = 0; i < n; i++) {
        rlon = lon[i] * DEG_TO_RAD;
        rlat = lat[i] * DEG_TO_RAD;

        if ( pj_transform(projection->pj_latlon, projection->pj_grid, 1, 1, &rlon, &rlat, NULL) != 0 ) {
            x[i] = y[i] = UNDEFINED;
        }
        else {
            x[i] = (rlon - projection->x_0);
            y[i] = (rlat - projection->y_0);
        }
    }
    return 0;
}


int proj4_xy2ll(struct proj4_struct *projection, int n, double *x, double *y, double *lon, double *lat) {
    int i;
    double rlon, rlat;

    if (projection->proj_is_nop == 1) {				// lat-lon relative to x_0, y_0
#pragma omp parallel for schedule(static) private(i,rlon,rlat)
        for (i = 0; i < n; i++) {
            rlon = x[i] + projection->x_0;
            rlat = y[i] + projection->y_0;
	    if (rlon >= 360.0) rlon -= 360.0;
	    if (rlon < 0.0) rlon += 360.0;
	    if (rlon < 0.0) rlon += 360.0;
	    lon[i] = rlon;
	    lat[i] = rlat;
	}
        return 0;
    }
#pragma omp parallel for schedule(static) private(i,rlon,rlat)
    for (i = 0; i < n; i++) {
        rlon = x[i] + projection->x_0;
        rlat = y[i] + projection->y_0;
        if ( pj_transform(projection->pj_grid, projection->pj_latlon, 1, 1, &rlon, &rlat, NULL) != 0 ) {
	    lon[i] = lat[i] = UNDEFINED;
	}
	else {
            lon[i] = rlon * RAD_TO_DEG;
            lat[i] = rlat * RAD_TO_DEG;
            if (lon[i] < 0.0) lon[i] += 360.0;
            if (lon[i] > 360) lon[i] -= 360.0;
	}
    }
    return 0;
}

#endif
