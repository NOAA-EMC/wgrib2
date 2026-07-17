/** @file
 * @brief Takes forecast averages/accumulations and unmerges them.
 * @author Public Domain: Wesley Ebisuzaki @date 11/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Number of grid points in the x direction. */
extern int nx;

/** Number of grid points in the y direction. */
extern int ny;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

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

/** Current output GRIB type. */
extern enum output_grib_type grib_type;

/*
 * HEADER:100:unmerge_fcst:output:3:unmerge_fcst X=output Y=fcst_time_0 Z: 0->result 1->+init 2->+all
 */

/** Processing type enumeration. */
enum processing_type {ave, acc, max, min};

/**
 * Unmerge forecast averages/accumulations.
 * 
 * Prior to GFS-FV3, the precipitation was stored in the "old-style". This style handled 6 hour 
 * accumulations in a simple manner.
 *
 *  0-6, 6-12, 12-18, 18-24, etc
 *
 * Eventually someone wanted to store 3 hour accumulations. However, people who had code that 
 * read the 6 hour accumulations said that they couldn't change the code. So the the file 
 * still had to contain the 6 hour accumulations. So the code to read the 3 hour accumulations 
 * had to a subtraction for some of the interval. They stored the accumulations like this.
 *
 *  0-3, 0-6, 6-9, 6-12, 12-15, 12-18, 18-21, 18-24, etc
 *
 * Now the FV3-GFS model developers agreed to store precipitation accumulations in the 
 * easy-to-use format,
 *
 *  0-3, 0-6, 0-9, 0-12, 0-15, 0-18, 0-21, 0-24, etc
 *
 * However, people who had code that read the accumulation in the old style said that they 
 * could not change their code. So they had put old style and new style accumulated 
 * precipitation in the files. 
 * 
 * To process the precip with wgrib2, you want to eliminate the old-style precip. It is 
 * surprisingly difficult because the old style can have to same name as the new style (ex. 3 
 * hour forecast). You have to eliminate the old style but not the new style even though they 
 * have the same name. 
 * 
 * The -unmerge_fcst option helps deal with the FV3-GFS precip problem when trying to make 
 * time series. 
 * 
 * ## Usage
 * -unmerge_fcst (output file) (N)(time unit) I
 * 
 * <pre>
 * output file: name of output grib file
 * N: integer,  usually 0
 * time unit:  the forecast time unit, hr, dy, mo, etc (If you want to process A-B hour acc fcst, then N=1, time unit=hr)
 * I : 1
 * </pre>
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * # make a file with the APCP from cat gfs.t00z.pgrb2.0p25.f(N) N=000..029 
 * $ cat gfs.t00z.pgrb2.0p25.f00? gfs.t00z.pgrb2.0p25.f01? gfs.t00z.pgrb2.0p25.f02? | wgrib2 - -match APCP -grib $stmp/all_apcpc.grb
 * # Lets see the new-style precip. in the file
 * $ cd $stmp
 * $ wgrib2 all_apcpc.grb -match ':0-'
 * 1:0:d=2020101600:APCP:surface:0-1 hour acc fcst:
 * 2:225707:d=2020101600:APCP:surface:0-1 hour acc fcst:
 * 3:451414:d=2020101600:APCP:surface:0-2 hour acc fcst:
 * 4:726257:d=2020101600:APCP:surface:0-2 hour acc fcst:
 * 5:1001100:d=2020101600:APCP:surface:0-3 hour acc fcst:
 * 6:1307459:d=2020101600:APCP:surface:0-3 hour acc fcst:
 * 7:1613818:d=2020101600:APCP:surface:0-4 hour acc fcst:
 * 8:1940346:d=2020101600:APCP:surface:0-4 hour acc fcst:
 * 9:2266874:d=2020101600:APCP:surface:0-5 hour acc fcst:
 * 10:2610484:d=2020101600:APCP:surface:0-5 hour acc fcst:
 * 11:2954094:d=2020101600:APCP:surface:0-6 hour acc fcst:
 * 12:3311989:d=2020101600:APCP:surface:0-6 hour acc fcst:
 * 14:3894753:d=2020101600:APCP:surface:0-7 hour acc fcst:
 * 16:4539821:d=2020101600:APCP:surface:0-8 hour acc fcst:
 * 18:5222224:d=2020101600:APCP:surface:0-9 hour acc fcst:
 * 20:5934324:d=2020101600:APCP:surface:0-10 hour acc fcst:
 * 22:6672253:d=2020101600:APCP:surface:0-11 hour acc fcst:
 * 24:7431863:d=2020101600:APCP:surface:0-12 hour acc fcst:
 * 26:8071608:d=2020101600:APCP:surface:0-13 hour acc fcst:
 * 28:8767119:d=2020101600:APCP:surface:0-14 hour acc fcst:
 * 30:9496847:d=2020101600:APCP:surface:0-15 hour acc fcst:
 * 32:10253719:d=2020101600:APCP:surface:0-16 hour acc fcst:
 * 34:11034026:d=2020101600:APCP:surface:0-17 hour acc fcst:
 * 36:11835057:d=2020101600:APCP:surface:0-18 hour acc fcst:
 * 38:12515832:d=2020101600:APCP:surface:0-19 hour acc fcst:
 * 40:13250721:d=2020101600:APCP:surface:0-20 hour acc fcst:
 * 42:14017034:d=2020101600:APCP:surface:0-21 hour acc fcst:
 * 44:14807995:d=2020101600:APCP:surface:0-22 hour acc fcst:
 * 46:15623322:d=2020101600:APCP:surface:0-23 hour acc fcst:
 * 48:16460745:d=2020101600:APCP:surface:0-1 day acc fcst:
 * 50:17171481:d=2020101600:APCP:surface:0-25 hour acc fcst:
 * 52:17933059:d=2020101600:APCP:surface:0-26 hour acc fcst:
 * 54:18724060:d=2020101600:APCP:surface:0-27 hour acc fcst:
 * 56:19538757:d=2020101600:APCP:surface:0-28 hour acc fcst:
 * 58:20377109:d=2020101600:APCP:surface:0-29 hour acc fcst:
 * @endcode
 * 
 * As you can see, 0-1, 0-2, 0-3, 0-4, 0-5 and 0-6 accumulations are duplicated. 
 * 
 * The -unmerge_fcst option is used to find the precipitation time series. 
 * 
 * @code{.sh}
 * $ wgrib2 all_apcpc.grb -unmerge_fcst precip_ts.grb 0hr 1
 * 1:0:d=2020101600:APCP:surface:0-1 hour acc fcst:
 * 2:225707:d=2020101600:APCP:surface:0-1 hour acc fcst:
 * 3:451414:d=2020101600:APCP:surface:0-2 hour acc fcst:
 * 4:726257:d=2020101600:APCP:surface:0-2 hour acc fcst:
 * 5:1001100:d=2020101600:APCP:surface:0-3 hour acc fcst:
 * 6:1307459:d=2020101600:APCP:surface:0-3 hour acc fcst:
 * ...
 * @endcode
 * 
 * The first argument is the output file. The second argument is the start of the accumulations to 
 * start, i.e., 0hr. Finally the third argument is 1 to include the 0-1 accumulation. The output is 
 * 
 * @code{.sh}
 * $ wgrib2 precip_ts.grb 
 * 1:0:d=2020101600:APCP:surface:0-1 hour acc fcst:
 * 2:225707:d=2020101600:APCP:surface:1-2 hour acc fcst:
 * 3:2302390:d=2020101600:APCP:surface:2-3 hour acc fcst:
 * 4:2302593:d=2020101600:APCP:surface:3-4 hour acc fcst:
 * 5:2302796:d=2020101600:APCP:surface:4-5 hour acc fcst:
 * ,,
 * 26:2307059:d=2020101600:APCP:surface:25-26 hour acc fcst:
 * 27:2307262:d=2020101600:APCP:surface:26-27 hour acc fcst:
 * 28:2307465:d=2020101600:APCP:surface:27-28 hour acc fcst:
 * 29:2307668:d=2020101600:APCP:surface:28-29 hour acc fcst:
 * @endcode
 *
 * @author Wesley Ebisuzaki @date 11/2019
 */
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

        // if new_type == 0 .. calculate residual
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
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
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
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
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
