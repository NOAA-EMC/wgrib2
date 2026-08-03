/** @file
 * @brief Convert from reduced Gaussian grid to full Gaussian grid.
 * @author Public Domain: Wesley Ebisuzaki @date 10/2017
 */
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

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Number of grid points in x-direction. */
extern unsigned int nx_;

/** Number of grid points in y-direction. */
extern unsigned int ny_;

/** Scan mode flag. */
extern int scan;

/** Pointer to string representation of scan order. */
extern char *scan_order[];

/** Flush of output flag. */
extern int flush_mode;

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

/** Number of variable dimensions. */
extern int n_variable_dim;

/** Pointer to array of variable dimensions. */
extern int *variable_dim;

/** Pointer to array of raw variable dimensions. */
extern int *raw_variable_dim;


/** Current output GRIB type. */
extern enum output_grib_type grib_type;

/** Desired output order type. */
extern enum output_order_type output_order_wanted;

/** Current output order type. */
extern enum output_order_type output_order;

/** Interpolation method. */
enum interpol {linear, linear_extrapolate, neighbor, neighbor_extrapolate};

static void interpolate(float *in, int n_in, float *out, int n_out, enum interpol interpolation);

#define WESN	0		 /**< FLAG TABLE 3.4 - WESN order */
#define WENS	64		 /**< FLAG TABLE 3.4 - WENS order */

/*
 * HEADER:100:reduced_gaussian_grid:output:3:reduced Gaussian grid, X=outputfile Y=-1  Z=(neighbor|linear)[-extrapolate]
 */

/**
 * Converts reduced Gaussian grid to regular Gaussian grid.
 * 
 * Regular Gaussian grids are not space efficient because the number of grid points on a Gaussian 
 * latitude does not vary with latitude. For example, you don't need as many points near the poles 
 * as next to the equator to describe the same spatial scales. With a reduced Gaussian grid, the 
 * number of grid points on a Gaussian latitude varys from large near the equator to small near the 
 * poles. This reduces the number of grid points, reduces calculations and speeds up the model. One 
 * draw back for the users are they often have convert the grid to a regular Gaussian grid for plotting, 
 * interpolation to another grid or making a regional subset. 
 * 
 * The -reduced_gaussian_grid option converts from a reduced Gaussian grid to a regular Gaussian grid. 
 * This option has two interpolation options. The "linear" interpolation means that the grid points are 
 * interpolated linearly from the surrounding two points on the Gaussian latitude. "Neighbor" 
 * interpolation will use the value from the nearest grid point on the Gaussian latitude. 
 * 
 * ## Interpolation
 * The -reduced_gaussian_grid option interpolates using points on the same Gaussian latitude. There are two 
 * basic interpolation techniques, linear and neighbor. 
 * 1. linear interpolates linearly using the two surrounding grid points on the same Gaussian latitude 
 * assuming that both grid points have valid values. 
 * 2. neighbor interpolation will take the value of the nearest grid point on the same Gaussian latitude 
 * assuming that both grid points have valid values. 
 * 
 * Some fields have undefined values. Therefore, in some cases you may have to extrapolate a value. 
 * 
 * D------x-------U         (Gaussian latitude)
 * 
 * D=defined value, U=undefined values, x=location of value that is desired
 * 
 * - Option 1: no extrapolation, value at x is undefined (default)
 * - Option 2: extrapolation, value at x is the value D (extrapolate)
 * - Option 3: x has the value D if x is closer to "D" than "U", otherwise it has a undefined values 
 * (not supported)
 * 
 * The supported interpolation types are:
 * 
 * 1. linear : linear, no extrapolation
 * 2. linear-extrapolate :  linear, extrapolation
 * 3. neighbor : nearest neighbor, no extrapolation
 * 4. neighbor-extrapolate : nearest neighbor, extrapolation
 *
 * ## Usage
 * -reduced_gaussian_grid OUT -1 INTERPOLATION
 *
 * OUT = output grib file for regular Gaussian grid. The interpolation is linear on the Gaussian latitude, 
 * but will be made selectable in the future.
 * 
 * -1 = means interpolate to a regular Gaussian grid. In the future, this parameter will be extended for 
 * conversion to a reduced Gaussian grid.
 * 
 * INTERPOLATION = linear, neighbor, linear-extrapolate, neighbor-extrapolate
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise
 * 
 * ## Example
 * See that reduced_gaussian_surface_jpeg.grib2 is a reduced Gaussian grid.
 * @code{.sh}
 * $ wgrib2 reduced_gaussian_surface_jpeg.grib2 -grid
 * 1:0:grid_template=40:winds(N/S):
 *	thinned global Gaussian grid: (-1 x 64) units 1e-06 input WE:NS output raw
 *	number of latitudes between pole-equator=32 #points=6114
 *	lat 87.864000 to -87.864000
 *	lon 0.000000 to 357.188000 by -2147.483647
 *	#grid points by latitude: 20 27 36 40 45 50 60 64 72 75 80 90 90
 *	 96 100 108 108 120 120 120 128 128 128 128 128 128 128 128 128 128 128 128 128
 *	 128 128 128 128 128 128 128 128 128 128 128 120 120 120 108 108 100 96 90 90
 *	 80 75 72 64 60 50 45 40 36 27 20
 * @endcode
 * 
 * Write regular Gaussian grid in new.grb.
 * @code{.sh}
 * $ wgrib2 reduced_gaussian_surface_jpeg.grib2 -reduced_gaussian_grid new.grb -1 linear
 * 1:0:d=2007032312:TMP:surface:anl:
 * @endcode
 * 
 * Check that new.grb is a regular Gaussian grid.
 * @code{.sh}
 * $ wgrib2 new.grb -grid
 * 1:0:grid_template=40:winds(N/S):
 *	Gaussian grid: (128 x 64) units 1e-06 input WE:NS output WE:SN
 *	number of latitudes between pole-equator=32 #points=8192
 *	lat 87.864000 to -87.864000
 *	lon 0.000000 to 357.187500 by 2.812500
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 10/2017
 */
int f_reduced_gaussian_grid(ARG3) {

    struct local_struct {
        struct seq_file out;
        int conversion;
        unsigned char sec3[72];
        enum interpol interpolation;
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

/**
 * Perform interpolation to convert the reduced Gaussian grid to a regular Gaussian grid.
 *
 * @param in Pointer to input data array
 * @param n_in Size of input data array
 * @param out Pointer to output data array
 * @param n_out Size of output data array
 * @param interpolation Interpolation method to use
 * 
 * @author Wesley Ebisuzaki @date 10/2017
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
#ifdef USE_OPENMP
#pragma omp parallel for private(i,r,j,jp1, def_j, def_jp1)
#endif
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
