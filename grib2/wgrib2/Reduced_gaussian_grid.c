#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Reduced_gaussian_grid.c
 *
 * 10/2017: Public Domain: Wesley Ebisuzaki
 *
 * convert from reduced Gaussian to full Gaussian
 * need to do: convert from full Gaussian to reduced Gaussian
 */

extern int decode, file_append, latlon;
extern unsigned int nx_, ny_;
extern int scan;
extern char *scan_order[];
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;

extern int n_variable_dim;
extern int *variable_dim, *raw_variable_dim;

extern enum output_grib_type grib_type;
extern enum output_order_type output_order_wanted, output_order;

enum interpol {linear, linear_extrapolate, neighbor, neighbor_extrapolate};
static void interpolate(float *in, int n_in, float *out, int n_out, enum interpol interpolation);

#define WESN	0		 /* FLAG TABLE 3.4 */
#define WENS	64		 /* FLAG TABLE 3.4 */

/*
 * HEADER:100:reduced_gaussian_grid:output:3:reduced Gaussian grid, X=outputfile Y=-1  Z=(neighbor|linear)[-extrapolate]
 */


int f_reduced_gaussian_grid(ARG3) {

    struct local_struct {
        struct seq_file out;
        int conversion;
        unsigned char sec3[72];
	enum interpol {linear, linear_extrapolate, neighbor, neighbor_extrapolate} interpolation;
    };
    struct local_struct *save;

    unsigned int i;
    int code_3_11, code_3_1, flag_3_4;
    int max_nx;
    int basic_ang, sub_ang;
    double units;
    double lon1, lon2;

    float *grid, *p_in, *p_out;
    unsigned char *new_sec[8], *gds;

    if (mode == -1) {			// initialization
        decode = 1;
	output_order_wanted = raw;

	// allocate static variables
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation reduced_gaussian_grid","");

	// open output file
        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("reduced_gaussian_grid: Could not open %s", arg1);
        }

	// conversion
	if (strcmp(arg2,"-1") == 0) {
	    save->conversion = -1;
	}
//	else if (strcmp(arg2,"to") == 0) {
//	    save->conversion = 1;
//	}
	else {
	    fatal_error("reduced_gaussian_grid: arg: %s should be -1", arg2);
	    exit(8);
	}

	if (strcmp(arg3,"linear") == 0) {
	    save->interpolation = linear;
	}
	else if (strcmp(arg3,"neighbor") == 0) {
	    save->interpolation = neighbor;
	}
	else if (strcmp(arg3,"linear-extrapolate") == 0) {
	    save->interpolation = linear_extrapolate;
	}
	else if (strcmp(arg3,"neighbor-extrapolate") == 0) {
	    save->interpolation = neighbor_extrapolate;
	}

	else {
	    fatal_error("reduced_gaussian_grid: bad comversion (%s)", arg3);
	    exit(8);
	}

	return 0;
    }
    save = *local;
    if (mode == -2) {				// cleanup
	fclose_file(&(save->out));
	free(save);
	return 0;
    }
    else if (mode >= 0) {			// processing
	if (output_order != raw) fatal_error("reduced_gaussian_grid: must be in raw output order","");
        // need to check that we:ns or we:sn
        flag_3_4 = flag_table_3_4(sec);
        if (flag_3_4 != WESN && flag_3_4 != WENS) {
	    fprintf(stderr,"reduced_gaussian_grid: only WESN or WENS processed\n");
	    return 0;
	}

	code_3_1 = code_table_3_1(sec);
	if (code_3_1 != 40) return 0;		// not Gaussian grid, return

	if (cyclic(sec) == 0) {
	    fprintf(stderr,"reduced_gaussian_grid: gaussian grid is not global, not handled\n");
	    return 0;
	}

	if (save->conversion == -1) {			// to full gaussian grid
	    code_3_11 = code_table_3_11(sec);
	    if (code_3_11 != 1) return 0;
	    if (n_variable_dim != ny_) fatal_error_i("reduced_gaussian_grid: unexpected code table 3.11 %d", code_3_11);

	    max_nx = variable_dim[0];
	    for (i = 1; i < n_variable_dim; i++) {
		max_nx = max_nx >= variable_dim[i] ? max_nx : variable_dim[i];
	    }

	    /* allocate memory for full grid */
	    grid = (float *) malloc(max_nx * ny_ * sizeof (float));
	    if (grid == NULL) fatal_error("reduced_gaussian_grid: memory allocation","");

	    p_in = data;
	    p_out = grid;
	    for (i = 0; i < ny_; i++) {
		interpolate(p_in, variable_dim[i], p_out, max_nx, save->interpolation);
		p_in += variable_dim[i];
		p_out += max_nx;
	    }

	    /* make GDS (sec3) for full grid */

	    // copy sec3 == save->sec3
	    for (i = 0; i < 72; i++) {
		save->sec3[i] = sec[3][i];
	    }
            uint_char(72, save->sec3);
	    save->sec3[4] = 3;
	    save->sec3[5] = 0;					// source of grid
	    uint_char(ny_*max_nx, save->sec3 + 6);		// number of data points
	    save->sec3[10] = 0;					// no extra lat/lon definitions
	    save->sec3[11] = 0;					// no extra lat/lon definitions
	    uint2_char(40, save->sec3 + 12);			// grid template number, Gaussian grid=40
	    uint_char(max_nx, save->sec3 + 30);	// nx

	    gds = sec[3];

            basic_ang = GDS_Gaussian_basic_ang(gds);
            sub_ang = GDS_Gaussian_sub_ang(gds);
            units = basic_ang == 0 ?  0.000001 : (float) basic_ang / (float) sub_ang;
            lon1 = units * GDS_Gaussian_lon1(gds);
	    lon2 = lon1 + 360.0 * (max_nx - 1.0) / max_nx;
	    if (lon2 > 360.0) lon2 -= 360.0;
	    lon2 = floor(lon2 / units + 0.5);
	    if (lon2 > 4294967295.0) fatal_error("reduced_gaussian_grid: unsigned integer overflow","");
	    i = lon2;
	    uint_char(i, save->sec3 + 59);		// lon2
	    i = floor( 360.0/max_nx/units + 0.5);
	    uint_char(i, save->sec3 + 63);		// dlon

	    for (i = 0; i < 8; i++) new_sec[i] = sec[i];
            new_sec[3] = save->sec3;

            grib_wrt(new_sec, grid, max_nx*ny_, max_nx, ny_, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));

	    free(grid);
        }
    }
    return 0;
}

/*
 * interpolation - cyclic
 *
 * interpolation == 0 .. linear
 * interpolation == 1 .. near neighbor
 */

static void interpolate(float *in, int n_in, float *out, int n_out, enum interpol interpolation) {
    unsigned int i, j, jp1, def_j, def_jp1;
    double r;
    if (n_in == n_out) {
	for (i = 0; i < n_in; i++) {
	    out[i] = in[i];
	}
    }
    else {
#pragma omp parallel for private(i,r,j,jp1, def_j, def_jp1)
	for (i = 0; i < n_out; i++) {
	    r = i * (double) n_in / (double) (n_out);
	    j = floor(r);
	    r = r - j;

	    jp1 = j + 1;
	    if (j >= n_in) j -= n_in;
	    if (jp1 >= n_in) jp1 -= n_in;
	    def_j = DEFINED_VAL(in[j]);
	    def_jp1 = DEFINED_VAL(in[jp1]);

	    if (r <= 0.0) {
		out[i] = in[j];
	    }
	    else if (r >= 1.0) {
		out[i] = in[jp1];
	    }
	    else if (def_j && def_jp1) {
	        if (interpolation == linear || interpolation == linear_extrapolate) {			// linear
	            out[i] = (1.0-r)*in[j] + r*in[jp1];
	        }
		else {
		    out[i] =  (r < 0.5) ? in[j] : in[jp1];
		}
	    }
	    else if (interpolation == linear || interpolation == neighbor) {
		out[i] = UNDEFINED;
	    }
	    else {
		out[i] = def_j ? in[j] : in[jp1];
	    }
	}
    }
}
