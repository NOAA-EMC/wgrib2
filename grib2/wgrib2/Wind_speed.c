#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Wind speed - U then V in input file
 *
 *  v 0.1 experimental
 *
 * 3/2009: Public Domain: Wesley Ebisuzaki
 *
 */

extern int decode, file_append, save_translation;
extern unsigned int nx_, ny_;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;

extern enum output_grib_type grib_type;

/*
 * HEADER:100:wind_speed:output:1:calculate wind speed, X = output gribfile (U then V in datafile)
 */

int f_wind_speed(ARG1) {

    struct local_struct {
        float *val;
        int has_u;
        unsigned char *clone_sec[9];
        struct seq_file out;
    };
    struct local_struct *save;

    unsigned int i;
    int is_u;
    float *d1, *data_tmp;
    int discipline, mastertab, parmcat, parmnum;

    if (mode == -1) {			// initialization
        save_translation = decode = 1;

	// allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("wind_speed: memory allocation","");

        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }

	save->has_u = 0;
	init_sec(save->clone_sec);
	return 0;
    }

    save = *local;
    if (mode == -2) {			// cleanup
	if (save->has_u == 1) {
	    free(save->val);
	    free_sec(save->clone_sec);
	}
	fclose_file(&(save->out));
	free(save);
	return 0;
    }

    if (mode >= 0) {			// processing

	// get variable name parameters

        discipline = GB2_Discipline(sec);
        mastertab = GB2_MasterTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
	// use_scale = 0;

	if (mode == 99) fprintf(stderr,"-wind_speed %d %d %d %d\n",mastertab,discipline,parmcat,parmnum);

	is_u = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 2);
	if (mode == 99 && is_u) fprintf(stderr,"\n-wind_speed: is u\n");

	if (is_u) {		// save data
	    if (save->has_u) {
	        free(save->val);
	        free_sec(save->clone_sec);
	    }
            copy_sec(sec, save->clone_sec);
	    copy_data(data,ndata,&(save->val));
            GB2_ParmNum(save->clone_sec) = 3;		// set id to V
	    save->has_u = 1;
	    return 0;
	}

        if (save->has_u == 0) return 0;

	// check for V

        if (same_sec0(sec,save->clone_sec) == 1 &&
            same_sec1(sec,save->clone_sec) == 1 &&
            same_sec3(sec,save->clone_sec) == 1 &&
            same_sec4(sec,save->clone_sec) == 1) {

	    // calculate wind speed

            d1 = save->val;
#pragma omp parallel for private(i)
	    for (i = 0; i < ndata; i++) {
                if (!UNDEFINED_VAL(data[i]) && !UNDEFINED_VAL(d1[i])) {
	            d1[i] = sqrt(data[i]*data[i] + d1[i] * d1[i]);
		}
	        else d1[i] = UNDEFINED;
	    }
            GB2_ParmNum(save->clone_sec) = 1;		// set id to wind speed

	    // copy data to temp space

            if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("wind_speed: memory allocation","");
            undo_output_order(save->val, data_tmp, ndata);
            grib_wrt(save->clone_sec, data_tmp, ndata, nx_, ny_, use_scale, dec_scale, 
		bin_scale, wanted_bits, max_bits, grib_type, &(save->out));

            if (flush_mode) fflush_file(&(save->out));

            // cleanup
            free(data_tmp);
            free(save->val);
            free_sec(save->clone_sec);
	    save->has_u = 0;
	}
    }
    return 0;
}
