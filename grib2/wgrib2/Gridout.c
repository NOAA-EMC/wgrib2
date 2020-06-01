#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
/*
 * 2014: Public Domain, Wesley Ebisuzaki, John Howard
 */
extern int file_append;
extern double *lat, *lon;
extern enum output_order_type output_order;
extern int decode, latlon;

/*
 * HEADER:100:gridout:output:1:text file with grid: i j lat lon (1st record)
 */

int f_gridout(ARG1) {
    int nx, ny, res, scan, i, j, n;
    unsigned int npnts;

    if (mode == -1) {
        if ((*local = (void *) ffopen(arg1, file_append ? "ab" : "wb")) == NULL) {
	    fatal_error("Could not open %s", arg1);
	}
	latlon = 1;
    }
    else if (mode >= 0) {
	if (lat == NULL || lon == NULL || *local == NULL) return 0;
	if (output_order != wesn && output_order != wens) return 0;

        get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
	if (nx*ny != npnts) return 0;
	
	n = 0;
	for (j=0; j<ny; j++) {
	    for ( i=0; i<nx; i++) {
		fprintf((FILE *) *local, "%10i,%10i, %.3f, %.3f\n", i+1, j+1, lat[n], lon[n]);
		n++;
	    }
	}
	ffclose((FILE *) *local);
	*local = NULL;
    }
    return 0;
}
