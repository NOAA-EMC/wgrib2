#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* NCEP dlat/dlon are only good to milli-degrees because
   they are converted from grib1 */

#define ERROR (0.001 * nx_)
#define TOTAL_ERROR 0.001
/*
 * HEADER:-1:cyclic:inv:0:is grid cyclic? (not for thinned grids)
 */

extern enum output_order_type output_order;
extern int n_variable_dim;
extern int *variable_dim, *raw_variable_dim;


int f_cyclic(ARG0) {
    if (mode >= 0) {
	sprintf(inv_out,cyclic(sec) ? "cyclic" : "not cyclic");
    }
    return 0;
}

/*
 * cyclic: return 0/1 if not cyclic/is cyclic in longitude
 *
 * v1.1 add gaussian (not thinned)
 * v1.2 dx has to be defined, added mercator
 * v1.3 added staggered -> not cyclic
 * v1.4 add reduced Gaussian
 */

int cyclic(unsigned char **sec) {
    int grid_template, res, scan, flag_3_3, no_dx, basic_ang, sub_ang, i;
    unsigned int nx_, ny_, max_nx, nx_last;

    unsigned int npnts;
    unsigned char *gds;
    double dlon, units, lon1, lon2;

    get_nxny_(sec, &nx_, &ny_, &npnts, &res, &scan);
    if (GDS_Scan_staggered(scan)) return 0;

    grid_template = code_table_3_1(sec);
    gds = sec[3];

    if (nx_ == 0) {		// thinned or reduced grids
        if (grid_template == 40) {	// reduced Gaussian grid
	    if (ny_ == 0) fatal_error("cyclic: bad grid definition","");

	    max_nx = variable_dim[0];
	    for (i = 1; i < n_variable_dim; i++) {
		max_nx = variable_dim[i] > max_nx ? variable_dim[i] : max_nx;
	    }
	    nx_last = variable_dim[n_variable_dim-1];

            basic_ang = GDS_Gaussian_basic_ang(gds);
            sub_ang = GDS_Gaussian_sub_ang(gds);
            units = basic_ang == 0 ?  0.000001 : (float) basic_ang / (float) sub_ang;

            lon1 = units * GDS_Gaussian_lon1(gds);
            lon2 = units * GDS_Gaussian_lon2(gds);
	    lon2 = (scan & 128) ?  lon1 - lon2 : lon2 - lon1;
	    if (lon2 < 0) lon2 += 360.0;
	    /* EMCWF version */
	    if (fabs(lon2 * max_nx / (max_nx-1.0) - 360.0) < TOTAL_ERROR) {
		return 1;
	    }
	    if (fabs(lon2 * nx_last / (nx_last-1.0) - 360.0) < TOTAL_ERROR) {
		return 1;
	    }
	    return 1;
	}
        return 0;
    }

    flag_3_3 = flag_table_3_3(sec);
    no_dx =  0;
    if (flag_3_3 != -1) {
        if ((flag_3_3 & 0x20) == 0) no_dx = 1;
    }
    if (no_dx) return 0;

    if (grid_template == 0) {
        basic_ang = GDS_LatLon_basic_ang(gds);
        sub_ang = GDS_LatLon_sub_ang(gds);
        units = basic_ang == 0 ?  0.000001 : (double) basic_ang / (double) sub_ang;

	/* dlon has to be defined */
        dlon = units * GDS_LatLon_dlon(gds);
	return (fabs(nx_*dlon-360.0) < ERROR);
    }
    if (grid_template == 10) {
	if (output_order != wesn) return 0;		// only works with we:sn order
	lon1 = GDS_Mercator_lon1(gds);
	lon2 = GDS_Mercator_lon2(gds);
	if (lon2 < lon1) lon2 += 360.0;
	dlon = (lon2-lon1)*nx_/(nx_-1.0);
        return (fabs(dlon-360.0) < TOTAL_ERROR);
    }

    if (grid_template == 40) {

        basic_ang = GDS_Gaussian_basic_ang(gds);
        sub_ang = GDS_Gaussian_sub_ang(gds);
        units = basic_ang == 0 ?  0.000001 : (double) basic_ang / (double) sub_ang;

	/* dlon has to be defined */
        dlon = units * GDS_Gaussian_dlon(gds);
        return (fabs(nx_*dlon-360.0) < ERROR);
    }

    return 0;
}
