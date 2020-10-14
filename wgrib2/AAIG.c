#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * AAIG.c - converts lat-lon file to ArcInfo ASCII grid file
 *   each grid is written into its own file
 *
 * 7/2008 v0.9: Public Domain: Wesley Ebisuzaki
 * 10/2010 v0.99: bug fix H. Peifer
 * 7/2016  v1.0  Manfred Schwarb, allow dx != dy
 */


extern int decode, latlon;
extern double *lat, *lon;
extern unsigned int nx_, ny_;
extern enum output_order_type output_order, output_order_wanted;

/*
 * HEADER:100:AAIG:output:0:writes Ascii ArcInfo Grid file, lat-lon grid only (alpha)
 */

int f_AAIG(ARG0) {

    double cellsize, dlon, dlat;
    char *save_inv_out, level[STRING_SIZE], file[STRING_SIZE], name[STRING_SIZE];
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

    *name = *level = 0;

    if (getExtName(sec, mode, NULL, name, NULL, NULL,".","_") != 0) {
        fatal_error("AAIG does nothing, no name","");
        return 0;
    }

    f_lev(call_ARG0(level,NULL));

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
