#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * merge
 *
 *  takes equally sized fcst averages/accumulations and merges them together
 *  for.  1-2 day fcst ave + 2-3 day fcst ave + 3-4 day fcst ave give 1-4 day fcst ave
 *
 * 8/2009: v0.1 Public Domain: Wesley Ebisuzaki
 * 9/2013: v0.2 extensive modifications, support more PDTs Wesley Ebisuzaki
 * 7/2014: v0.3 add N=0 .. output after every merge operation (tigge, S2S output)
 *
 */

extern int decode, file_append, nx, ny, save_translation;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

/*
 * HEADER:100:merge_fcst:output:2:merge forecast ave/acc/min/max X=number to intervals to merge (0=every) Y=output grib file
 */

enum processing_type {ave, acc, max, min};

int f_merge_fcst(ARG2) {

    struct local_struct {
        float *val;				// grid point value accumulator
        int has_val;				// 1 = val is valid
        int n;					// number of grids accumulated
	int nx, ny, use_scale, dec_scale;
	int bin_scale, wanted_bits, max_bits;
	enum output_grib_type grib_type;
	int n_idx;				// idx of n_number_time_ranges
	int time_units;				// time unit for statistical processing
	unsigned int ndata;			// size of grid
	int fhour;				// last fhour
	int dt;					// stat processing interval
	enum processing_type processing;	// ave, acc, max, min
        unsigned char *clone_sec[9];		// copy of original sec
	unsigned char last_end_time[7];		// copy of the last end_of_overal_period
	int num_to_merge;			// number of intervals to merge
        struct seq_file out;			// output file
    };
    struct local_struct *save;

    unsigned int i;
    int j, new_type, time_units, idx;
    float *d, *data_tmp, factor;
    unsigned char temp_date[7], temp_date2[4];

    if (mode == -1) {			// initialization
        save_translation = decode = 1;

	// allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("merge_fcst: memory allocation","");

	if ((save->num_to_merge = atoi(arg1)) < 0) fatal_error("merge_fcst: bad number","");
        if (fopen_file(&(save->out), arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }
	save->has_val = 0;
	init_sec(save->clone_sec);
	return 0;
    }

    save = (struct local_struct *) *local;
    if (mode == -2) {			// cleanup
	fclose_file(&(save->out));
	if (save->has_val) {
	    free(save->val);
	    free_sec(save->clone_sec);
	}
	free(save);
	return 0;
    }

    if (mode >= 0) {			// processing

	// only process ave,acc,min,max

	j = code_table_4_10(sec);
	if (j == 0) save->processing = ave;
	else if (j == 1) save->processing = acc;
	else if (j == 2) save->processing = max;
	else if (j == 3) save->processing = min;
	else return 0;

if (mode == 99)  fprintf(stderr,"merge_fcst: code_table 4.10=%d  ",j);
if (mode == 99)  fprintf(stderr,"merge_fcst: save->has_val=%d  ",save->has_val);

	// ave/acc/min/max must only have one time range
	idx = stat_proc_n_time_ranges_index(sec);
	if (idx < 0 || sec[4][idx] != 1) return 0;

	// time units must match for fcst time and stat_proc interval
	time_units = code_table_4_4(sec);
	if (time_units != (int) sec[4][idx + 49 - 42]) return 0;

	// translate the data into raw mode now because the translation table
	// will be different for a new grid or missing at mode == -2

        if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL) {
	    fclose_file(&(save->out));
	    if (save->has_val) {
	        free(save->val);
	        free_sec(save->clone_sec);
	    }
	    free(save);
            fatal_error("merge: memory allocation","");
	}

	// grib_wrt wants data in original order
        undo_output_order(data, data_tmp, ndata);

	// check if new field

	new_type = 1;
	if (save->has_val == 1) {
	    new_type = 0;

	    if (save->fhour + save->dt * (save->n - 1) == forecast_time_in_units(sec))
			new_type = 1;

            if (same_sec0(sec,save->clone_sec) == 0 ||
            same_sec1(sec,save->clone_sec) == 0 ||
            same_sec3(sec,save->clone_sec) == 0 ||
            same_sec4_for_merge(0, sec,save->clone_sec) == 0) {
                new_type = 1;
                if (mode == 99) {
                   fprintf(stderr,"test sec0=%d\n",same_sec0(sec,save->clone_sec));
                   fprintf(stderr,"test sec1=%d\n",same_sec1(sec,save->clone_sec));
                   fprintf(stderr,"test sec3=%d\n",same_sec3(sec,save->clone_sec));
                   fprintf(stderr,"test sec4=%d\n",same_sec4_for_merge(mode, sec,save->clone_sec));
                }
            }
        }

if (mode == 99)  fprintf(stderr,"merge_fcst: new_type %d\n",new_type);
if (mode == 99)  fprintf(stderr,"merge_fcst: save->has_val=%d\n",save->has_val);

	// if new_type == 1 .. new field to process

	if (new_type == 1) {
	    if (mode == 99) fprintf(stderr,"\nmerge: new_type == 1, first field\n");
            copy_sec(sec, save->clone_sec);
            copy_data(data_tmp, ndata, &(save->val));

//	    save last_end_time
	    memcpy(save->last_end_time,sec[4]+idx+35-42,7);
	    
            save->has_val = 1;
            save->n = 1;
	    save->fhour = forecast_time_in_units(sec); 
	    save->dt = int4(sec[4]+idx+50-42);		// delta-time
	    save->nx = nx;
	    save->ny = ny;
	    save->use_scale = use_scale;
	    // 1/2015 save->use_scale = 0;
 	    save->dec_scale = dec_scale;
	    save->bin_scale = bin_scale;
	    save->wanted_bits = wanted_bits;
	    save->grib_type = grib_type;
	    save->max_bits = max_bits;
	    save->ndata = ndata;
	    save->n_idx = idx;

	    if (save->num_to_merge == 0) {
                grib_wrt(save->clone_sec, data_tmp, save->ndata, save->nx, save->ny, save->use_scale, 
		    save->dec_scale, save->bin_scale, save->wanted_bits, save->max_bits, 
		    save->grib_type, &(save->out));
                if (flush_mode) fflush_file(&(save->out));
	    }
	    free(data_tmp);
	    return 0;
	}

	if (mode == 99) fprintf(stderr,"merge_fcst: new type %d\n",new_type);

	d = save->val;

	if (save->processing == acc || save->processing == ave) {
	    for (i = 0; i < ndata; i++) {
		if (UNDEFINED_VAL(d[i]) || UNDEFINED_VAL(data_tmp[i])) d[i] = UNDEFINED;
		    else d[i] += data_tmp[i];
	    }
	}
	else if (save->processing == max) {
	    for (i = 0; i < ndata; i++) {
		if (UNDEFINED_VAL(d[i])) d[i] = data_tmp[i];
		else {
 		    if (DEFINED_VAL(data_tmp[i]) && (d[i] < data_tmp[i])) d[i] = data_tmp[i];
		}
	    }
	}
	else if (save->processing == min) {
	    for (i = 0; i < ndata; i++) {
		if (UNDEFINED_VAL(d[i])) d[i] = data_tmp[i];
		else {
 		    if (DEFINED_VAL(data_tmp[i]) && (d[i] > data_tmp[i])) d[i] = data_tmp[i];
		}
	    }
	}

        save->n = save->n + 1;
	memcpy(save->last_end_time,sec[4]+idx+35-42,7);

	// write the average/accumulation

	if (mode == 99) fprintf(stderr,"merge_fcst n %d want %d\n",save->n, save->num_to_merge);
	if (save->n == save->num_to_merge || save->num_to_merge == 0) {

	    d = save->val;
	    for (i = 0; i < save->ndata; i++) {
		data_tmp[i] = d[i];
	    }
	    if (save->processing == ave) {
	        factor = 1.0 / (double) save->n;
	        for (i = 0; i < save->ndata; i++) {
		    if (DEFINED_VAL(data_tmp[i])) data_tmp[i] *= factor;
	        }
	    }

	    // change time codes
            memcpy(temp_date2, save->clone_sec[4]+idx+50-42, 4);
            uint_char(save->dt * save->n, save->clone_sec[4]+idx+50-42);
	    // save clone_sec
	    memcpy(temp_date, save->clone_sec[4]+idx+34-42,7);
	    memcpy(save->clone_sec[4]+idx+34-42,sec[4]+idx+34-42,7);

            // grib_wrt wants data in original order
            undo_output_order(data_tmp, save->val, ndata);

            grib_wrt(save->clone_sec, data_tmp, save->ndata, save->nx, save->ny, save->use_scale, 
		save->dec_scale, save->bin_scale, save->wanted_bits, save->max_bits, 
		save->grib_type, &(save->out));
            if (flush_mode) fflush_file(&(save->out));

	    if (save->num_to_merge == 0) {
	        // restore clone_sec
                memcpy(save->clone_sec[4]+idx+50-42, temp_date2, 4);
	        memcpy(save->clone_sec[4]+idx+34-42, temp_date, 7);
	    }
	    else { 	// clear data
                save->has_val = 0;
	    }
	}
        free(data_tmp);
    }
    return 0;
}
