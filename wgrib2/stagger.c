#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * stagger.c Public domain 1/2014 Wesley Ebisuzaki
 *
 *
 * usually x[0] = y[0] = 0.0
 *
 *  usually x[] and y[] are integers (except when grids are shiffed +/- 1/2)
 *  
 *  normally:
 *    x = x[]*dx + x_0,   note: for gctpc x_0 is not needed
 *    y = y[]*dy + y_0,   note: for gctpc y_0 is not needed
 *    x_0 and y_0 are the x and y of the first grid point (raw order)
 *
 *  version: proposal 5
 */

extern double *lat, *lon;
extern enum output_order_type output_order;
extern int scan;

/*
 * stagger fills x[] and y[], lo1 and la1 have X==0 and Y==0
 *
 * assumed_npnts is number if grid points that the calling program thinks is right
 *  this is for error checking.  use -1 if don't know
 * 
 * to a grid transform:
 *   setup grid transform (proj4 library for example(
 *   call stagger() to get the x() and y() values of the grid
 *     transform x() and y() to lon() and lat()
 *
 * like many programs, stagger requires grid to be on we:sn order
 */

int stagger(unsigned char **sec, unsigned int assumed_npnts, double *x, double *y) {
    int nx, ny, res, scan;
    unsigned int npnts;
    int nnx, nx_even, nx_odd, nx2;
    double x0, y0, dx_offset, dx_offset_even, dx_offset_odd, dy_offset;
    unsigned int i, ix, iy, n;

    int reduced_grid, dx_off_odd, dx_off_even, dy_off;
    int dx, dy, even;

    get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
    if (scan == -1) return 1;
    if (output_order != wesn) return 1;
    if (nx < 1 || ny < 1) return 1;

    /* get stagger bits */
    dx_off_odd = ((unsigned int) scan >> 3) & 1;
    dx_off_even = ((unsigned int) scan >> 2) & 1;
    dy_off = ((unsigned int) scan >> 1) & 1;
    reduced_grid = (unsigned int) scan & 1;

    dx =  (scan & 128) ? -1 : 1;
    dy =  (scan & 64) ? 1 : -1;

    if (reduced_grid && dy_off) ny--;
 
    if (dy < 0 && ((ny % 2) == 0)) { // swap even and odd rows if ns to sn and even number of rows
        i = dx_off_odd;
	dx_off_odd = dx_off_even;
	dx_off_even = i;
    }

    dx_offset_odd  = reduced_grid ? 0.5 * dx_off_odd  : 0.5 * dx_off_odd  * dx;
    dx_offset_even = reduced_grid ? 0.5 * dx_off_even : 0.5 * dx_off_even * dx;
    dy_offset = reduced_grid ? 0.5 * dy_off : 0.5 * dy_off * dy;

    nx_odd  = nx - (dx_off_odd  & reduced_grid);
    nx_even = nx - (dx_off_even & reduced_grid);
    nx2 = nx_odd + nx_even;

//fprintf(stderr, "stagger: dx_off_odd %lf dx_off_even %lf dy_off %lf  reduced_grid %d nx=%d %d\n", 
//    dx_offset_odd, dx_offset_even, dy_offset, reduced_grid, nx_odd,nx_even);
//fprintf(stderr,"dx_off_odd %d reduced_grid %d, and %d\n", dx_off_odd , reduced_grid, dx_off_odd & reduced_grid);
//fprintf(stderr,"dx_off_even %d reduced_grid %d, and %d\n", dx_off_even , reduced_grid, dx_off_even & reduced_grid);

    // number of grid points
    n = (ny/2)*nx_even + ((ny+1)/2)*nx_odd;

#ifdef WMO_VALIDATION
    if (code_table_3_1(sec) == 60) {		/* cubed sphere */
	if (GDS_Gnom_tile(sec[3]) == 0) {	/* global - 6 faces */
	    n = n*6;
	}
	fprintf(stderr,"stagger: n=%d assume %d sec3 %d\n", n,assumed_npnts, GB2_Sec3_npts(sec));
    }
#endif

    // check to number of points
    if (assumed_npnts != n) 
	fatal_error_ii("stagger: program error think npnts=%d assumed npnts=%d",n, (int) assumed_npnts);
    if (n != GB2_Sec3_npts(sec)) 
	fatal_error_ii("stagger: program error think npnts=%d, Sec3 gives %d",n, GB2_Sec3_npts(sec));

    if (x == NULL || y == NULL) return 1;

    /* return X[] and Y[] relative to the first grid point but on a we:sn grid */

    x0 = (dx > 0) ? 0.0 : 1.0 - (double) nx;
    y0 = (dy > 0) ? 0.0 : 1.0 - (double) ny;

#pragma omp parallel for private(ix,iy,even,i,dx_offset, nnx)
    for (iy = 0; iy < ny; iy++) {
	// even = iy % 2;		// first row is odd .. iy % 2 == 0
	even = (iy & 1);		// first row is odd
	i = even ?  nx2*(iy >> 1) + nx_odd : nx2*(iy >> 1);
	nnx = even ? nx_even : nx_odd;
	dx_offset = even ? dx_offset_even : dx_offset_odd;
	for (ix = 0; ix < nnx; ix++) {
	    x[i + ix] = x0 + dx_offset + ix;
	    y[i + ix] = y0 + dy_offset + iy;
	}
    }

#ifdef WMO_VALIDATION
    if (code_table_3_1(sec) == 60) {			/* cubed sphere */
	if (GDS_Gnom_tile(sec[3]) == 0) {		/* global - 6 faces */
	    /* calculated X, Y for one face, duplicate for all 8 faces */
	    n = n / 6;
#pragma omp parallel for private(i)
	    for (i = 0; i < n; i++) {
		x[i+n] = x[i+2*n] = x[i+3*n] = x[i+4*n] = x[i+5*n] = x[i];
		y[i+n] = y[i+2*n] = y[i+3*n] = y[i+4*n] = y[i+5*n] = y[i];
	    }
	}
    }
#endif

    return 0;
}
