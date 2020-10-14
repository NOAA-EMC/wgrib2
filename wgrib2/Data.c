#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Data.c
 *
 *  Some routines that examine the data
 *
 * 2006: Public Domain: Wesley Ebisuzaki
 * 1/2007 cleanup M. Schwarb
  *
 */

extern int decode, latlon;
extern double *lat, *lon;

#define CONV (3.14159265/180.0)
/*
 * HEADER:100:stats:inv:0:statistical summary of data values
 */

int f_stats(ARG0) {
    double sum, sum_wt, wt, t, sum_thread, sum_wt_thread, wt_thread, last_t, last_lat;
    int do_wt;
    unsigned int n, n_thread, first, i;
    float mn, mx, min_thread, max_thread;

    if (mode == -1) {
        latlon = decode = 1;
    }
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
    sum = wt = sum_wt = 0.0;

#pragma omp parallel private(min_thread, max_thread, n_thread, sum_thread, t, wt_thread, sum_wt_thread, last_t, last_lat)
    {
        min_thread = max_thread = mx;
	n_thread = 0;
	sum_thread = wt_thread = sum_wt_thread = 0.0;
        last_t = 1.0;
	last_lat = 0.0;
#pragma omp for private(i) schedule(static) nowait
        for (i = first; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
                min_thread = (min_thread > data[i]) ? data[i] : min_thread;
                max_thread = (max_thread < data[i]) ? data[i] : max_thread;
		sum_thread += data[i];
		n_thread++;
		if (do_wt) {
		    if (lat[i] == last_lat) {
			t = last_t;
		    }
		    else {
		        last_t = t = cosf((float) CONV*lat[i]);
			last_lat = lat[i];
		    }
		    wt_thread += t;
		    sum_wt_thread += data[i]*t;
		}
            }
        }

#pragma omp critical
        {
            if (min_thread < mn) mn = min_thread;
            if (max_thread > mx) mx = max_thread;
	    sum += sum_thread;
	    n += n_thread;
	    wt += wt_thread;
	    sum_wt += sum_wt_thread;
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
