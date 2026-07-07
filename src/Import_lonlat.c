/** @file
 * @brief Routine to use lat-lon data from external file.
 * @author Public Domain: Wesley Ebisuzaki @date 2/2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * import_latlon
 *
 *  routine to use lat-lon data from external file
 *
 * 2/2020: Public Domain: Wesley Ebisuzaki
  *
 */

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Decode grib file flag. */
extern int decode;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Current geolocation type. */
extern enum geolocation_type geolocation;

/*
 * HEADER:100:import_lonlat:misc:1:read lon-lat data from binary file
 */

/**
 * Read lat-lon data from binary file.
 * 
 * ## Reading longitudes and latitudes to a file
 * By default grib2 saves the latitude and longitude in micro-degrees. This is more precision than the standard single-precision floating point variable can hold. So wgrib2 uses double precision variables for its angles. However, the -rpn facility is single precision, so it cannot be use for angles without losing precision. The -export_lonlat and -import_lonlat options allow you to write and read double-precision longitude and latitudes. 
 * 
 * ## File format
 * The -import_lonlat option read the longitudes and latitudes in the following format. 
 * 
 * 8 bytes:                        'wgrib2ll'       text
 * (sizeof unsigned int) bytes      ndata           unsigned integer with number of grid points
 * (sizeof unsigned int) bytes      0               unsigned integer with value of zero
 * ndata*(sizeof double)            longitudes      ndata values of double precision longitudes
 * ndata*(sizeof double)            latitudes       ndata values of double precision latitudes
 * 
 * (sizeof unsigned int) is usually 4.  By wgrib2 requirements, the value must be 4 or greater.
 * (sizeof double) is usually 8.
 * 
 * Prior to wgrib2 v3.1.2, -import_lonlat would always read the first record of the input file. For wgrib2 v3.1.2+, -import_lonlat would read the records sequentially. If there was an EOF, then read would start at the beginning of the file. Consequently old behavior mimicked for files with only one record. 
 * 
 * ## Caution
 * The -import_lonlat will change the internal values of the longitudes and latitudes. However, wgrib2 will still use the original grid organization. For example, the grib message was a lat-long grid, and replaced the values for a polar stereographic grid. Some parts of wgrib2 may still assume the grid is a lat-lon grid. The -import_lonlat will be more useful for unstructured grids like GDT 3.101 (General Unstructured Grid). GDT 3.101 does not include geolocation, and -import_lonlat can add the geolocation. Consequently the nearest neighbor interpolation will then work. 
 * 
 * ## Usage
 * -import_lonlat FILE
 * 
 * FILE = file that has special format with longitude and latitude values.
 * 
 * The longitude and latitudes replace the internal values.
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
 * @author Wesley Ebisuzaki @date 2/2020
 */
int f_import_lonlat(ARG1) {
    struct seq_file *save;
    unsigned int zero, ndata_ll;
    char id[8];
    int j;
    unsigned int i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("export_latlon: memory allocation","");
        if (fopen_file(save, arg1, "r") != 0) {
            free(save);
            fatal_error("import_lonlat: Could not open %s", arg1);
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
        if (lat == NULL) {
            lat = (double *) malloc(sizeof(double) * (size_t) ndata);
            if (lat == NULL) fatal_error_i("memory allocation error in grid_def #lat=%d", (int) ndata);
        }
        if (lon == NULL) {
            lon = (double *) malloc(sizeof(double) * (size_t) ndata);
            if (lon == NULL) fatal_error_i("memory allocation error in grid_def #lat=%d", (int) ndata);
        }

        /* file:  'wgrib2ll ' // ndata // 0 // lon // lat */

        j = fread_file(id, sizeof(char), (size_t) 8, save);
        /* 6/2022 if EOF try from beginning of file */
        if (j == 0) {
            if (fseek_file(save, 0l, SEEK_SET) != 0) fatal_error("import_lonlat: fseek_file error","");
            j = fread_file(id, sizeof(char), (size_t) 8, save);
        }
        if (j == 0) fatal_error("import_lonlat: empty or missing file","");
        if (j != 8) fatal_error("import_lonlat: missing header","");
        if (strncmp(id,"wgrib2ll",8) != 0) fatal_error("import_lonlat: bad header","");
        if (mode > 0) {
            fprintf(stderr, "header=");
            for (i = 0; i < 8; i++) fprintf(stderr,"%c", id[i]);
            fprintf(stderr, "\n");
        }
        if (fread_file(&ndata_ll, sizeof(unsigned int) , (size_t) 1, save) != 1) fatal_error("import_lonlat: ndata","");
        if (mode > 0) fprintf(stderr, "import_lonlat: ndata=%u\n", ndata_ll);
        if (fread_file(&zero, sizeof(unsigned int), (size_t) 1, save) != 1) fatal_error("import_lonlat: bad header","");
        if (zero != 0) fatal_error("import_lonlat: zero was expected","");
        if (ndata != ndata_ll) fatal_error("import_lonlat: grid size does not match","");
        if (fread_file(&lon[0], sizeof(double), (size_t) ndata, save) != ndata) fatal_error("import_lonat:lon read","");
        if (fread_file(&lat[0], sizeof(double), (size_t) ndata, save) != ndata) fatal_error("import_lonat:lat read","");
        geolocation = external;
    }
    return 0;
}
