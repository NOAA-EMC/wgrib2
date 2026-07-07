/** @file
 * @brief For grids of the form grid_val(NX,NY), longitude(NX,NY) and latitude(NX,NY), 
 * print out i, j, latitude(i,j), longitude(i,j).
 * @author Public Domain: Wesley Ebisuzaki, John Howard @date 2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Append grib file flag. */
extern int file_append;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Current output order type. */
extern enum output_order_type output_order;

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/*
 * HEADER:100:gridout:output:1:text file with grid: i j lat lon (1st record)
 */

/**
 * Many grib message contain grids of the following form: grid_val(NX,NY), longitude(NX,NY) 
 * and latitude(NX,NY). Common exceptions are spectral data, thinned grids and staggered grids. 
 * If the grib message is of the first form, then you can use the -gribout option to print out 
 * i, j, latitude(i,j), longitude(i,j).
 * 
 * ## Usage
 * -gridout FILE
 * 
 * FILE is an output of the command.
 * 
 * This will print ((i, j, lat(i,j), lon(i,j), i=1,nx), j=1,ny) using the format "%10i,%10i, %.3f, %.3f\n"
 * if the grid is of the form: grid_val(nx,ny), lat(nx,ny), lon(nx,ny).
 * 
 * FILE will be a CSV file with the latitudes and longitudes of the grid points.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2014
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
