#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int decode;
extern double *lat, *lon;
extern int msg_no;

/*
 * HEADER:100:grid_def:misc:0:read lon and lat data from grib file -- experimental
 */

/* some grids dont have a built-in lat-lon definition
 *
 * this code allows the grib file to define the lat-lon
 *
 * rules: records 1 and 2 must have the lat/lon fields
 *        the grid definition must be consistent with the grib file
 *
 * public domain 2/2008 Wesley Ebisuzaki
 */


int f_grid_def(ARG0) {
    unsigned int i;
    char name[NAMELEN];

    if (mode == -1) {
	decode = 1;
	return 0;
    }
    if (mode < 0) return 0;
    if (data != NULL) {
        getName(sec,  mode, NULL, name, NULL, NULL);
        if (strcmp("LAUV",name) == 0 || strcmp("LAPP",name) == 0 || strcmp("NLAT",name) == 0) {
            if (lat != NULL) free(lat);
	    lat = (double *) malloc(sizeof(double) * (size_t) ndata);
	    if (lat == NULL) fatal_error_i("memory allocation error in grid_def #lat=%d", (int) ndata);
	    for (i = 0; i < ndata; i++) lat[i] = data[i];
        }
        if (strcmp("LOUV",name) == 0 || strcmp("LOPP",name) == 0 || strcmp("ELON",name) == 0) {
            if (lon != NULL) free(lon);
	    lon = (double *) malloc(sizeof(double) * (size_t) ndata);
	    if (lon == NULL) fatal_error_i("memory allocation error in grid_def #lon=%d", (int) ndata);
	    for (i = 0; i < ndata; i++) lon[i] = data[i];
        }
    }
    return 0;
}
