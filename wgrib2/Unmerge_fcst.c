#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * unmerge_fcst
 *
 *  takes fcst averages/accumulations and unmerges them 
 *  for.  0-9 hour fcst ave, 0-12 hour fcst ave -> 0-9 hour fcst ave, 9-12 hour fcst ave
 *
 * 11/2019: v0.9 Public Domain: Wesley Ebisuzaki
 *
 */

extern int decode, file_append, nx, ny, save_translation;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

/*
 * HEADER:100:unmerge_fcst:output:3:unmerge_fcst X=output Y=fcst_time_0 Z: 0->result 1->+init 2->+all
 */

enum processing_type {ave, acc, max, min};

int f_unmerge_fcst(ARG3) {

    struct local_struct {
        float *val;				// grid point value accumulator
        int has_val;				// 1 = val is valid
        unsigned int n;					// number of grid points
	int fcst_time;				// forecast time
	int fcst_units;
        unsigned char *clone_sec[9];		// copy of original sec
	int output_option;			// 0: result, 1: init 2: all
        struct seq_file out;			// output file
    };
    struct local_struct *save;

    unsigned int i, k;
    int j, new_type, time_units, time_units_fcst, fcst_time, fcst_time_b, idx;
    int timea, timeb;
    unsigned char *verf_timea, *verf_timeb, *p;
    char string[3];
    enum processing_type processing;	// ave, acc, max, min
    float *data_tmp;

    if (mode == -1) {			// initialization
        save_translation = decode = 1;

	// allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("unmerge_fcst: memory allocation","");

        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("unmerge_fcst: Could not open %s", arg1);
        }
	save->has_val = 0;
	init_sec(save->clone_sec);

	/* fcst time */

        j = sscanf(arg2, "%d%2s", &(save->fcst_time), string);
        string[2] = 0;
        if (j != 2) fatal_error("unmerge_fcst: bad (int)(mn|hr|dy|mo|yr)","");
        save->fcst_units = string2time_unit(string);
        if (save->fcst_units == -1) fatal_error("ndates: unsupported time unit %s", string);

	save->output_option = atoi(arg3);

	return 0;
    }

    save = (struct local_struct *) *local;
    if (mode == -2) {				// cleanup
	fclose_file(&(save->out));
	if (save->has_val) {
	    free(save->val);
	    free_sec(save->clone_sec);
	}
	free(save);
	return 0;
    }

    if (mode >= 0) {				// processing

	// only process ave and acc

	j = code_table_4_10(sec);
	if (j == 0) processing = ave;
	else if (j == 1) processing = acc;
	else return 0;				// only process averages or accumulations

	// ave/acc must only have one time range
	idx = stat_proc_n_time_ranges_index(sec);
	if (idx < 0 || sec[4][idx] != 1) return 0;

	// same fcst as specified

	fcst_time = forecast_time_in_units(sec);
	if (fcst_time != save->fcst_time) return 0;

	time_units_fcst = code_table_4_4(sec);
	if (fcst_time != 0 && time_units_fcst != save->fcst_units) return 0;

	// time units (of stat processing) must be same as fcst time units, or fcst_time == 0
	time_units = (int) sec[4][idx + 49 - 42];

	if ((time_units != time_units_fcst) && (fcst_time != 0)) {
	    fprintf(stderr,"unmerge_fcst: fcst and stat_processing time units must equal (%d %d)\n",
		time_units_fcst, time_units);
	    return 0;
	}

	if (data == NULL) fatal_error("unmerge_fcst: grid values unavailable","");

	if (save->has_val == 0) {
	    save->n = ndata;
	    save->val = (float *) malloc(sizeof(float) * (size_t) ndata);
	    if (save->val == NULL) fatal_error("unmerge_fcst: memory allocation","");
	    // copy data to save->val in raw order
	    undo_output_order(&(save->val[0]), data, ndata);
            copy_sec(sec, save->clone_sec);
            save->has_val = 1;
	    if (save->output_option > 0) {
                i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
                if (flush_mode) fflush_file(&(save->out));
	    }
	    return 0;
	}

	/* at this point, have two grib messages */

	/* fcst times must be the same */

	fcst_time = forecast_time_in_units(sec);
	fcst_time_b = forecast_time_in_units(save->clone_sec);
	if (fcst_time != fcst_time_b) return 0;

	/* reference times must be the same */
	for (i = 12; i <= 18; i++) {
	    if (sec[1][i] != save->clone_sec[1][i]) break;
	}
	if (i != 19) {
	    /* different reference times, save latest message */
	    if (save->n != ndata) {
		free(save->val);
	        save->n = ndata;
	        save->val = (float *) malloc(sizeof(float) * (size_t) ndata);
	        if (save->val == NULL) fatal_error("unmerge_fcst: memory allocation","");
	    }
	    // copy data to save->val in raw order
	    undo_output_order(&(save->val[0]), data, ndata);
            copy_sec(sec, save->clone_sec);
            i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
            if (flush_mode) fflush_file(&(save->out));
	    return 0;
	}

	new_type = 0;

	// check various secs
	if (new_type == 0) {
            if (same_sec0(sec,save->clone_sec) == 0 ||
                same_sec1(sec,save->clone_sec) == 0 ||
                same_sec3(sec,save->clone_sec) == 0 ||
                same_sec4_unmerge_fcst(mode,sec,save->clone_sec) == 0) {

		new_type = 1;
                if (mode == 99) {
                    fprintf(stderr,"test sec0=%d\n",same_sec0(sec,save->clone_sec));
                    fprintf(stderr,"test sec1=%d\n",same_sec1(sec,save->clone_sec));
                    fprintf(stderr,"test sec3=%d\n",same_sec3(sec,save->clone_sec));
                    fprintf(stderr,"test sec4=%d\n",same_sec4_unmerge_fcst(99,sec,save->clone_sec));
                }
            }

	}

	/* check to see if repeat of saved message */
	if (new_type == 0 && same_sec4(sec,save->clone_sec) == 1) return 0;

        // if new_type == 0 .. caclulate residual
        //     save sec
        // if new_type == 1
	//     save sec

	if (new_type == 1) {
	    if (save->n != ndata) {
		free(save->val);
	        save->n = ndata;
	        save->val = (float *) malloc(sizeof(float) * (size_t) ndata);
	        if (save->val == NULL) fatal_error("unmerge_fcst: memory allocation","");
	    }
	    // copy data to save->val in raw order
	    undo_output_order(&(save->val[0]), data, ndata);
            copy_sec(sec, save->clone_sec);
	    if (save->output_option > 0) {
                i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
                if (flush_mode) fflush_file(&(save->out));
	    }
	}
	else {
	    /* new fcst hour is fcst hour + stat_processing time of clone */

            verf_timea = stat_proc_verf_time_location(sec);
            verf_timeb = stat_proc_verf_time_location(save->clone_sec);
            timea = int4(verf_timea + 15);
            timeb = int4(verf_timeb + 15);
	    if (timeb >= timea) {
		fatal_error("unmerge_fcst: wrong order","");
		return 0;
	    }

	    if (save->output_option > 1) {
                i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
                if (flush_mode) fflush_file(&(save->out));
	    }

	    /* copy sec[4] to save->clone_sec[4] */
	    k = GB2_Sec4_size(sec);
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	    for (i = 0; i < k; i++) save->clone_sec[4][i] = sec[4][i];

	    /* update forecast time */
	    p = forecast_time_in_units_location(save->clone_sec, &j);
	    if (p == NULL) fatal_error("unmerge_fcst, no fcst time","");
	    if (j == 4) int_char(fcst_time + timeb, p);
	    else int2_char(fcst_time + timeb, p);

	    /* update statistical processing time */
	    int_char(timea-timeb, verf_timeb+15);

	    data_tmp = (float *)  malloc(sizeof(float) * (size_t) ndata);
	    if (data_tmp == NULL) fatal_error("unmerge_fcst: memory allocation","");
	    // copy data to data_tmp in raw order
	    undo_output_order(data_tmp, data, ndata);

	    /* update data[] */
	    if (processing == acc) {
#pragma omp parallel for private(i)
		for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->val[i])) {
			save->val[i] = data_tmp[i] - save->val[i];
		    }
		    else {
			save->val[i] = UNDEFINED;
		    }
		}
	    }
	    else if (processing == ave) {
#pragma omp parallel for private(i)
		for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->val[i])) {
			save->val[i] = (data_tmp[i]*timea - save->val[i]*timeb)/(timea-timeb);
		    }
		    else {
			save->val[i] = UNDEFINED;
		    }
		}
	    }
	    else fatal_error("unmerge_fcst: program error","");

	    /* write grib */
            grib_wrt(save->clone_sec, &(save->val[0]), ndata, nx, ny, use_scale,
                dec_scale, bin_scale, wanted_bits, max_bits,
                grib_type, &(save->out));
            if (flush_mode) fflush_file(&(save->out));

	    /* copy raw_order(data) -> save->val */
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	    for (i = 0; i < ndata; i++) save->val[i] = data_tmp[i];
	    free(data_tmp);
            copy_sec(sec, save->clone_sec);
	}
    }
    return 0;
}
