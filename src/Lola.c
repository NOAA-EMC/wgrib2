/** @file
 * @brief Creates a LOngitude LAtitude grid.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 10/2006 | W. Ebisuzaki | Initial
 * 1/2008 | W. Ebisuzaki | lat and lon changed from float to double
 * 1/2011 | W. Ebisuzaki | replaced new_GDS by GDS_change_no
 * 12/2015 | W. Ebisuzaki | 4G grid cell limit except for bin output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Flush of output flag. */
extern int flush_mode;

/** Append grib file flag. */
extern int file_append;

/** Current output order type. */
extern enum output_order_type output_order;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Flag to indicate lat-lon grid processing.  */
extern int latlon;

/** GDS change number. */
extern int GDS_change_no;

/** Use scaling flag. */
extern int use_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/** Number of bits wanted. */
extern int wanted_bits;

/** Maximum number of bits. */
extern int max_bits;

/** Current output GRIB type. */
extern enum output_grib_type grib_type;

/** File header flag. */
extern int header;

/** Pointer to text format. */
extern char *text_format;

/** Number of columns in text output. */
extern int text_column;

/*
 * HEADER:100:lola:output:4:lon-lat grid values X=lon0:nlon:dlon Y=lat0:nlat:dlat Z=file A=[bin|text|spread|grib]
 */

/**
 * The -lola option was not named after a girl but for extracting data on a LOngitude-LAtitude grid. 
 * You need to specify the lower left corner of the grid, the number of points in the zonal and 
 * meridional directions and the latitude/longitude increments. Finally you need to specify the 
 * output file and the format. WARNING: winds and other vector fields will not be rotated. If the 
 * vector fields use a grid relative orientation, then your interpolated winds will be using the 
 * original grid. 
 * 
 * ## Interpolation Scheme
 * The interpolation to the lola grid is by nearest neighbor. Sure there are more accurate schemes 
 * and people are welcome to do better. Warning: the interpolation scheme simply picks up the value 
 * of the nearest neighbor. This can be very inaccurate for winds and other vectors near the pole. 
 * 
 * ## Usage
 * -lola LonSW:#lon:dlon LatSW:#lat:dlat file format
 * 
 * LonSW        Longitude of the South-West point, values from 0 .. 360
 * #lon         number of longitude points
 * dlon         spacing of the points in the zonal direction in degrees
 *
 * LatSW        Latitude of the South-West point, values from -90 .. 90
 * #lat         number of latitude points
 * dlat         spacing of the points in the meridional direction in degrees
 * 
 * file         name of the output file

 * format       format of the output file: bin, text, spread
 *               bin = binary
 *               text = simple "text" format, one value per line
 *               spread = spread-sheet format, latitude, longitude and value of each grid point
 *               grib = grib2
 * 
 * The order of the data points is WE:SN (wgrib2 standard).
 * 
 * ## Comments
 * I dislike this routine. It is slow, uses a simplistic interpolation scheme and doesn't handle 
 * rotated winds in a useful manner. 
 * 
 * The grib file support allows you to make lat-lon templates which is used by g2grb.gs. As much as 
 * I dislike this routine, I keep using it for interpolations. Keep wanting to modify it to do 
 * bilinear interpolations for interpolating from lat-lon grids. 
 * 
 * Interpolation from regular lat-lon grids is now handled as special case. This speeds up the 
 * interpolation and paves the way for a bilinear interpolation for the special cases. 
 * 
 * @param ARG4 List of function arguments set by wgrib2's main() function (see @ref ARG4). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 10/2006
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
#ifdef USE_OPENMP
#pragma omp parallel for private(i,j,k,latitude,longitude)
#endif
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
