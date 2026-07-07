/** @file
 * @brief Read latitude and longitude data from GRIB file.
 * @author Public Domain: Wesley Ebisuzaki @date 2/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag.  */
extern int decode;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Message number. */
extern int msg_no;

/** Current geolocation type. */
extern enum geolocation_type geolocation;

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

/**
 * Read latitude and longitude data from GRIB file. 
 * 
 * When wgrib2 processes a field, it often calculates the longitude and latitude (location) of 
 * each grid point. Wgrib2 doesn't do the calculation when one of the following conditions are 
 * true: 
 * 
 * 1. the locations of the grid points are not needed by any of the options
 * 2. the grid is the same as the previously processed grib message
 * 3. wgrib2 does not know how to calculate the locations
 * 
 * For some grids, wgrib2 does not know how to calculate the grid locations and the grid locations 
 * are available from the center in the form of grib files. Then you can use the -grid_def to 
 * add the grid locations for wgrib2 processing. The option, -grid_def checks to see if the grib 
 * message is a longitude or latitude. If it is, the longitude or latitude is associated with the 
 * longitude or latitude of the grid points. Because of "2", wgrib2 will use these longitudes or 
 * latitudes for the following fields until wgrib2 encountours a different grid. 
 * 
 * Wgrib2 v3.0.0+ can get associated latitudes and longitudes using -rpn and the "sto_lat" and 
 * "sto_lon" options. You can import double precision lat and lons using -import_lonlat. 
 * 
 * ## Precision of the latitudes and longitudes
 * By default, grib2 stores angles to the millionth of a degree. This requires the angles to be 
 * stored in double precision. Reading the latitudes and longitudes in grib format could be done 
 * in double precision but the current decoder is limited to 25 bits which is basically single 
 * precision. If you need double precision lat and lon values, use -import_lonlat. 
 * 
 * ## Usage
 * -grib_def
 * 
 * Will alter the latitude or longitudes associated with the grid points when the variable is a 
 * latitude or longitude field.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 2/2008
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
        if (strcmp("LAUV",name) == 0 || strcmp("LAPP",name) == 0 || strcmp("NLAT",name) == 0 || strcmp("GEOLAT",name) == 0) {
            if (lat != NULL) free(lat);
            lat = (double *) malloc(sizeof(double) * (size_t) ndata);
            if (lat == NULL) fatal_error_i("memory allocation error in grid_def #lat=%d", (int) ndata);
            for (i = 0; i < ndata; i++) lat[i] = data[i];
            geolocation = external;
        }
        if (strcmp("LOUV",name) == 0 || strcmp("LOPP",name) == 0 || strcmp("ELON",name) == 0 || strcmp("GEOLON",name) == 0) {
            if (lon != NULL) free(lon);
            lon = (double *) malloc(sizeof(double) * (size_t) ndata);
            if (lon == NULL) fatal_error_i("memory allocation error in grid_def #lon=%d", (int) ndata);
            for (i = 0; i < ndata; i++) lon[i] = data[i];
            geolocation = external;
        }
    }
    return 0;
}
