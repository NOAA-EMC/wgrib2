#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:400:hybrid:inv:0:shows vertical coordinate parameters from Sec4
 */

int f_hybrid(ARG0) {
    int pdtsize, n, secsize, k;
    if (mode >= 0) {
	pdtsize =  prod_def_temp_size(sec);     // size of PDT with no extra vert coordinates
	secsize = GB2_Sec4_size(sec);		// size of PDT + vert coordinatese + extras
        n = uint2(&(sec[4][5]));                // number of vert coordinate values
	if (n % 2 != 0) fatal_error_i("vertical coordinate parameters should be in pairs, n=%d", n);
	if (n == 0) return 0;
	if (pdtsize + 4*n > secsize) 
	    fatal_error("not enough space allocated for vertical coordinate data","");
	sprintf(inv_out,"Hybrid levels=%d", n/2);
	inv_out += strlen(inv_out);
	for (k = 1; k <= n/2; k++) {
	    sprintf(inv_out," %d=(%9.6f,%9.6f)", k, (double) ieee2flt(sec[4]+pdtsize+k*8-8),
		(double) ieee2flt(sec[4]+pdtsize+k*8-4));
	    inv_out += strlen(inv_out);
	}
    }
    return 0;
}
