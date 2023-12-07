#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Wind dir - U then V in input file
 *
 *  v 0.1 experimental based on Wind_speed.c
 *
 * 3/2009: Public Domain: Wesley Ebisuzaki (wind_speed.c)
 * 1/2013: Public Domain: Wesley Ebisuzaki (wind_dir.c)
 *
 */

extern int decode, file_append, save_translation;
extern unsigned int nx_, ny_;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;

extern enum output_grib_type grib_type;

/*
 * HEADER:100:wind_dir:output:1:calculate wind direction, X = output gribfile (direction in degrees, 0=wind from north, 90=wind from east)
 */


int f_wind_dir(ARG1) {

    struct local_struct {
        float *val;
        int has_u;
        unsigned char *clone_sec[9];
        struct seq_file out;
    };
    struct local_struct *save;

    unsigned int i;
    int is_u, is_v;
    int test_s0, test_s1, test_s3, test_s4;
    float *d1, *data_tmp;
    int discipline, mastertab, parmcat, parmnum;

    if (mode == -1) {			// initialization
        save_translation = decode = 1;

	// allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation -wind_dir","");

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
	    fprintf(stderr,"WARNING: -wind_dir, unused UGRD\n");
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

	if (mode == 99) fprintf(stderr,"-wind_dir %d %d %d %d\n",mastertab,discipline,parmcat,parmnum);

	is_u = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 2);
	if (mode == 99 && is_u) fprintf(stderr,"\n-wind_dir: is u\n");

	if (is_u) {		// save data
	    if (save->has_u) {
	        fprintf(stderr,"WARNING: -wind_dir, unused UGRD\n");
	        free(save->val);
	        free_sec(save->clone_sec);
	    }
            copy_sec(sec, save->clone_sec);
	    copy_data(data,ndata,&(save->val));
            GB2_ParmNum(save->clone_sec) = 3;		// set id to V
	    save->has_u = 1;
	    return 0;
	}

	is_v = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 3);
	if (!is_v) return 0;
        if (save->has_u == 0) {
	    fprintf(stderr,"WARNING: -wind_dir, unused VGRD\n");
	    return 0;
	}

	// check for corresponding V

	test_s0 = test_s1 = test_s3 = test_s4 = 0;
        if ((test_s0 = same_sec0(sec,save->clone_sec)) == 1 &&
            (test_s1 = same_sec1(sec,save->clone_sec)) == 1 &&
            (test_s3 = same_sec3(sec,save->clone_sec)) == 1 &&
            (test_s4 = same_sec4(sec,save->clone_sec)) == 1) {


	    // check to see if winds are earth relative

            if ( (flag_table_3_3(sec) & 8)  != 0 ||
                   (flag_table_3_3(save->clone_sec) & 8)  != 0)  {
		fprintf(stderr,"wind_dir will not work with grid-relative winds, skipping\n");
                free(save->val);
                free_sec(save->clone_sec);
    	        save->has_u = 0;
                return 0;
	    }

	    // calculate wind direction

	    if (mode == 99) fprintf(stderr,"\n-wind_dir: calc wind direction\n");

            d1 = save->val;

#pragma omp parallel for private(i)
	    for (i = 0; i < ndata; i++) {
                if (!UNDEFINED_VAL(data[i]) && !UNDEFINED_VAL(d1[i])) {
		    d1[i] = (atan2(d1[i],data[i]) * 180.0 / 3.14159265359 + 180.0);
		}
	        else d1[i] = UNDEFINED;
	    }
            GB2_ParmNum(save->clone_sec) = 0;		// set id to direction degrees

	    // copy data to temp space

            if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("memory allocation - data_tmp","");
            undo_output_order(save->val, data_tmp, ndata);

/*  changed WNE 4/2019
            grib_wrt(save->clone_sec, data_tmp, ndata, nx_, ny_, use_scale, dec_scale, 
		bin_scale, wanted_bits, max_bits, grib_type, &(save->out));
 */
 /* direction to nearest degree */
            grib_wrt(save->clone_sec, data_tmp, ndata, nx_, ny_, 1, 0, 
		0, 9, 9, grib_type, &(save->out));

            if (flush_mode) fflush_file(&(save->out));
            free(data_tmp);

            // cleanup
            free(save->val);
            free_sec(save->clone_sec);
	    save->has_u = 0;
	}
	else {
	    if (mode) {
	        fprintf(stderr,"wind_dir: match failed sec0 %d sec1 %d sec3 %d sec4 %d\n", 
		test_s0, test_s1, test_s3, test_s4);
	    }
	    fprintf(stderr,"WARNING: -wind_dir, unused VGRD, not corresponding -v to see more\n");
	}
    }
    return 0;
}
