/** @file
 * @brief Some routines that examine the data.
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2006 | W. Ebisuzaki | Initial
 * 1/2007 | M. Schwarb | Cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "wmath.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/*
 * HEADER:100:stats:inv:0:statistical summary of data values
 */

/**
 * Writes a statistical summary of the field into the inventory.
 * 
 * Summary includes the number of defined values, undefined values, mean, minimum, 
 * and maximum values. If latitude information is available, it also computes a 
 * cosine-weighted mean.
 * 
 * ## Usage:
 * -stats
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -stats
 * 1:0:ndata=65160:undef=0:mean=83.8696:min=-428.1:max=317.8
 * @endcode
 * 
 * <pre>
 * ndata = number of grid points
 * undef = number of grid points with an undefined value
 * mean = grid point average (not area weighted)
 * min = minimum value
 * max = maximum value
 * </pre>
 * 
 * The -stats option can be combined with the -undefine option to produce statistics for a box.
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -stats
 * 1:4:ndata=10512:undef=0:mean=77.5081:min=-370:max=340.2:cos_wt_mean=97.267
 * @endcode
 * 
 * produces the global statistics. By setting grid points to undefined, we can produce the statistics for 
 * a box.
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -undefine outobx 0:90 -10:10 -stats
 * 1:4:ndata=10512:undef=10179:mean=79.8829:min=41:max=144:cos_wt_mean=79.8688
 * @endcode
 * 
 * Note, if we reverse the order of the -stats and -undefine options, we get the global mean. That is 
 * because the -stats option is excuted before the -undefine options.
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -stats -undefine out-box 0:10 -10:10
 * 1:4:ndata=10512:undef=0:mean=77.5081:min=-370:max=340.2:cos_wt_mean=97.267
 * @endcode
 * 
 * If all the data are undefined, the -stats option will produce values of zero for the the mean, min 
 * and max. The -min and -max options will yield a text string of "undefined".
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_stats(ARG0) {
    double sum, sum_wt, wt, last_coslat, last_lat;
    int do_wt;
    unsigned int n, first, i;
    float mn, mx;

    if (mode == -1) latlon = decode = 1;
    if (mode < 0) return 0;

    sum = wt = sum_wt = 0.0;

    /* find first = first defined value */
    for (first = 0; first < ndata; first++) {
        if (DEFINED_VAL(data[first])) break;
    }
    if (first >= ndata) {
        sprintf(inv_out,"ndata=%u:undef=%u:mean=%lg:min=%lg:max=%lg", ndata, ndata, sum, sum, sum);
        return 0;
    }

    mn = mx = data[first];
    do_wt = (lat != NULL);
    n = 0;
    last_lat = 0.0;
    last_coslat = 1.0;

#ifdef USE_OPENMP
#pragma omp parallel for firstprivate(last_coslat, last_lat) private(i) reduction(+:n,sum,wt,sum_wt) reduction(min:mn) reduction(max:mx)
#endif
    for (i = first; i <	ndata; i++) {
        if (DEFINED_VAL(data[i])) {
            n += 1;
            sum += data[i];
            mx = data[i] > mx ? data[i] : mx;
            mn = data[i] < mn ? data[i] : mn;
            if (do_wt) {
                if (lat[i] != last_lat) {
                    last_coslat = cosf((float)(DEG_TO_RAD*lat[i]));
                    last_lat = lat[i];
                }
                wt += last_coslat;
                sum_wt += last_coslat * data[i];
            }
        }
    }
    sum /= n;
    sprintf(inv_out,"ndata=%u:undef=%u:mean=%lg:min=%g:max=%g", ndata, ndata-n, sum, mn, mx);
    if (wt > 0) {
        sum_wt = sum_wt/wt;
        inv_out += strlen(inv_out);
        sprintf(inv_out,":cos_wt_mean=%lg", sum_wt);
    }
    return 0;
}

/*
 * HEADER:100:max:inv:0:print maximum value
 */

/**
 * Prints the maximum value of the field.
 * 
 * ## Usage:
 * -max
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -max
 * 1:0:max=317.8
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_max(ARG0) {
    float mn, mx;
    int ok;

    if (mode == -1) {
        decode = 1;
    }
    else if (mode >= 0) {
	    ok = min_max_array(data, ndata, &mn, &mx);
        if (ok == 0) sprintf(inv_out,"max=%g", (double) mx);
        else sprintf(inv_out,"max=undefined");
    }
    return 0;
}

/*
 * HEADER:100:min:inv:0:print minimum value
 */

/**
 * Prints the minimum value of the field.
 * 
 * ## Usage:
 * -min
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -min
 * 1:0:min=-428.1
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_min(ARG0) {
    float mx, mn;
    int ok;

    if (mode == -1) {
        decode = 1;
    }
    else if (mode >= 0) {
        ok = min_max_array(data, ndata, &mn, &mx);
        if (ok == 0) sprintf(inv_out,"min=%g", (double) mn);
        else sprintf(inv_out,"min=undefined");
    }
    return 0;
}
