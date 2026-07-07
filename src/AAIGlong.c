/** @file
 * @brief Converts lat-lon file to ArcInfo ASCII grid file.
 * @author Public Domain: Wesley Ebisuzaki @date 10/2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag  */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Number of grid points in x-direction. */
extern unsigned int nx_;

/** Number of grid points in y-direction. */
extern unsigned int ny_;

/** Current output order type. */
extern enum output_order_type output_order;

/** Desired output order type. */
extern enum output_order_type output_order_wanted;

/*
 * HEADER:100:AAIGlong:output:0:writes Ascii ArcInfo Grid file, lat-lon grid only long-name *.asc (alpha)
 */

/**
 * Converts a lat-lon file and writes the data into a Ascii ArcInfo Grid file. This option
 * is similar to the -AAIG option, but uses a new file name convention for the output files.
 * 
 * This option is experimental and only supports equally spaced lat-lon grids. You can use the
 * -new_grid option to create an equally spaced lat-lon grid.
 * 
 * Each field is written to a different file (*.asc) which is written to the current directory.
 * 
 * ## Usage
 * -AAIGlong
 * 
 * ## File name convention for *asc output
 * 
 * <pre>
 * NAME = (wgrib2 -S)
 * NAME=(wgrib2 -S)
 * remove (message number)[.submessage number]:(byte location): 
 * remove trailing semicolon if any
 * replace "/" by " DIV "
 * replace "\" by " BS "
 * replace ":" by "_"
 * replace "'" by " Q "
 * replace '"' by " Q "
 * </pre>
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, error code otherwise.
 *
 * @note The (wgrib2 -S) output can change between different versions of wgrib2. The grib table 
 * can be updated. The new metadata can be added to the inventory in order to uniquely identify 
 * fields. In rare cases, the format of the metadata has been updated. If you need to format of 
 * the NAME to be unchanging, please freeze the version of wgrib2.
 * 
 * ## Example: 
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180 -match "HGT:500 mb" -AAIGlong
 * raster file: D=20090605000000_HGT_500 mb_180 hour fcst_ENS=+19.asc
 * 9:280952:d=2009060500:HGT:500 mb:180 hour fcst:ENS=+19
 * $ ls *asc
 * D=20090605000000_HGT_500 mb_180 hour fcst_ENS=+19.asc
 * @endcode
 * 
 * The above line converts all the 500 mb HGT fields into an arcinfo ascii grid file.
 * 
 * @author Wesley Ebisuzaki @date 10/2015
 */
int f_AAIGlong(ARG0) {

    size_t i, j;
    double cellsize, dlon, dlat;
    char file[STRING_SIZE], name[STRING_SIZE];
    FILE *out;

    if (mode == -1) {
        decode = latlon = 1;
        return 0;
    }
    if (mode < 0) return 0;

    if (lat == NULL || lon == NULL) {
        fprintf(stderr,"AAIGlong does nothing, no lat-lon information\n");
        return 0;
    }
    if (output_order != wesn) {
        fprintf(stderr,"AAIGlong does nothing, not in we:sn order\n");
        return 0;
    }
    if (code_table_3_1(sec) != 0) {	// check grid template
        fprintf(stderr,"AAIGlong does nothing, not lat-lon grid\n");
        return 0;
    }
    if (nx_ == 0 || ny_ == 0) {
        fprintf(stderr,"AAIGlong does nothing, found thinned lat-lon grid\n");
        return 0;
    }

    cellsize = dlon = lon[1] - lon[0];
    dlat = lat[nx_] - lat[0];
    if ( fabs(dlat - dlon) > 0.0001*dlon) cellsize = 0.0;

    f_S(call_ARG0(name,NULL));
    name[STRING_SIZE-1] = 0;

    /* get rid of trailing semicolon */
    i = strlen(name);
    if (name[i-1] == ':') name[i-1] = 0;

    /* get rid of bad characters for a filename */

    j = i = 0;
    while (name[i]) {
        if (name[i] == '/') {
            file[j++] = ' ';
            file[j++] = 'D';
            file[j++] = 'I';
            file[j++] = 'V';
            file[j++] = ' ';
        }
        else if (name[i] == '\\') {
            file[j++] = ' ';
            file[j++] = 'B';
            file[j++] = 'S';
            file[j++] = ' ';
        }
        else if (name[i] == ':') {
            file[j++] = '_';
        }
        else if (name[i] == '"' || name[i] == '\'') {
            file[j++] = ' ';
            file[j++] = 'Q';
            file[j++] = ' ';
        }
        else file[j++] = name[i];
        i++;
    }
    file[j++] = '.';
    file[j++] = 'a';
    file[j++] = 's';
    file[j++] = 'c';
    file[j] = 0;

    if ((out = ffopen(file,"w")) == NULL) {
        fprintf(stderr,"AAIG could not open raster file %s\n",file);
        return 0;
    }

    fprintf(stderr, "raster file: %s\n", file);

    fprintf(out,"ncols %u\n", nx_);
    fprintf(out,"nrows %u\n", ny_);
    fprintf(out,"xllcenter %lf\n", lon[0] > 180.0 ? lon[0]-360.0 : lon[0]);
    fprintf(out,"yllcenter %lf\n", lat[0]);
    if (cellsize > 0.0) {
        fprintf(out,"cellsize %lf\n", cellsize);
    }
    else {
        fprintf(out,"dx       %lf\n", dlon);
        fprintf(out,"dy       %lf\n", dlat);
    }
    fprintf(out,"NODATA_VALUE 9.999e20\n");
    for (j = 0; j < ny_; j++) {
        for (i = 0; i < nx_; i++) {
            fprintf(out,"%f\n", data[i+(ny_ - 1 - j)*nx_]);
        }
    }
    ffclose(out);
    return 0;
}
