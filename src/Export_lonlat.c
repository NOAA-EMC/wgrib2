/** @file
 * @brief Export latitude and longitude data to an external binary file.
 * @author Public Domain: Wesley Ebisuzaki @date 7/2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Decode grib file flag */
extern int decode;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/*
 * HEADER:100:export_lonlat:misc:1:save lon-lat data in binary file
 */

/**
 * By default grib2 saves the latitude and longitude in micro-degrees. This is more 
 * precision than the standard single-precision floating point variable can hold. So 
 * wgrib2 uses double precision variables for its angles. However, the -rpn facility 
 * is single precision, so it cannot be use for angles without losing precision. The 
 * -export_lonlat option allows you to write double-precision longitude and latitudes. 
 * 
 * This option writes the longitudes and latitudes in the following format:
 *
 * 8 bytes:                        'wgrib2ll'       text
 * (sizeof unsigned int) bytes      ndata           unsigned integer with number of grid points
 * (sizeof unsigned int) bytes      0               unsigned integer with value of zero
 * ndata*(sizeof double)            longitudes      ndata values of double precision longitudes
 * ndata*(sizeof double)            latitudes       ndata values of double precision latitudes
 *
 * (sizeof unsigned int) is usually 4. By wgrib2 requirements, the value must be 4 or greater.
 * (sizeof double) is usually 8.
 *
 * ## Usage
 * -export_lonlat FILE
 * 
 * FILE = file that is written with the binary data
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code on failure
 * 
 * ## Example 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 7/2019
 */
int f_export_lonlat(ARG1) {
    struct seq_file *save;
    unsigned int zero;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("export_latlon: memory allocation","");
        if (fopen_file(save, arg1, "w") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        latlon = 1;
        /* need to decode to get value of ndata */
        decode = 1;
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
    }
    else if (mode >= 0) {
        save = *local;
        if (lat != NULL && lon != NULL) {
            /* output file:  'wgrib2ll ' // ndata // 0 // lon // lat */
            fwrite_file("wgrib2ll", 1, 8, save);
            fwrite_file(&ndata, sizeof(unsigned int), (size_t) 1, save);
            zero = 0;
            fwrite_file(&zero, sizeof(unsigned int), (size_t) 1, save);
            fwrite_file(&(lon[0]), sizeof(double), (size_t) ndata, save);
            fwrite_file(&(lat[0]), sizeof(double), (size_t) ndata, save);
        }
    }
    return 0;
}
