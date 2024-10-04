#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
/*
 * Cress_lola.c                  10/2024  Public Domain  Wesley Ebisuzaki
 *  use Cressman analysis to create a LOngitude LAtitude grid
 *
 */

/* M_PI is not ANSI C but are commonly defined */
/* values from GNU C library version of math.h copyright Free Software Foundation, Inc. */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif


extern int decode, flush_mode;
extern int file_append;

extern double *lat, *lon;
extern int latlon;
extern unsigned int npnts;
extern int GDS_change_no;

extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

#define MAX_SCANS 10
#define DIST_SQ(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))

/*
 * HEADER:111:cress_lola:output:4:lon-lat grid values X=lon0:nlon:dlon Y=lat0:nlat:dlat Z=file A=radius1:radius2:..:radiusN
 */

int f_cress_lola(ARG4) {

    int n, nx, ny, nxny, ix, iy, i, j, k, m, iradius;
    double x0,dx, y0,dy, x, y, z, r_sq, sum;
    unsigned char *new_sec[8];
    double *cos_lon, *sin_lon, s, c, tmp, *tmpv, *inc, *wt;
    float *background;

    struct local_struct {
        int nlat, nlon, nRadius;
        double lat0, lon0, dlat, dlon, latn, lonn;
        struct seq_file out;
	int last_GDS_change_no;
	double Radius[MAX_SCANS];
	double R_earth;
        double *in_x, *in_y, *in_z;
        double *out_x, *out_y, *out_z;
	char *mask;
    };
    struct local_struct *save;

    /* initialization phase */

    if (mode == -1) {
        decode = latlon = 1;	/* request decode of data, lat and lon */

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("cress_lola memory allocation ","");

	/* parse command line arguments */

        if (sscanf(arg1,"%lf:%d:%lf", &x0, &nx, &dx) != 3)
            fatal_error("cress_lola parsing longitudes lon0:nx:dlon  %s", arg1);

        if (dx < 0) fatal_error("cress_lola: dlon < 0", "");
        if (nx <= 0) fatal_error_i("cress_lola: bad nlon %d", nx);
        if (x0 < 0.0) x0 += 360.0;
        if (x0 < 0.0 || x0 >= 360.0) fatal_error("cress_lola: bad initial longitude","");
        save->nlon = nx;
        save->lon0 = x0;
        save->dlon = dx;
        save->lonn = x0 + (nx-1) * dx;

        if (sscanf(arg2,"%lf:%d:%lf", &y0, &ny, &dy) != 3) 
            fatal_error("cress_lola parsing latitudes lat0:nx:dlat  %s", arg2);

        if (dy < 0) fatal_error("cress_lola: dlat < 0","");
        if (ny <= 0) fatal_error_i("cress_lola: bad nlat %d", ny);
        save->nlat = ny;
        save->lat0 = y0;
        save->dlat = dy;
        save->latn = y0 + (ny-1)*dy;
        if (save->latn > 90.0 || save->lat0 < -90.0) fatal_error("cress_lola: bad latitude","");
        nxny = nx*ny;

        if (fopen_file(&(save->out), arg3, file_append ? "ab" : "wb") != 0) {
            fatal_error("Could not open %s", arg3);
        }

	iradius = 0;
	save->mask = NULL;
	k = sscanf(arg4, "%lf%n", &tmp, &m);
        while (k == 1) {
            if (iradius >= MAX_SCANS) fatal_error("cres_lola: too many radius parameters","");
            save->Radius[iradius++] = tmp;
	    if (tmp < 0.0 && save->mask == NULL) {
		save->mask = (char *) malloc(((size_t) nxny) * sizeof(char));
		if (save->mask == NULL) fatal_error("cress_lola memory allocation ","");
	    }
            arg4 += m;
            k = sscanf(arg4, ":%lf%n", &tmp, &m);
	}
	save->nRadius = iradius;

// fprintf(stderr,"nRadius=%d nx=%d ny=%d\n",save->nRadius, nx, ny);

	save->out_x = (double *) malloc(((size_t) nxny) * sizeof(double));
	save->out_y = (double *) malloc(((size_t) nxny) * sizeof(double));
	save->out_z = (double *) malloc(((size_t) nxny) * sizeof(double));
	if (save->out_x == NULL || save->out_y == NULL || save->out_z == NULL) 
		fatal_error("cress_lola: memory allocation","");

	save->in_x = save->in_y = save->in_z = NULL;
	save->last_GDS_change_no = 0;

	/* out_x, out_y, out_z have the 3-d coordinates of the lola grid */

	cos_lon = (double *) malloc(((size_t) nx) * sizeof(double));
	sin_lon = (double *) malloc(((size_t) nx) * sizeof(double));
	if (cos_lon == NULL || sin_lon == NULL) fatal_error("cress_lola: memory allocation","");
	for (i = 0; i < nx; i++) {
	    x = (x0 + i*dx) * (M_PI / 180.0);
	    cos_lon[i] = cos(x);
	    sin_lon[i] = sin(x);
	}

        for (k = j = 0; j < ny; j++) {
	    y = (y0 + j*dy) * (M_PI / 180.0);
            s = sin(y);
            c = sqrt(1.0 - s * s);
	    for (i = 0; i < nx; i++) {
                save->out_z[k] = s;
                save->out_x[k] = c * cos_lon[i];
                save->out_y[k] = c * sin_lon[i];
		k++;
	    }
	}
	free(cos_lon);
	free(sin_lon);
        return 0;
    }

    save = (struct local_struct *) *local;
    if (mode == -2) {
	fclose_file(&(save->out));
	free(save);
        return 0;
    }

    /* processing phase */
fprintf(stderr,">>processing\n");

    nx = save->nlon;
    ny = save->nlat;
    nxny = nx*ny;

    background = (float *) malloc(((size_t) nxny) * sizeof(float));
    tmpv = (double *) malloc(((size_t) nxny) * sizeof(double));
    inc = (double *) malloc(((size_t) nxny) * sizeof(double));
    wt = (double *) malloc(((size_t) nxny) * sizeof(double));
    if (background == NULL || tmpv == NULL || wt == NULL || inc == NULL) fatal_error("cress_lola: memory allocation","");

    /* Calculate x, y and z of input grid if new grid */
    if (save->last_GDS_change_no != GDS_change_no || save->in_x == NULL) {
	save->last_GDS_change_no = GDS_change_no;
        if (lat == NULL || lon == NULL || data == NULL) fatal_error("cress_lola: no lat, lon, or data","");

	save->R_earth  =  radius_earth(sec);

	if (save->in_x) free(save->in_x);
	if (save->in_y) free(save->in_y);
	if (save->in_z) free(save->in_z);

	save->in_x = (double *) malloc(((size_t) npnts) * sizeof(double));
	save->in_y = (double *) malloc(((size_t) npnts) * sizeof(double));
	save->in_z = (double *) malloc(((size_t) npnts) * sizeof(double));
	if (save->in_x == NULL || save->in_y == NULL || save->in_z == NULL)
	    fatal_error("cress_lola: memory allocation","");

	for (i = 0; i < npnts; i++) {
	    tmp = lon[i];
	    if (tmp < save->lon0) tmp += 360.0;
	    if (lat[i] >= 999.0 || lat[i] > save->latn || lat[i] < save->lat0 || tmp > save->lonn) {
		save->in_x[i] = 999.9;
	    }
	    else {
                s = sin(lat[i] * (M_PI / 180.0));
                c = sqrt(1.0 - s * s);
                save->in_z[i] = s;
                save->in_x[i] = c * cos(lon[i] * (M_PI / 180.0));
                save->in_y[i] = c * sin(lon[i] * (M_PI / 180.0));
            }
        }
fprintf(stderr,"done new gds processing npnts=%u\n", npnts);
    }

    /* at this point x, y, and z of input and output grids have been made */

    /* make new_sec[] with new grid definition */
    for (i = 0; i < 8; i++) new_sec[i] = sec[i];
    new_sec[3] = sec3_lola(nx, save->lon0, save->dlon, ny, save->lat0, save->dlat, sec);

    /* set background to average value of data */

    n = 0;
    sum = 0.0;
    /* make background = ave value */
    for (i = 0; i < npnts; i++) {
        if (save->in_x[i] < 999.0  && ! UNDEFINED_VAL(data[i]) ) {
	    n++;
	    sum += data[i];
	}
    }
    if (n == 0) {
	/* write undefined grid */
	for (i = 0; i < nxny; i++) background[i] = UNDEFINED;
        grib_wrt(new_sec, background, nxny, nx, ny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));
        if (flush_mode) fflush_file(&(save->out));
	free(background);
	free(tmpv);
	free(inc);
	free(wt);
	return 0;
    }
    sum /= n;
    for (i = 0; i < nxny; i++) background[i] = sum;
fprintf(stderr,">>sum=%lf n %d background[1] %lf\n", sum, n, background[1]);

    for (iradius = 0; iradius < save->nRadius; iradius++) {
fprintf(stderr,">>radias=%lf nxny %d npnts %u\n", save->Radius[iradius],nxny, npnts);
	/* save->Radius has units of km */
	/* normalize to a sphere of unit radius */
	r_sq = save->Radius[iradius] / (save->R_earth / 1000.0);
	r_sq = r_sq * r_sq;
	/* wt = inc = 0.0; */
	for (k = 0; k < nxny; k++) inc[k] = wt[k] = 0.0;

	for (j = 0; j < npnts; j++) {
	    if (save->in_x[j] > 999.0 || UNDEFINED_VAL(data[j]) ) continue;

	    /* find the background value */
	    x = lon[j] - save->lon0;
	    x = (x < 0.0) ? (x + 360.0) / save->dlon : x / save->dlon;
	    y = (lat[j] - save->lat0) / save->dlat;

	    ix = floor(x);
	    iy = floor(y);
	    if ((double) ix == x && ix == nx-1) ix--;
	    if ((double) iy == y && iy == ny-1) iy--;

	    if (ix < 0 || iy < 0 || ix >= nx || iy >= ny) fatal_error("cress_lola: prog error ix, iy","");

	    x = x - ix;
	    y = y - iy;

	    /* find background value */

	    tmp = background[ix+iy*nx] * (1-x)*(1-y) +
		  background[ix+1+iy*nx] * (x)*(1-y) +
		  background[ix+(iy+1)*nx] * (1-x)*(y) +
		  background[(ix+1)+(iy+1)*nx] * (x)*(y);

// fprintf(stderr,"obs: lat/lon %lf %lf, ix %d / %d iy %d data %lf, background %lf\n", lat[j],lon[j], ix, nx, iy, data[j], tmp);
	    /* data increment */
	    tmp = data[j] - tmp;

	    x = save->in_x[j];
	    y = save->in_y[j];
	    z = save->in_z[j];

	    for (k = 0; k < nxny; k++) {
		tmpv[k] = DIST_SQ(x-save->out_x[k], y-save->out_y[k], z-save->out_z[k]);
		if (tmpv[k] < r_sq) {
		    tmpv[k] = (r_sq - tmpv[k]) / (r_sq + tmpv[k]);
		    wt[k] += tmpv[k];
		    inc[k] += tmpv[k] * tmp;
		}
	    }
	}		

	/* make mask or update background */

	if (save->Radius[iradius] < 0.0) {
	    for (k = 0; k < nxny; k++) save->mask[k] = (wt[k] > 0) ? 1 : 0;
	}
	for (k = 0; k < nxny; k++) {
	    if (wt[k] > 0) background[k] += inc[k]/wt[k];
	}
    }

    if (save->mask) {
	for (k = 0; k < nxny; k++) {
	    if (save->mask[k] == 0) background[k] = UNDEFINED;
	}
    }
    grib_wrt(new_sec, background, nxny, nx, ny, use_scale, dec_scale, bin_scale,
	wanted_bits, max_bits, grib_type, &(save->out));

    if (flush_mode) fflush_file(&(save->out));
    free(background);
    free(tmpv);
    free(wt);
    free(inc);
    return 0;
}
