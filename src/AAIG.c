/** @file
 * @brief Converts lat-lon file to ArcInfo ASCII grid file (alpha).
 * @author Public Domain: Wesley Ebisuzaki @date 07/2008
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 07/2008 | W. Ebisuzaki | v0.9 - Initial
 * 10/2010 | H. Peifer | v0.99 - bug fix
 * 07/2016 | M. Schwarb | v1.0 - allow dx != dy
 * 01/2020 | M. Schwarb | v1.1 - output filename has ensemble info
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
 * HEADER:100:AAIG:output:0:writes Ascii ArcInfo Grid file, lat-lon grid only (alpha)
 */

/**
 * Converts a lat-lon file and writes the data into a Ascii ArcInfo Grid file. 
 * This option is experimental and only supports equally spaced lat-lon grids.
 * 
 * Each field is written to a different file (*.asc) which is written to the current directory.
 * 
 * ## Usage
 * -AAIG
 * 
 * ## File name convention for *asc output
 * 
 * <pre>
 * NAME = grib name (-var), ex. TEMP, HGT
 * LEVEL = level, ex. surface, 2_m_above_ground, 500_mb
 * RT = reference time YYYYMMDDHH
 * VT = verification time (end_ft) YYYYMMDDHH
 *
 * If RT is the same as VT:
 *      output = NAME.LEVEL.RT.asc
 * If RT is different than VT:
 *      output = NAME.LEVEL.RT.VT.asc
 * </pre>
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, error code otherwise.
 *
 * @note The above file name convention works for a simple GFS forecast. However, life quickly gets 
 * more complicated and a new file name convention was needed (-AAIGlong).
 * 
 * ## Example: 
 * @code{.sh}
 * $ wgrib2 in_file -match ':HGT:400 mb:' -AAIG
 * @endcode
 *
 * The above line converts all the 400 mb HGT fields into an arcinfo ascii grid file.
 * 
 * @author Wesley Ebisuzaki @date 07/2008
 */
int f_AAIG(ARG0) {

    double cellsize, dlon, dlat;
    char *save_inv_out, level[STRING_SIZE], file[STRING_SIZE*3], name[STRING_SIZE];
    char ensinfo[STRING_SIZE];
    size_t i, j;
    struct full_date date_ref, date_verf;
    FILE *out;

    if (mode == -1) {
        decode = latlon = 1;
        return 0;
    }
    if (mode < 0) return 0;

    if (lat == NULL || lon == NULL) {
        fprintf(stderr,"AAIG does nothing, no lat-lon information\n");
        return 0;
    }
    if (output_order != wesn) {
        fprintf(stderr,"AAIG does nothing, not in we:sn order\n");
        return 0;
    }
    if (code_table_3_1(sec) != 0) {	// check for lat/lon grid
        fprintf(stderr,"AAIG does nothing, not lat-lon grid\n");
        return 0;
    }
    if (nx_ == 0 || ny_ == 0) {
        fprintf(stderr,"AAIG does nothing, found thinned lat-lon grid\n");
        return 0;
    }
    cellsize = dlon = lon[1] - lon[0];
    dlat = lat[nx_] - lat[0];
    if ( fabs(dlat - dlon) > 0.0001*dlon) cellsize = 0.0;

    name[0] = level[0] = ensinfo[0] = 0;

    if (getExtName(sec, mode, NULL, name, NULL, NULL) != 0) {
        fatal_error("AAIG does nothing, no name","");
        return 0;
    }

    f_lev(call_ARG0(level,NULL));
    f_ens(call_ARG0(ensinfo,NULL));
    if ((i = strlen(ensinfo)) > 0) {
        if (strlen(level)+i+1  < STRING_SIZE) {
            strncat(level,".",2);
            strncat(level,ensinfo,STRING_SIZE-strlen(level)-1);
        }
    }

    save_inv_out = level;
    while (*save_inv_out) {
        if (*save_inv_out == ' ') *save_inv_out = '_';
	    save_inv_out++;
    }

    Ref_time(sec, &date_ref);

    if (Verf_time(sec, &date_verf) != 0) {
        fprintf(stderr,"f_AAIG no verf time\n");
        date_verf = date_ref;
    }
    if (date_verf.year == date_ref.year && date_verf.month == date_ref.month && 
        date_verf.day == date_ref.day && date_verf.hour == date_ref.hour) {
        sprintf(file,"%s.%s.%4.4d%2.2d%2.2d%2.2d.asc",name,level,date_ref.year,date_ref.month,date_ref.day,date_ref.hour);
    } 
    else {
        sprintf(file,"%s.%s.%4.4d%2.2d%2.2d%2.2d_%4.4d%2.2d%2.2d%2.2d.asc",
        name,level, date_ref.year,date_ref.month,date_ref.day,date_ref.hour,
            date_verf.year,date_verf.month,date_verf.day,date_verf.hour);
    }

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
            fprintf(out,"%f\n", data[i + (ny_ - 1 - j)*nx_]);
        }
    }
    ffclose(out);
    return 0;
}
