/** @file
 * @brief Routine for determining whether grid is cyclic. Needed to determine whether
 * a grid has an E or W boundary.
 * @author Public Domain: Wesley Ebisuzaki @date 2009
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Cyclic.c                 10/2024 Public Domain  Wesley Ebisuzaki
 *
 * routine for determining whether grid is cyclic
 *  covers the entire longitude band evenly
 *
 * only applies to lat-lon, Mercator and Gaussian grids
 *  any other grids?
 *
 * problem is made more difficult because NCEP grids can
 *   1. have millidegree precision because they were converted from grib1
 *   2. have less than microdegree precision because they often use single precision
 *        for lat/lon calculations (centi-degrees in one case).
 *
 *   
 * cyclic is needed to determine whether a grid has a E or W boundary
 */


/* NCEP dlat/dlon are only good to milli-degrees because
   they are converted from grib1 */

/** Error tolerance for cyclic grid checks. */
#define ERROR (0.001 * nx_)

/** Total error for cyclic grid checks. */
#define TOTAL_ERROR 0.001

/*
 * HEADER:-1:cyclic:inv:0:is grid cyclic? (not for thinned grids)
 */

/** Output order type. */
extern enum output_order_type output_order;

/** Number of variable dimensions. */
extern int n_variable_dim;

/** Pointer to variable dimension array. */
extern int *variable_dim;

/** Pointer to raw variable dimension array. */
extern int *raw_variable_dim;

/** 
 * Checks if domain is cyclic and prints the result. Covers the entire longitude band evenly.
 * 
 * This option is needed to determine whether a grid has an E or W boundary.
 * 
 * For example, the large domain goes from 0E to 359W by steps of 1 degree. Now there 
 * is a request for a subdomain that goes from 350E (left) to 10E (right). Knowing 
 * that the domain is cyclic is useful in this case. The -small_grib option calls 
 * the cyclic code. 
 * 
 * The -cyclic option will only detect cyclic grids that are either lat-lon, Gaussian 
 * or Mercator. All other grids will return a not-cyclic response. In addition, 
 * staggered and thinned grids will also return a not-cyclic response. The test for 
 * cyclic is a bit loose because NCEP grib2 files often have the lon(gitude) and 
 * dlon(gitude) to the nearest millidegree rather than the nearest microdegree. 
 * (Reason 1: grib1 only stored the lon and dlon to the nearest millidegree. Reason 2: 
 * most NCEP codes use single precision for the longitudes and latitudes.) 
 * 
 * ## Usage
 * -cyclic
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * @code{.sh}
 * $ wgrib2 gdt140.g2 -cyclic -nl -grid
 * 1:0:not cyclic:
 * :grid_template=140:winds(N/S):
 * 	    Lambert Azimuthal Equal Area grid: (1050 x 1050) input WE:NS output WE:SN res 48
 * 	    Lat1 18.876988 Lon1 225.000000 Cen Lon 0.000000 Std Par 90.000000
 * 	    Dx 10000.000000 m Dy 10000.000000 m mode 48
 * @endcode 
 * 
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180 -cyclic -nl -grid
 * 1:0:cyclic:
 * :grid_template=0:winds(N/S):
 * 	    lat-lon grid:(360 x 181) units 1e-06 input WE:NS output WE:SN res 48
 * 	    lat 90.000000 to -90.000000 by 1.000000
 * 	    lon 0.000000 to 359.000000 by 1.000000 #points=65160
 * @endcode 
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int f_cyclic(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,cyclic(sec) ? "cyclic" : "not cyclic");
    }
    return 0;
}

/**
 * Checks if the grid is cyclic. 
 * 
 * @param sec Pointer to the section array.
 * 
 * @return 1 if the grid is cyclic in longitude, 0 otherwise.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2009 | W. Ebisuzaki | Initial
 * 12/2011 | W. Ebisuzaki | Add Gaussian (not thinned)
 * 9/2013 | W. Ebisuzaki | dx has to be defined, added mercator
 * 11/2015 | W. Ebisuzaki | added staggered -> not cyclic
 * 2/2017 | W. Ebisuzaki | add reduced Gaussian
 * 
 * @author Wesley Ebisuzaki @date 2009
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
