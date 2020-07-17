#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
/*
 * Lola.c
 *  creates a LOngitude LAtitude grid
 *
 * 10/2006 Public Domain,  Wesley Ebisuzaki
 * 1/2008 lat and lon changed from float to double
 * 1/2011 replaced new_GDS by GDS_change_no, WNE
 * 12/2015 4G grid cell limit except for bin output WNE
 */
extern int decode, flush_mode;
extern int file_append;
extern enum output_order_type output_order;

extern double *lat, *lon;
extern int latlon;

extern int GDS_change_no;

extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

extern int header;
extern char *text_format;
extern int text_column;

/*
 * HEADER:100:lola:output:4:lon-lat grid values X=lon0:nlon:dlon Y=lat0:nlat:dlat Z=file A=[bin|text|spread|grib]
 */

int f_lola(ARG4) {

    unsigned int i, j, k;
    unsigned int nx, ny, nxny;
    size_t long_i;
    double latitude, longitude;
    double x0,dx, y0,dy;
    unsigned char *new_sec[8];
    float *tmp, t;

    struct local_struct {
        unsigned int nlat, nlon;
        double lat0, lon0, dlat, dlon;
	struct seq_file out;
        int *iptr;
	int last_GDS_change_no;
    };
    struct local_struct *save;
    char open_mode[4];

    /* initialization phase */

    if (mode == -1) {
        decode = latlon = 1;
        if (sscanf(arg1,"%lf:%u:%lf", &x0, &nx, &dx) != 3) {
            fatal_error("lola parsing longitudes lon0:nx:dlon  %s", arg1);
        }
        if (sscanf(arg2,"%lf:%u:%lf", &y0, &ny, &dy) != 3) {
            fatal_error("lola parsing latitudes lat0:nx:dlat  %s", arg2);
        }

        if (strcmp(arg4,"spread") != 0 
                && strcmp(arg4,"text") != 0 
                && strcmp(arg4,"grib") != 0
                && strcmp(arg4,"bin") != 0) fatal_error("lola bad write mode %s", arg4);

	if (strcmp(arg4,"grib") == 0 && (dx <= 0 || dy <= 0))
		fatal_error("lola parsing, for grib dlat > 0 and dlon > 0","");

        strcpy(open_mode, file_append ? "a" : "w");
        if (strcmp(arg4,"bin") == 0 || strcmp(arg4,"grib") == 0) strcat(open_mode,"b");

        long_i= (size_t) nx * (size_t) ny;
	nxny = (unsigned int) long_i;
	if ((size_t) nxny != long_i) fatal_error("lola: grid has more than 2**32-1 grid points","");

        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("lola: memory allocation ","");
	if (x0 < 0.0) x0 += 360.0;
	if (x0 >= 360.0) x0 -= 360.0;
	if (x0 < 0.0 || x0 >= 360.0) fatal_error("lola: bad initial longitude","");
	if (nx == 0) fatal_error("lola: bad nlon 0","");

        save->nlon = nx;
        save->lon0 = x0;
        save->dlon = dx;

	if (y0 < -90.0 || y0 > 90.0) fatal_error("-lola: bad initial latitude","");
	if (ny == 0) fatal_error("lola: bad nlat 0","");
        save->nlat = ny;
        save->lat0 = y0;
        save->dlat = dy;

        save->iptr = (int *) malloc( ((size_t) nxny) * sizeof(int) );
	if (save->iptr == NULL) fatal_error("lola: memory allocation","");

        if (fopen_file(&(save->out), arg3, open_mode) != 0) {
	    free(save->iptr);
            free(save);
            fatal_error("Could not open %s", arg3);
        }
	save->last_GDS_change_no = 0;
        return 0;
    }

    save = (struct local_struct *) *local;

    /* cleanup phase */

    if (mode == -2) {
	fclose_file(&(save->out));
	free(save->iptr);
	free(save);
	return 0;
    }

    /* processing phase */

    nx = save->nlon;
    ny = save->nlat;
    nxny = nx*ny;

    if (save->last_GDS_change_no != GDS_change_no) {
	save->last_GDS_change_no = GDS_change_no;
        // if (output_order != wesn) fatal_error("lola only works in we:sn order","");
        if (lat == NULL || lon == NULL || data == NULL) fatal_error("lola: no val","");

        /* find the nearest points for the grid */
        closest_init(sec);
#pragma omp parallel for private(i,j,k,latitude,longitude)
        for (j = 0; j < ny; j++) {
            k = j*nx;
            latitude = save->lat0 + j*save->dlat;
            for (i = 0; i < nx; i++) {
               longitude = save->lon0 + i*save->dlon;
               save->iptr[k+i] = closest(sec, latitude, longitude);
            }
        }
    }

    if (strcmp(arg4,"spread") == 0) {
	if (save->out.cfile == NULL) fatal_error("lola: could not write spread","");
        k = 0;
        fprintf(save->out.cfile, "longitude, latitude, value,\n");
        for (j = 0; j < ny; j++) {
            latitude = save->lat0 + j*save->dlat;
            for (i = 0; i < nx; i++) {
                t = save->iptr[k] >= 0 ? data[save->iptr[k]] : UNDEFINED;
		if (DEFINED_VAL(t)) {
                    longitude = save->lon0 + i*save->dlon;
		    if (longitude >= 360.0) longitude -= 360.0;
                    fprintf(save->out.cfile, "%.6lf, %.6lf, %g,\n",longitude,latitude,t);
		}
		k++;
            }
        }
    }

    else if (strcmp(arg4,"bin") == 0) {
         if (header) {
	    if (nxny > 4294967295U / sizeof(float))
		fatal_error("lola: grid too large for header","");
            i = nxny * sizeof(float);   // may overflow
	    fwrite_file((void *) &i, sizeof(int), 1, &(save->out));
	}
        for (i = 0; i < nxny; i++) {
            t = save->iptr[i] >= 0 ? data[save->iptr[i]] : UNDEFINED;
            fwrite_file(&t, sizeof(float), 1, &(save->out));
	}
        if (header) {
            i = nxny * sizeof(float);   // may overflow
	    fwrite_file((void *) &i, sizeof(int), 1, &(save->out));
	}
    }

    else if (strcmp(arg4,"text") == 0) {
	if (save->out.cfile == NULL) fatal_error("lola: could not write text","");
        /* text output */
        if (header == 1) {
            fprintf(save->out.cfile ,"%u %u\n", nx, ny);
        }
        for (i = 0; i < nxny; i++) {
            t = save->iptr[i] >= 0 ? data[save->iptr[i]] : UNDEFINED;
            fprintf(save->out.cfile, text_format, t);
            fprintf(save->out.cfile, ((i+1) % text_column) ? " " : "\n");
        }
    }

    else if (strcmp(arg4,"grib") == 0) {
	/* make new_sec[] with new grid definition */
	for (i = 0; i < 8; i++) new_sec[i] = sec[i];
	new_sec[3] = sec3_lola(nx, save->lon0, save->dlon, ny, save->lat0, save->dlat, sec);

	/* get grid values */
	tmp = (float *) malloc(((size_t) nxny) * sizeof (float));
	if (tmp == NULL) fatal_error("-lola: memory allocation","");
        for (i = 0; i < nxny; i++) 
            tmp[i] = save->iptr[i] >= 0 ? data[save->iptr[i]] : UNDEFINED;

        grib_wrt(new_sec, tmp, nx*ny, nx, ny, use_scale, dec_scale, bin_scale, 
		wanted_bits, max_bits, grib_type, &(save->out));

	free(tmp);
    }

    if (flush_mode) fflush_file(&(save->out));
    return 0;
}
