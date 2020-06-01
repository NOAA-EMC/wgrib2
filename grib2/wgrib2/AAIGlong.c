#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * AAIGlong.c - converts lat-lon file to ArcInfo ASCII grid file
 *   each grid is written into its own file
 *
 * copied from AAIG.c 10/2015 Public Domain Wesley Ebisuzaki
 *     filename convention for *.asc files is too simple for modern grib2 files
 *     use new filename conventions
 *
 */


extern int decode, latlon;
extern double *lat, *lon;
extern unsigned int nx_, ny_;
extern enum output_order_type output_order, output_order_wanted;

/*
 * HEADER:100:AAIGlong:output:0:writes Ascii ArcInfo Grid file, lat-lon grid only long-name *.asc (alpha)
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
