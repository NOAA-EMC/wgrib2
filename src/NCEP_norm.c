/** @file
 * @brief Normalize NCEP-type average/accumulation fields.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2009 | W. Ebisuzaki | Initial
 * 9/2013 | W. Ebisuzaki | Add support for more PDTs (only PDT=8 in v 0.1)
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 3/2009
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
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

/** Number of wanted bits. */
extern int wanted_bits;

/** Maximum number of bits. */
extern int max_bits;

/** Current output GRIB type. */
extern enum output_grib_type grib_type;

/*
 * HEADER:100:ncep_norm:output:1:normalize NCEP-type ave/acc X=output grib file
 */

/**
 * Normalize NCEP-type average/accumulation fields.
 * 
 * The GFS model, for example, produces precipitation with a hard-to-use time step. If you 
 * gathered the precip forecasts from the GFS, it would look like, 
 * 
 * @code{.sh}
 * $ wgrib2 apcp.grb
 * 1:0:d=2009042400:APCP:surface:0-3 hour acc fcst:
 * 2:3340:d=2009042400:APCP:surface:0-6 hour acc fcst:
 * 3:5534:d=2009042400:APCP:surface:6-9 hour acc fcst:
 * 4:7239:d=2009042400:APCP:surface:6-12 hour acc fcst:
 * 5:9304:d=2009042400:APCP:surface:12-15 hour acc fcst:
 * 6:12378:d=2009042400:APCP:surface:12-18 hour acc fcst:
 * @endcode
 * 
 * If you obtained the 6 hourly forecasts, everthing would be quite useful. But if you 
 * obtained the 3 hourly forecasts, you would notice the averaging period is either 3 or 6 
 * hours. To create 3-hour accumulations, you can use the -ncep_norm option. 
 * 
 * @code{.sh}
 * $ wgrib2 apcp.grb -ncep_norm new_apcp.grb
 * 1:0:d=2009042400:APCP:surface:0-3 hour acc fcst:
 * 2:3340:d=2009042400:APCP:surface:0-6 hour acc fcst:
 * 3:5534:d=2009042400:APCP:surface:6-9 hour acc fcst:
 * 4:7239:d=2009042400:APCP:surface:6-12 hour acc fcst:
 * 5:9304:d=2009042400:APCP:surface:12-15 hour acc fcst:
 * 6:12378:d=2009042400:APCP:surface:12-18 hour acc fcst:
 * 7:14396:d=2009042400:APCP:surface:18-21 hour acc fcst:
 * $
 * $ wgrib2 new_apcp.grb
 * 1:0:d=2009042400:APCP:surface:0-3 hour acc fcst:
 * 2:7769:d=2009042400:APCP:surface:3-6 hour acc fcst:
 * 3:13271:d=2009042400:APCP:surface:6-9 hour acc fcst:
 * 4:18773:d=2009042400:APCP:surface:9-12 hour acc fcst:
 * 5:24275:d=2009042400:APCP:surface:12-15 hour acc fcst:
 * 6:32044:d=2009042400:APCP:surface:15-18 hour acc fcst:
 * 7:38301:d=2009042400:APCP:surface:18-21 hour acc fcst:
 * @endcode
 * 
 * The -ncep_norm option is not limited to 3 hour intervals. It can be used with the CFSv2 
 * which has 1 hour intervals. Some restrictions are: 
 * 
 * 1. time units of forecast time and statistical processing must be the same
 * 2. only works with averages and accumulations (Code Table 4.10)
 * 3. the grid must be the same
 * 4. the meta-data must be the same except for the forecast timing.
 * 5. Does not work for CRAIN, CSNOW, CICEP, CFRZR
 * 
 * ## Usage
 * -ncep_norm FILE
 * 
 * ## Conditions
 * 
 * The input grib file must have a very specific format. Each field must be grouped together 
 * in time series format. That is because the program scans the file in sequential order. Of 
 * course, you can alter the order by the -i option. For example,
 * 
 * @code{.sh}
 * $ wgrib2 -match ':APCP:' mixed_up -vt | sort -t: -k3,3 | \
 *  wgrib2 -i mixed_up -ncep_norm new_apcp.grb
 * 1:0:d=2009042400:APCP:surface:0-0 day acc fcst:
 * 6:26940:d=2009042400:APCP:surface:0-3 hour acc fcst:
 * 11:56805:d=2009042400:APCP:surface:0-6 hour acc fcst:
 * 16:85235:d=2009042400:APCP:surface:6-9 hour acc fcst:
 * 51:281962:d=2009042400:APCP:surface:6-12 hour acc fcst:
 * 61:338477:d=2009042400:APCP:surface:12-15 hour acc fcst:
 * 66:367933:d=2009042400:APCP:surface:12-18 hour acc fcst:
 * 71:396448:d=2009042400:APCP:surface:18-21 hour acc fcst:
 * ...
 * $ wgrib2 new_apcp.grb
 * 1:0:d=2009042400:APCP:surface:0-3 hour acc fcst:
 * 2:7769:d=2009042400:APCP:surface:3-6 hour acc fcst:
 * 3:13271:d=2009042400:APCP:surface:6-9 hour acc fcst:
 * 4:18773:d=2009042400:APCP:surface:9-12 hour acc fcst:
 * 5:24275:d=2009042400:APCP:surface:12-15 hour acc fcst:
 * 6:32044:d=2009042400:APCP:surface:15-18 hour acc fcst:
 * 7:38301:d=2009042400:APCP:surface:18-21 hour acc fcst:
 * ...
 * @endcode
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 3/2009
 */
int f_ncep_norm(ARG1) {

    struct local_struct {
        float *val;
        int has_val;
        unsigned char *clone_sec[9];
        struct seq_file out;
    };
    struct local_struct *save;

    int idx, j, fhr_1, fhr_2,  dt1, dt2, new_type, is_ave;
    unsigned int i;
    float *d1, *data_tmp;

    if (mode == -1) {			// initialization
        save_translation = decode = 1;
        // 1/2015 use_scale = 0;

        // allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_ncep_norm","");
        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }

        save->has_val = 0;
        init_sec(save->clone_sec);
        return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2) {			// cleanup
        fclose_file(&(save->out));
        if (save->has_val == 1) {
            free(save->val);
            free_sec(save->clone_sec);
        }
        free(save);
        return 0;
    }

    if (mode >= 0) {			// processing

        idx = stat_proc_n_time_ranges_index(sec);

        // only process stat processed fields
        if (idx < 0) return 0;

        // n_time_ranges has to be one
        if (sec[4][idx] != 1) return 0;

        // only process averages or accumulations
        j = code_table_4_10(sec);
        if (j == 0) is_ave = 1;			// average
        else if (j == 1) is_ave = 0;		// accumulation
        else return 0;				// only process average or accumulations

        fhr_2 = forecast_time_in_units(sec);		// start time
        dt2 = int4(sec[4]+idx+50-42);			// delta-time
        if (dt2 == 0) return 0;	 			// dt == 0

        // units of fcst and stat proc should be the same if fcst time != 0
        if (fhr_2 != 0 && code_table_4_4(sec) != sec[4][49-42+idx]) {
            if (mode == 98) fprintf(stderr,"ncep_norm: units of fcst and stat_proc do not match %d %u\n",
                                    code_table_4_4(sec),sec[4][49-42+idx]);
            return 0;
        }

        if (mode == 98) fprintf(stderr,"ncep_norm: code table 4.10 (ave/acc/etc)=%d\n",j);
        if (mode == 98) fprintf(stderr,"ncep_norm: fhr_2=%d dt2=%d index of dt2=%d\n",fhr_2, dt2, idx+50-42);

        if (save->has_val == 0) {			// new data: write and save
            if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("memory allocation - data_tmp","");
            undo_output_order(data, data_tmp, ndata);
            grib_wrt(sec, data_tmp, ndata, nx, ny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));

            if (flush_mode) fflush_file(&(save->out));
            free(data_tmp);

            if (save->has_val  == 1) {			// copy data to save
                free(save->val);			// save = new field
                free_sec(save->clone_sec);
            }
            copy_sec(sec, save->clone_sec);
            copy_data(data,ndata,&(save->val));
            save->has_val = 1;
            if (mode == 99) fprintf(stderr,"ncep_norm: saved as new field\n");
            return 0;
        }

        new_type = 0;

        fhr_1 = forecast_time_in_units(save->clone_sec);		// start_time of save message
        dt1 = int4(save->clone_sec[4]+idx+50-42);			// delta-time

        if (mode == 98) fprintf(stderr,"ncep_norm: is_ave = %d\n fhr_1 %d dt1 %d fhr_2 %d dt2 %d\n",is_ave,
                                fhr_1, dt1, fhr_2, dt2);

        if (fhr_1 != fhr_2) {
            if (mode == 98) fprintf(stderr,"ncep_norm: new_type=1 because fhr1 != fhr2 %d %d\n", fhr_1, fhr_2);
            new_type = 1;
        }

        if (new_type == 0) {
            if (same_sec0(sec,save->clone_sec) == 0 ||
                same_sec1(sec,save->clone_sec) == 0 ||
                same_sec3(sec,save->clone_sec) == 0 ||
                same_sec4_diff_ave_period(sec,save->clone_sec) == 0) {
        
                if (mode == 98) fprintf(stderr,
                    "ncep_norm: test sec same_sec0=%d same_sec1=%d same_sec3=%d same_sec4_diff_ave_period=%d\n",
                    same_sec0(sec,save->clone_sec), same_sec1(sec,save->clone_sec), same_sec3(sec,save->clone_sec),
                    same_sec4_diff_ave_period(sec,save->clone_sec));
                    new_type = 1;
            }
        }
        if (mode == 98) fprintf(stderr,"ncep_norm: new_type=%d write and save\n",new_type);

        if (new_type == 1) {                    // fields dont match: write and save
            if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("memory allocation - data_tmp","");
            undo_output_order(data, data_tmp, ndata);
            grib_wrt(sec, data_tmp, ndata, nx, ny, use_scale, dec_scale, bin_scale,
                    wanted_bits, max_bits, grib_type, &(save->out));

            if (flush_mode) fflush_file(&(save->out));
            free(data_tmp);

            if (save->has_val  == 1) {                  // copy data to save
                free(save->val);                        // save = new field
                free_sec(save->clone_sec);
            }
            copy_sec(sec, save->clone_sec);
            copy_data(data,ndata,&(save->val));
            save->has_val = 1;
            if (mode == 98) fprintf(stderr," ncep_norm: saved as new type/sequence\n");
            return 0;
        }

        /* now do stuff */

        if (dt1 == dt2) return 0;			// same ending time

        // change metadata

        int_char(dt2-dt1, save->clone_sec[4]+50-42+idx);	//  dt = dt2 - dt1

        save->clone_sec[4][17] = sec[4][49-42+idx];             // new forecast time unit
        int_char(dt1+fhr_1, save->clone_sec[4]+18);             // fhr = fhr + dt1
        if (mode == 99) fprintf(stderr,"ncep_norm new fcst time %d + %d\n", dt1,fhr_1);

        for (i = idx-7; i < idx; i++) {                             // ending time from pds2
            save->clone_sec[4][i] = sec[4][i];
        }

        if (mode == 99) {
            if (is_ave)
                fprintf(stderr," process: factor: NEW*%g - OLD*%g\n", dt2/ (double) (dt2 - dt1),
                        dt1/ (double) (dt2-dt1));
            else fprintf(stderr," process: current-last\n");
        }

        // change floating point data

        d1= save->val;
        for (i = 0; i < ndata; i++) {
            if (!UNDEFINED_VAL(data[i]) && !UNDEFINED_VAL(*d1) ) {
                if (is_ave) {
                    *d1 = (data[i]*dt2 - *d1*dt1) / (double) (dt2 - dt1);
                }
                else {		// accumulation
                    *d1 = data[i] - *d1;
                }
            }
            else *d1 = UNDEFINED;
            d1++;
        }

        // write grib output

        if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("memory allocation - data_tmp","");
        undo_output_order(save->val, data_tmp, ndata);
        grib_wrt(save->clone_sec, data_tmp, ndata, nx, ny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));

        if (flush_mode) fflush_file(&(save->out));
        free(data_tmp);

        // save data
        free(save->val);                    // save = new field
        free_sec(save->clone_sec);
        copy_sec(sec, save->clone_sec);
        copy_data(data,ndata,&(save->val));
        return 0;
    }
    return 0;
}
