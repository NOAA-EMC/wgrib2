/** @file
 * @brief Time processing functions for GRIB2 data.
 * 
 * Mean and variance are computed using [Welford's method]
 * (http://jonisalonen.com/2013/deriving-welfords-method-for-computing-variance/).
 * 
 *  variance(samples):
 *    M := 0
 *    S := 0
 *    for k from 1 to N:
 *       x := samples[k]
 *       oldM := M
 *       M := M + (x-M)/k
 *       S := S + (x-M)*(x-oldM)
 *   return S/(N-1)
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 4/2009 | W. Ebisuzaki | Initial
 * 4/2010 | W. Ebisuzaki | Added means of means
 * 4/2013 | W. Ebisuzaki | Added PDT 4.11 (ensemble)
 * 12/2014 | W. Ebisuzaki | set use_scale to zero, optimizations
 * 1/2015 | W. Ebisuzaki | removed set use_scale
 * 3/2016 | W. Ebisuzaki | added PDT 2 and 12
 * 9/2017 | W. Ebisuzaki | reborn as time processing
 * @author Public Domain: Wesley Ebisuzaki @date 4/2009
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
 * HEADER:000:ave:output:2:average X=time step Y=output v2
 */

/**
 * Makes temporal averages assuming the reference and verification times are increasing by a 
 * constant amount between samples. 
 * 
 * The -ave and -fcst_ave options are very similar. Both now call the -time_processing option. 
 * This option handles more statistical operations and more Product Definition Templates (PDT). 
 * The old -ave and -fcst_ave options will available as -ave0 and -fcst_ave0. I expect that 
 * these two options to be eliminated soon. 
 * 
 * You would use -fcst_ave to temporally average a single forecast run. For example, you have 
 * a 3 week forecast with output every 6 hours. You could use -fcst_ave to find the forecast 
 * for the second week of the forecast. 
 * 
 * You would use -ave to temporally average several analyses. Suppose you have analyses every 
 * 6 hours and you want to find the analysis for the month. 
 * 
 * The -ave and -fcst_ave options require the grib messages to be processed in a special order. 
 * Don't worry, a grib file can be ordered very easily with the sort command. Wgrib2 reads the 
 * data sequentially and when ever it encounters a new variable/level/chemical-type, it starts 
 * the averaging process. The length of the averaging depends on how many records it finds to 
 * average. For example, to make a daily average, a file has to be in the following order. 
 * 
 *      U500 2000-01-02 00Z             start ave
 *      U500 2000-01-02 06Z
 *      U500 2000-01-02 12Z
 *      U500 2000-01-02 18Z             end ave
 *      V500 2000-01-02 00Z             start ave
 *      V500 2000-01-02 06Z
 *      V500 2000-01-02 12Z
 *      V500 2000-01-02 18Z             end ave
 *      Z500 2000-01-02 00Z             start ave
 *      Z500 2000-01-02 06Z
 *      Z500 2000-01-02 12Z
 *      Z500 2000-01-02 18Z             end ave
 *
 * To make a daily average of the above file, you need to specify the output file and the time 
 * interval between samples. The time units are the same as used by GrADS (hr, dy, mo, yr). 
 * 
 * @code{.sh}
 * $ wgrib2 input.grb -ave 6hr out.grb
 * @endcode
 * 
 * If the file is not sorted, you can use the unix sort by, 
 * 
 * @code{.sh}
 * $ wgrib2 input.grb | sort -t: -k4,4 -k5,5 -k6,6 -k3,3 | \
 * wgrib2 -i input.grb -set_grib_type c3 -ave 6hr output.grb
 * @endcode
 * 
 * If you want to make daily means from 4x daily monthly files and assuming that more than one 
 * variable/level is in the monthly file. 
 * 
 * @code{.sh}
 * $ wgrib2 input.grb |  sed 's/\(:d=........\)/\1:/' | \
 * sort -t: -k3,3 -k5,5 -k6,6 -k7,7 -k4,4 | \
 * wgrib2 input.grb -i -set_grib_type c3 -ave 6hr daily.ave.grb
 * @endcode
 * 
 * ## Usage
 * -ave (time interval) (output grib file)
 * 
 * ## Limitations
 * There is a limit in the maximum number of -if_fs/-ave clauses. Wgrib2 v2.0.6+ can process 
 * up to 2000 -if and 2000 -if_fs options and accept 10000 words on the command line. Since 
 * each -if_fs/-ave clause takes 5 words on the command line and you need to include the name 
 * of the input file, you get a limit of 999 -if_fs/-ave clauses. To speed up the code, the 
 * evaluation of the -if_fs options is done in parallel. 
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_ave(ARG2) {
    if (mode == -1) 
        return f_time_processing(init_ARG4(inv_out,local,"0","1",arg1,arg2));
    if (mode == -2) 
        return f_time_processing(fin_ARG4(inv_out,local,"0","1",arg1,arg2));
    return f_time_processing(call_ARG4(inv_out,local,"0","1",arg1,arg2));
}

/*
 * HEADER:000:fcst_ave:output:2:average X=time step Y=output v2
 */

/**
 * Makes temporal averages assuming that the reference (initial) time is constant and the 
 * verification time is increasing by a constant amount between samples.
 * 
 * The -ave and -fcst_ave options are very similar. Both now call the -time_processing option. 
 * This option handles more statistical operations and more Product Definition Templates (PDT). 
 * The old -ave and -fcst_ave options will available as -ave0 and -fcst_ave0. I expect that 
 * these two options to be eliminated soon. 
 * 
 * You would use -fcst_ave to temporally average a single forecast run. For example, you have 
 * a 3 week forecast with output every 6 hours. You could use -fcst_ave to find the forecast 
 * for the second week of the forecast. 
 * 
 * You would use -ave to temporally average several analyses. Suppose you have analyses every 
 * 6 hours and you want to find the analysis for the month. 
 * 
 * The -ave and -fcst_ave options require the grib messages to be processed in a special order. 
 * Don't worry, a grib file can be ordered very easily with the sort command. Wgrib2 reads the 
 * data sequentially and when ever it encounters a new variable/level/chemical-type, it starts 
 * the averaging process. The length of the averaging depends on how many records it finds to 
 * average. For example, to make a daily average, a file has to be in the following order. 
 * 
 *      U500 2000-01-02 00Z             start ave
 *      U500 2000-01-02 06Z
 *      U500 2000-01-02 12Z
 *      U500 2000-01-02 18Z             end ave
 *      V500 2000-01-02 00Z             start ave
 *      V500 2000-01-02 06Z
 *      V500 2000-01-02 12Z
 *      V500 2000-01-02 18Z             end ave
 *      Z500 2000-01-02 00Z             start ave
 *      Z500 2000-01-02 06Z
 *      Z500 2000-01-02 12Z
 *      Z500 2000-01-02 18Z             end ave
 *
 * Using -fcst_ave is like using -ave except you use the verification time instead of the 
 * reference time. To make an inventory that use the verification time instead of the reference 
 * time, you type:
 * 
 * @code{.sh}
 * $ wgrib2 input.grb -vt -var -lev -misc 
 * 1:0:vt=2011040101:PRATE:surface:
 * 2:592224:vt=2011040102:PRATE:surface:
 * 3:1233694:vt=2011040103:PRATE:surface:
 * 4:1909322:vt=2011040104:PRATE:surface:
 * 5:2612620:vt=2011040105:PRATE:surface:
 * @endcode
 * 
 * ## Usage
 * -fcst_ave (time interval) (output grib file)
 * 
 * ## Limitations
 * There is a limit in the maximum number of -if_fs/-ave clauses. Wgrib2 v2.0.6+ can process 
 * up to 2000 -if and 2000 -if_fs options and accept 10000 words on the command line. Since 
 * each -if_fs/-ave clause takes 5 words on the command line and you need to include the name 
 * of the input file, you get a limit of 999 -if_fs/-ave clauses. To speed up the code, the 
 * evaluation of the -if_fs options is done in parallel. 
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_fcst_ave(ARG2) {
    if (mode == -1) 
        return f_time_processing(init_ARG4(inv_out,local,"0","2",arg1,arg2));
    if (mode == -1) 
        return f_time_processing(call_ARG4(inv_out,local,"0","2",arg1,arg2));
    return f_time_processing(call_ARG4(inv_out,local,"0","2",arg1,arg2));
}

/* supported code table 4.10 */
#define AVE	0           /**< Code Table 4.10 - Average */
#define MAX	2           /**< Code Table 4.10 - Maximum */
#define MIN	3           /**< Code Table 4.10 - Minimum */
#define DIFF1	4       /**< Code Table 4.10 - Difference (value at the end of the time range minus value at the beginning) */
#define RMS	5           /**< Code Table 4.10 - Root Mean Square */
#define STD_DEV	6       /**< Code Table 4.10 - Standard Deviation */
#define DIFF2	8       /**< Code Table 4.10 - Difference (value at the beginning of the time range minus value at the end) */

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

/** Pointer to the translation array. */
extern unsigned int *translation;

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

/** Structure to hold average calculation data. */
struct ave_struct {
    double *sum;                        /**< Pointer to the sum values. */
    double *M;                          /**< Pointer to mean values. */
    double *S;                          /**< Pointer to variance values. */
    double *first;                      /**< Pointer to first values in difference. */
    double *last;                       /**< Pointer to last values in difference. */
    int *n;                             /**< Pointer to number of times for sum, etc. */
    unsigned int npnts;                 /**< Number of grid points. */
    int has_val;                        /**< Flag to indicate if values are present. */
    int n_fields;                       /**< Number of fields. */
    int n_missing;                      /**< Number of missing values. */
    int dt;                             /**< Time interval. */
    int dt_unit;                        /**< Time unit. */
    int nx;                             /**< Number of grid points in the x direction. */
    int ny;                             /**< Number of grid points in the y direction. */
    unsigned char *first_sec[9];        /**< Pointer to first section data. */
    unsigned char *next_sec[9];         /**< Pointer to next section data. */
    int use_scale;                      /**< Use scaling flag. */
    int dec_scale;                      /**< Decimal scaling. */
    int bin_scale;                      /**< Binary scaling. */
    int wanted_bits;                    /**< Number of bits wanted. */
    int max_bits;                       /**< Maximum number of bits. */
    enum output_grib_type grib_type;    /**< Current output GRIB type. */
    int code_table_4_10;                /**< Code Table 4.10 - Type of Statistical Processing. */
    int code_table_4_11;                /**< Code Table 4.11 - Type of Time Intervals. */
    struct full_date ref_time0;         /**< Lowest reference time. */
    struct full_date ref_time;          /**< Current reference time. */
    struct full_date verf_time;         /**< Verification time. */
    struct seq_file out;                /**< Output sequence file. */
};

static int do_ave(struct ave_struct *save);
static int free_ave_struct(struct ave_struct *save);
static int init_ave_struct(struct ave_struct *save, unsigned int ndata);
static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing);

/**
 * Free the memory allocated for the average calculation structure.
 *
 * @param save Pointer to the ave_struct.
 *
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
static int free_ave_struct(struct ave_struct *save) {
    if (save->has_val == 1) {
        if (save->code_table_4_10 == STD_DEV) { free(save->M); free(save->S); }
        else if (save->code_table_4_10 == DIFF1 || save->code_table_4_10 == DIFF2 ) { free(save->first); free(save->last); }
        else free(save->sum);
        free(save->n);
        free_sec(save->first_sec);
        free_sec(save->next_sec);
    }
    free(save);
    return 0;
}

/**
 * Initialize the average calculation structure.
 *
 * @param save Pointer to the ave_struct to be initialized.
 * @param ndata Number of data points.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
static int init_ave_struct(struct ave_struct *save, unsigned int ndata) {
    unsigned int i;

    /* allocated but wrong size, free all */
    if (save->has_val == 1 && save->npnts != ndata) {
        if (save->code_table_4_10 == STD_DEV) {
            free(save->M);
            free(save->S);
        }
        else if (save->code_table_4_10 == DIFF1 || save->code_table_4_10 == DIFF2) {
            free(save->first);
            free(save->last);
        }
        else free(save->sum);
        free(save->n);
        save->has_val = 0;
    }

    /* if not allocated, allocate */
    if (save->has_val == 0) {
        if (save->code_table_4_10 == STD_DEV) {
            save->M = (double *) malloc( ((size_t) ndata) * sizeof(double));
            save->S = (double *) malloc( ((size_t) ndata) * sizeof(double));
            if (save->M == NULL || save->S == NULL)
                fatal_error("time_processing: memory allocation problem: val","");
        }
        else if (save->code_table_4_10 == DIFF1 || save->code_table_4_10 == DIFF2) {
            save->first = (double *) malloc( ((size_t) ndata) * sizeof(double));
            save->last = (double *) malloc( ((size_t) ndata) * sizeof(double));
            if (save->first == NULL || save->last == NULL)
                fatal_error("time_processing: memory allocation problem: val","");
        }
        else {
            if ((save->sum = (double *) malloc( ((size_t) ndata) * sizeof(double))) == NULL)
                fatal_error("time_processing: memory allocation problem: val","");
        }

        if ((save->n = (int *) malloc(((size_t) ndata) * sizeof(int))) == NULL)
            fatal_error("time_processing: memory allocation problem: val","");
    }

    /* iniitialize variables */
    if (save->code_table_4_10 == STD_DEV) {
        for (i=0; i < ndata; i++) {
            save->S[i] = save->M[i] = 0.0;
        }
    }
    else if (save->code_table_4_10 == DIFF1 || save->code_table_4_10 == DIFF2) {
        for (i=0; i < ndata; i++) {
            save->first[i] = save->last[i] = 0.0;
        }
    }
    else {
        for (i=0; i < ndata; i++) {
            save->sum[i] = 0.0;
        }
    }
    for (i=0; i < ndata; i++) {
        save->n[i] = 0;
    }

    save->npnts = ndata;
    save->has_val = 1;
    save->n_fields = 0;
    save->n_missing = 0;
    free_sec(save->first_sec);
    free_sec(save->next_sec);
    return 0;
}

/**
 * Add data to the average calculation structure.
 *
 * @param save Pointer to the ave_struct.
 * @param sec Pointer to the GRIB sections.
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 * @param missing Number of missing data points.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata, int missing) {

    unsigned int i, ii;
    double x, oldM;

    if (save->npnts != ndata) fatal_error("time_processing: add_to_ave dimension mismatch","");

    /* the data needs to be translated from we:sn to raw, need to 
       do it now, translation[] may be different if called from finalized phase */

    if (save->code_table_4_10 == AVE) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
                ii = translation == NULL ? i : translation[i];
                save->sum[ii] += data[i];
                save->n[ii]++;
            }
        }
    }
    else if (save->code_table_4_10 == MAX) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
                ii = translation == NULL ? i : translation[i];
                if (save->n[ii]++) {
                    save->sum[ii] = save->sum[ii] >= data[i] ? save->sum[ii] : data[i];
                }
                else {
                    save->sum[ii] = data[i];
                }
            }
        }
    }
    else if (save->code_table_4_10 == MIN) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
                ii = translation == NULL ? i : translation[i];
                if (save->n[ii]++) {
                    save->sum[ii] = save->sum[ii] <= data[i] ? save->sum[ii] : data[i];
                }
                else {
                    save->sum[ii] = data[i];
                }
            }
        }
    }
    else if (save->code_table_4_10 == RMS) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
                ii = translation == NULL ? i : translation[i];
                save->sum[ii] += data[i]*data[i];
                save->n[ii]++;
            }
        }
    }
    else if (save->code_table_4_10 == STD_DEV) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii,x,oldM)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
                ii = translation == NULL ? i : translation[i];
                save->n[ii]++;
                x = data[i];
                oldM = save->M[ii];
                save->M[ii] += (x-oldM)/save->n[ii];
                save->S[ii] += (x-save->M[ii]) * (x-oldM);
            }
        }
    }
    else if (save->code_table_4_10 == DIFF1 || save->code_table_4_10 == DIFF2) {
        if (save->n_fields == 0) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii)
#endif
            for (i = 0; i < ndata; i++) {
                ii = translation == NULL ? i : translation[i];
                save->first[ii] = data[i];
            }
        }
#ifdef USE_OPENMP
#pragma omp parallel for private(i,ii)
#endif
        for (i = 0; i < ndata; i++) {
            ii = translation == NULL ? i : translation[i];
            save->last[ii] = data[i];
        }
    }
    save->n_fields += 1;
    if (save->n_fields == 1) {
        save->nx = nx;
        save->ny = ny;
        save->npnts = ndata;
        save->use_scale = use_scale;
        save->dec_scale = dec_scale;
        save->bin_scale = bin_scale;
        save->wanted_bits = wanted_bits;
        save->max_bits = max_bits;
        save->grib_type = grib_type;
    }
    save->n_missing += missing;

    // update current reference time and current verf time
    Get_time(sec[1]+12,&(save->ref_time));
    Verf_time(sec, &(save->verf_time));

    return 0;
}

/** Determines case for ave_pdt. 
 *
 * Two cases are considered for ave_pdt:
 *  1. ave_pdt is different from pdt
 *  2. ave_pdt is the same as pdt .. extend time specification
 *
 * case 1: ave_pdt < PDT_TYPE2
 * case 2: ave_pdt = (ave_pdt + PDT_TYPE2)
 */
#define PDT_TYPE2		131072  
#define PDT_MIN			0   /**< Minimum value for PDT. */
#define PDT_MAX			65535 /**< Maximum value for PDT. */

/**
 * Performs the average calculation based on the data stored in the ave_struct.
 *
 * @param save Pointer to the ave_struct containing the data.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 *
 * @author Wesley Ebisuzaki @date 4/2009
 */
static int do_ave(struct ave_struct *save) {
    int n, pdt, ave_pdt, ave_len;
    unsigned int i, ndata;
    float *data;
    double factor;
    unsigned char sec4[SET_PDT_SIZE], *sec[9], *p, *p_old;

    if (save->has_val == 0 || save->n_fields == 0) return 0; 

    ndata = save->npnts;
    if ((data = (float *) malloc(sizeof(float) * ((size_t) ndata))) == NULL) 
            fatal_error("time_processing: do_ave memory allocation","");

    if (save->code_table_4_10 == AVE) {
        factor = 1.0 / save->n_fields;
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            data[i] = (save->n[i] == save->n_fields) ? factor * save->sum[i] : UNDEFINED;
        }
    }
    else if (save->code_table_4_10 == RMS) {
        factor = 1.0 / save->n_fields;
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            data[i] = (save->n[i] == save->n_fields) ? sqrt(factor * save->sum[i]) : UNDEFINED;
        }
    }
    else if (save->code_table_4_10 == STD_DEV) {
        if (save->n_fields > 1) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                data[i] = (save->n[i] == save->n_fields) ?  sqrt(save->S[i]/(save->n_fields - 1)) 
                    : UNDEFINED;
            }
        }
        else {
            for (i = 0; i < ndata; i++) {
                data[i] = UNDEFINED;
            }
        }
    }
    else if (save->code_table_4_10 == DIFF1) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(save->first[i]) && DEFINED_VAL(save->last[i])) {
                data[i] = save->last[i] - save->first[i];
            }
            else data[i] = UNDEFINED;
        }
    }
    else if (save->code_table_4_10 == DIFF2) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(save->first[i]) && DEFINED_VAL(save->last[i])) {
                data[i] = save->first[i] - save->last[i];
            }
            else data[i] = UNDEFINED;
        }
    }
    else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            data[i] = (save->n[i] != save->n_fields) ? UNDEFINED : save->sum[i];
        }
    }

    pdt = GB2_ProdDefTemplateNo(save->first_sec);
    for (i = 0; i < 9; i++) sec[i] = save->first_sec[i];
    sec[4] = sec4;
//fprintf(stderr,"do_ave 0: pdt=%d\n", pdt);

    // average of an analysis or forecast

    ave_pdt = -1;
    ave_len = -1;

    if (pdt == 0) ave_pdt = 8;
    else if (pdt == 1) ave_pdt = 11;
    else if (pdt == 2) ave_pdt = 12;
    else if (pdt == 5) ave_pdt = 9;
    else if (pdt == 6) ave_pdt = 10;
    else if (pdt == 8) ave_pdt = 8 + PDT_TYPE2;
    else if (pdt == 9) ave_pdt = 9 + PDT_TYPE2;
    else if (pdt == 10) ave_pdt = 10 + PDT_TYPE2;
    else if (pdt == 11) ave_pdt = 11 + PDT_TYPE2;
    else if (pdt == 12) ave_pdt = 12 + PDT_TYPE2;
    else if (pdt == 46) ave_pdt = 46 + PDT_TYPE2;
    else if (pdt == 48) ave_pdt = 46;
    else if (pdt == 60) ave_pdt = 61;

    if (ave_pdt >= PDT_MIN && ave_pdt <= PDT_MAX) {
        // sec4 = new pdt with statistical processing
        i = new_pdt(save->first_sec, sec4, ave_pdt, -1, 1, NULL);

        /* save verf time */
        p = stat_proc_verf_time_location(sec);
        Save_time(&(save->verf_time), p);
        p += 7;

        //  write statistical processing 
        *p++ = 1;				// number of time ranges
        uint_char(save->n_missing, p);
        p += 4;
        *p++ = save->code_table_4_10;		// code table 4.10: average
        *p++ = save->code_table_4_11;		// code table 4.11: rt++
        *p++ = save->dt_unit;			// total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), p);
        p += 4;
        *p++ = save->dt_unit;					// time step
        uint_char(save->dt, p);
    }

    // average of an average or accumulation

    else if (ave_pdt >= PDT_TYPE2 + PDT_MIN && ave_pdt <= PDT_TYPE2 + PDT_MAX) {

        ave_len = GB2_Sec4_size(save->first_sec) + 12;
        i = new_pdt(save->first_sec, sec4, ave_pdt, ave_len, 1, NULL);

        /* update verfification time */
        p = stat_proc_verf_time_location(sec);
        Save_time(&(save->verf_time), p);

        // new statistical processing 

        p_old = stat_proc_verf_time_location(save->first_sec);

        p += 7;
        p_old += 7;

        *p++ = (n = *p_old++) + 1;			// number of time ranges
        uint_char(save->n_missing, p);
        p += 4;
        p_old += 4;

        // new time range

        *p++ = save->code_table_4_10;		// code table 4.10: average
        *p++ = save->code_table_4_11;		// code table 4.11: rt++
        *p++ = save->dt_unit;			// total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), p);
        p += 4;
        *p++ = save->dt_unit;					// time step
        uint_char(save->dt, p);
        p += 4;

        // copy the old time ranges
        for (i = 0; i < 12*n; i++) *p++ = *p_old++;
    }
    else {
        fatal_error_i("time_processing: do_ave prog error pdt=%d",pdt);
    }


    // write grib file
    p = save->first_sec[4];
    save->first_sec[4] = sec4;

    grib_wrt(save->first_sec, data, ndata, save->nx, save->ny, 
	save->use_scale, save->dec_scale, save->bin_scale, 
	save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    if (flush_mode) fflush_file(&(save->out));
    save->first_sec[4] = p;
    free(data);
    return 0;
}

/*
 * HEADER:000:time_processing:output:4:average X=CodeTable 4.10 Y=CodeTable 4.11 Z=time step A=output
 */
/**
 * Performs a superset of the  capablities of -ave and -fcst_ave. The new -ave and -fcst_ave 
 * options just call the -time_processing option. 
 * 
 * Product Definintion Templates (PDT) come in 3 variaties. Type A is an uncommon PDT and only 
 * has a reference time with no forecast information. Typically they are for observations such 
 * as radar or satellite data. Type B has a reference time with a forecast time. For example,
 * the reference time could be January 1, 2000 at 00Z. The forecast time could be a 120 hour
 * forecast. Typically the reference time is initial time of the forecast but Table 1.2 allows 
 * you to specify the significance of the reference time. Type B specifies a point in time. 
 * Finally Type C is similar to Type B except it is for a continous or non-continuous time 
 * interval. Some examples of type C PDTs,
 *
 * - average temperature of a 0-6 hour forecast
 * - maximum temperature of 00Z-24Z observations taken every hour
 * - maximum temperature of 00Z-24Z observations (as from a min-max thermometer)
 * - average 00Z temperature for the month of May (analysis)
 * - standard deviation of 00Z temperature of month of May 120 hour forecast
 * - 30 year climatology of average June temperature analyses
 * 
 * The interesting part of Type C PDTs is that the format is in terms of a stackable temporal 
 * operator. For example, a simple height forecast (ex 12 hour forecast) can be thought of as 
 * a height (time1, time2) where time1 is the intitial time of the forecast and time2 is the 
 * forecast time. The possible temporal averaging operators can be 
 * 
 *      ave1 = 1/N(H(t0,t1) + H(t0+dt,t1) + ... + H(t0+(N-1)*dt,(N-1)))
 *      ave2 = 1/N(H(t0,t1) + H(t0,t1+dt) + ... + H(t0,(N-1)*dt))
 *      ave3 = 1/N(H(t0,t1) + H(t0+dt,t1-dt) + ... + H(t0+(N-1)*dt, t1-(N-1)*dt))
 *      ave4 = 1/N(H(t0,t1) + H(t0-dt,t1+dt) + ... + H(t0-(N-1)*dt, t1+(N-1)*dt))
 * 
 * The continous case is denoted by having dt == 0.
 * 
 * By stacking the averaging operator, a monthly climatology can be described by a 30 year 
 * average of a monthly average.
 * 
 * The Type C PDTs allow 11 standard temporal operations (code table 4.10) which include the 
 * commonly used average, accumulation, maximum, minimum, RMS and standard deviation. 
 * 
 * The -time_processing option will allow you to calculate the 
 * average/minimum/maximum/standard_deviation of a time series of Type B or Type C PDTs. Only 
 * a limited number of operators are supported. The total set is given by [Code Table 4.10]
 * (https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-10.shtml). Only a 
 * limited number of types of time intervals as defined by [Code Table 4.11]
 * (https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-11.shtml) are 
 * supported. 
 * 
 * ## Usage
 * -time_processing Code_Table_4.10 Code_Table_4.11 (time interval)  (output grib file)
 * 
 * Code_Table_4.10
 * - 0 or ave, computes average
 * - 2 or max, computes maximum
 * - 3 or min, computes minimum
 * - 5 or rms, computes the root mean square
 * - 6 or std_dev, computes sample standard deviation
 * 
 * Code_Table_4.11
 * - 1 or analyses,  time series of set of analyses/forecasts
 * - 2 or forecast,  time series from one (long) forecast
 * <pre>
 * (time interval):  (integer)(units)
 * 
 * units:            hr, dy, mo, yr, mn
 * </pre>
 * 
 * ### Code Table 4.11 = 1 (analyses)
 * When Code Table 4.1 is set to 1, the input fields have to be processed in a special order. 
 * Suppose that you want to make a daily average from 4 analyses spaced at 6 hours. Then you 
 * process 4 fields with reference times incrementing by 6 hours with the same forecast time, 
 * variable, level and grid. Whenever the field is unexpected, a new average is made. Note: 
 * the -time_processing option will handle missing fields. For example
 *
 * <pre> 
 * Code Table 4.10 = 0, Code Table 4.11 = 1
 *
 *      U500 2000-01-02 00Z             start ave
 *      U500 2000-01-02 06Z
 *      U500 2000-01-02 12Z
 *      U500 2000-01-02 18Z             end ave
 *      V500 2000-01-02 00Z             start ave
 *      V500 2000-01-02 06Z
 *      V500 2000-01-02 12Z
 *      V500 2000-01-02 18Z             end ave
 *      Z500 2000-01-02 00Z             start ave
 *      Z500 2000-01-02 06Z
 *      Z500 2000-01-02 12Z
 *      Z500 2000-01-02 18Z             end ave
 * 
 * Code Table 4.1 = 1 is good for making means of many analyses. 
 * </pre>
 * 
 * ### Code Table 4.11 = 2 (forecast)
 * When Code Table 4.1 is set to 1, the input fields have to be processed in a special order. 
 * Suppose that you want to make a daily average from one forecast with using forecast 
 * hour = 0, 6, 12 and 18. (The forecast for the first day.) 
 * 
 * <pre>
 * Code Table 4.10 = 0, Code Table 4.11 = 2
 * 
 *      U500 start 2000-01-02 00Z fhour=00 hours    start ave
 *      U500 start 2000-01-02 00Z fhour=06 hours
 *      U500 start 2000-01-02 00Z fhour=12 hours
 *      U500 start 2000-01-02 00Z fhour=18 hours    end ave
 *      V500 start 2000-01-02 00Z fhour=00 hours    start ave
 *      V500 start 2000-01-02 00Z fhour=06 hours
 *      V500 start 2000-01-02 00Z fhour=12 hours
 *      V500 start 2000-01-02 00Z fhour=18 hours    end ave
 *      T500 start 2000-01-02 00Z fhour=00 hours    start ave
 *      T500 start 2000-01-02 00Z fhour=06 hours
 *      T500 start 2000-01-02 00Z fhour=12 hours
 *      T500 start 2000-01-02 00Z fhour=18 hours    end ave
 * 
 * Code Table 4.11 = 2 is good for processing a single forecast run. 
 * </pre>
 * 
 * ### Code Table 4.11 = 3, 4, 5
 * These values of Code Table 4.11 are not commonly used and are not supported. 
 * 
 * ### Code Table 4.10 = 0 (average)
 * Code Table 4.11 = 0 means the -time_processing option will compute the mean. This is the 
 * most common usage. If you are trying to compute the average and some fields are missing, 
 * then average will be computed from the available fields and the missing field value will 
 * reflect the number of missing fields. If some grid values are undefined, then the average 
 * for the grid value will be undefined as there is no mechanism to so show that the average 
 * was computed with fewer fields. 
 * 
 * ### Code Table 4.10 = 2 (maximum)
 * Code Table 4.11 = 2 means the -time_processing option will find the maximum value. If you 
 * are trying to find the maximum and some fields are missing, then the maximum will be found 
 * available fields and the missing field value will reflect the number of missing fields. If 
 * some grid values are undefined, then the maximum for the grid value will be undefined as 
 * there is no mechanism to so show that the maximum was derived from fewer fields. 
 * 
 * ### Code Table 4.10 = 3 (minimum)
 * Code Table 4.11 = 3 means the -time_processing option will find the minimum value. If you 
 * are trying to find the minimum and some fields are missing, then the minimum will be found 
 * available fields and the missing field value will reflect the number of missing fields. If 
 * some grid values are undefined, then the minimum for the grid value will be undefined as 
 * there is no mechanism to so show that the minimum was derived from fewer fields. 
 * 
 * ### Code Table 4.10 = 3 (root mean square)
 * Code Table 4.11 = 3 means the -time_processing option will compute the root mean square 
 * (rms). The mean of the square of the grid values is computed. Then the square root is saved 
 * in the grib message. The value is marked as undefined if any of the individual values is 
 * undefined. 
 * 
 * ### Code Table 4.10 = 6 (standard deviation)
 * Code Table 4.11 = 6 means the -time_processing option will find the sample standard devation. 
 * Welford's method for computing the mean and variance is used because it is a one-pass scheme 
 * with the accuracy of a two-pass algorithm. 
 * 
 * ### Code Table 4.10 = 1, 4, 7 8, 9, ..
 * These are not supported. Although adding support for 1, 4, 7, 8 and 11 seems easy. However, 
 * I have not seen these values used. 
 * 
 * ### -ave (dt) (output)
 * This option has been replaced by a macro that calls "-time_processing 0 1 (dt) (output)". 
 * 
 * ### -fcst_ave (dt) (output)
 * This option has been replaced by a macro that calls "-time_processing 0 2 (dt) (output)". 
 * 
 * ## Minutes and Seconds 
 * There is no minutes time unit. Wgrib2 uses the standard GrADS names for time units which means the "mn" 
 * is the unit for minutes. Unfortunately it is too easy to confuse "mn" with the month time unit. When 
 * there is a real need for minutes, then "mn" will be added. 
 * 
 * ## Fast Averaging
 * Suppose we have a month of analyses at 3 hour intervals and want to make a monthly mean for Nov. 2014. 
 * Using the above approach, the steps would be 
 * 
 * @code{.sh}
 * $cat narr.201411????.grb2 >tmp.grb2
 * $ wgrib2 tmp.grb2 |  \
 *      sort -t: -k4,4 -k5,5 -k6,6 -k3,3 | \
 *      wgrib2 tmp.grb -i -set_grib_type c3 -ave 3hr narr.201411
 * @endcode
 * 
 * - The first line creates a file with all the data.
 * - The second line make an inventory.
 * - The third line sorts the inventory in the order for -ave to process.
 * - The fourth line makes the average by processing data in the order determined by the inventory created by line 3.
 * 
 * The above approach processes one average at a time and requires a minimal amout of memory. However, if you count 
 * the I/O operations, you find that there are 4 I/O operations for every field as well as the writes of the monthly 
 * means. In addition, the read (line 4) is random access. 
 * 
 * HPC file systems are very fast for large files that are read sequentially. On the other hand, HPC file systems are 
 * horrible for small random access reads like in the previous example. Making monthly means by averaging 3 hourly NARR 
 * data was taking about three quarters of an hour on a multi-million dollar machine. The problem was that the file 
 * system was optimized for large sequential reads rather than small random-access reads. The following shows another approach. 
 * 
 * @code{.sh}
 * $ cat narr.201411????.grb2 | \
 *      wgrib2 - \
 *          -if_fs ":HGT:200 mb:" -ave 3hr narr.201411 -endif \
 *          -if_fs ":UGRD:200 mb:" -ave 3hr narr.201411 -endif \
 *          -if_fs ":VGRD:200 mb:" -ave 3hr narr.201411 -endif \
 *          -if_fs ":TMP:200 mb:" -ave 3hr narr.201411 -endif
 * @endcode
 * 
 * - The first line copies the data in chronological order and writes it to the pipe.
 * - The second line has wgrib2 read the grib data from the pipe.
 * - The third line selects the Z200 fields and runs the averaging option on it.  We are assuming the narr.* fields only 
 * have one Z200 field and narr.201411???? puts the data into chronological order.
 * - Lines 4-6 apply the averaging option to other fields.
 * 
 * The above approach computes the mean of Z200, U200, V200 and T200 data at the same time with the use of more memory. 
 * The I/O consists of sequential read of all the files and the writes of the monthly means. The above script only creates 
 * the mean of Z200, U200, V200 and T200 but you could write a very long command line and compute the mean of all the fields 
 * in the file.
 * 
 * ### Fast Forecast Averaging
 * Sometimes one wants to average several forecasts starting from the same initial time. An example would producing a week-4 forecast.
 * @code{.sh}
 * wgrib2 $1 -match_inv | cut -f4-5 -d:  >$tmp
 * cmd="cat $* | $wgrib2 - -set_grib_type c3 "
 * while read line
 * do
 *   cmd="$cmd -if_fs '$line' -fcst_ave $dt $out "
 * done <$tmp
 * eval $cmd
 * @endcode
 * 
 * 1. $1 is the first file to average. Line 1 creates a file with the name and level for each field. It is assumed that the name and 
 * level is unique in the file.
 * 2. cmd is the command line that is being built
 * 3. loop over all the lines in file $tmp
 * 4. generate the "-if_fs/-fcst_ave" for the cmd line. Older versions of the web paged used -if but that caused problems when
 * $line included metacharacters such as parentheses.
 * 5. bash syntax to have the while loop read from $tmp
 * 6. run the command line
 * 
 * ### Monthly Climatologies
 * Once you can make an average, making a monthly climatology should be easy. Except it isn't. Here are some of the problems that I encountered. 
 * 1. February has 28 days except when it doesn't. This causes problems because wgrib2 -ave will not average 28 and 29 day intervals.
 * 2. '116@6 hour ave(anl)' includes a regex metacharacter
 * 3. the process id changed
 * 4. the subcenter changed 
 * 
 * The solutions were:
 * 
 * 1. rewrite the grib file with:
 * @code{.sh}
 *  -if_fs '116@6 hour ave(anl)' -set_ftime2 '112@6 hour ave(anl)' -endif \
 *  -if_fs '116@6 hour ave(6 hour fcst)' -set_ftime2 '112@6 hour ave(6 hour fcst)' -endif \
 *  -if_fs '116@6 hour ave(3-6 hour acc fcst)' -set_ftime2 '112@6 hour ave(3-6 hour acc fcst)' -endif \
 * @endcode
 * 2. Use -if_fs instead of -if
 * 3. rewrite the file with -set analysis_or_forecast_process_id 180
 * 4. rewrite the file with -set subcenter 0 
 * 
 * ### Limitations
 * Fast averaging has limits impossed by wgrib2. For example, there is a limit in the maximum 
 * number of -if/-if_fs clauses. Wgrib2 v2.0.6 can process up to 2000 -if and 2000 -if_fs 
 * options. Wgrib2 v2.0.6 can accept 10000 words on the command line. Since each -if_fs/-ave 
 * clause takes 5 words on the command line and you need to include the name of the input file, 
 * you get a limit of 999 -if_fs/-ave clauses. To speed up the code, the evaluation of the 
 * -if/-if_fs options are done in parallel. 
 * 
 * Fast averaging has limits impossed by the computer memory because Fast averaging uses multiple 
 * calls to the time_processing option. Each time_processing option requires computer memory. For 
 * example, to compute the average, you need to keep arrays for the running sum and number of 
 * times the sum was incremented. So it is possible for the fast averaging could use up too 
 * much memory. 
 * 
 * @param ARG4 List of function arguments set by wgrib2's main() function (see @ref ARG4). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_time_processing(ARG4) {

    struct ave_struct *save;

    int i, pdt, new_type;
    struct full_date time, ttime, verftime, reftime;
    int missing;
    char string[10];

    // initialization

    if (mode == -1) {
        save_translation = decode = 1;

        // allocate static structure

        *local = save = (struct ave_struct *) malloc( sizeof(struct ave_struct));
        if (save == NULL) fatal_error("memory allocation f_ave","");

        if (strcmp(arg1,"ave") == 0) save->code_table_4_10 = AVE;
        else if (strcmp(arg1,"max") == 0) save->code_table_4_10 = MAX;
        else if (strcmp(arg1,"min") == 0) save->code_table_4_10 = MIN;
        else if (strcmp(arg1,"rms") == 0) save->code_table_4_10 = RMS;
        else if (strcmp(arg1,"stddev") == 0) save->code_table_4_10 = STD_DEV;
        else save->code_table_4_10 = atoi(arg1);

        i = atoi(arg2);
        if (strncmp(arg2,"analyses",4) == 0 || i == 1) save->code_table_4_11 = 1;
        else if (strncmp(arg2,"forecast",4) == 0 || i == 2) save->code_table_4_11 = 2;
        else fatal_error("time_processing: code_table_4.11 must be 1/2 or analyses/forecast not %s", arg2);

        i = sscanf(arg3, "%d%2s", &save->dt,string);
        if (i != 2) fatal_error("time_processing: delta-time: (int)(2 characters) %s", arg3);

        save->dt_unit = string2time_unit(string);
        if (save->dt_unit == -1) fatal_error("time_processing: unsupported time unit %s", string);

        if (fopen_file(&(save->out), arg4, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg4);
        }
        save->has_val = 0;
        save->n = NULL;
        save->sum = NULL;
        save->M = NULL;
        save->S = NULL;
        save->first = NULL;
        save->last = NULL;
        init_sec(save->first_sec);
        init_sec(save->next_sec);

        return 0;
    }

    save = (struct ave_struct *) *local;

    if (mode == -2) {			// cleanup
        if (save->has_val == 1) do_ave(save);
        fclose_file(&(save->out));
        free_ave_struct(save);
        return 0;
    }

    if (mode < 0) return 0;

    // 1/2015 use_scale = 0;
    pdt = GB2_ProdDefTemplateNo(sec);
    if (mode == 98) fprintf(stderr,"time_processing: pdt=%d\n",pdt);


    if (pdt != 0 && pdt != 1 && pdt != 2 && pdt != 5 && pdt != 6 && pdt != 8 && pdt != 9 &&
        pdt != 10 && pdt != 11 && pdt != 12 && pdt != 46 && pdt != 48 && pdt != 60) return 0;

    if (mode == 98) fprintf(stderr,"time_processing 1: pdt=%d\n",pdt);

    // check to see continuation of previous averaging

    new_type = 0;
    missing = 0;

    if (save->has_val == 0) new_type = 1; // first time

    // check timing stamp
    // set missing and new_type
    if (mode == 98) fprintf(stderr, "time_processing: missing calculation\n");

    if (new_type == 0) {
        if (save->code_table_4_11 == 1) {		// analyses: ref time++, verf_time = ++
            // get the reference time of field
            Get_time(sec[1]+12, &reftime);

            // get the reference time of last field
            ttime = save->ref_time;
            Add_time(&ttime, save->dt, save->dt_unit);
            while ((i=Cmp_time(&ttime, &reftime)) < 0) {
                missing++;
                Add_time(&ttime, save->dt, save->dt_unit);
            }
            if (i != 0) {
                new_type = 1;
                if (mode == 98) fprintf(stderr, "time_processing: no match - reference time code table 4.11=%d\n", 
                save->code_table_4_11);
            }

            // make sure verf time is as expected
            if (Verf_time(sec, &verftime) != 0) fatal_error("Ave: no verf time?","");
            ttime = save->verf_time;
            Add_time(&ttime, (missing+1)*save->dt, save->dt_unit);
            if (Cmp_time(&ttime, &verftime)) {
                new_type = 1;
                if (mode == 98) fprintf(stderr, "time_processing: no match - verf time\n");
            }
        }
        else if (save->code_table_4_11 == 2) {		// analyses: ref time = constant, verf_time++
            // see if reference times match 
            Get_time(sec[1]+12, &time);
            if (Cmp_time(&time, &(save->ref_time0))) {
                new_type = 1;
                if (mode == 98) fprintf(stderr, "time_processing: no match - reference time code table 4.11=%d\n", 
                save->code_table_4_11);
            }
            if (new_type == 0) {
                    if (Verf_time(sec, &time) != 0) fatal_error("Ave: no verf time?","");
                    // get the verf time of last field
                    ttime = save->verf_time;
                    Add_time(&ttime, save->dt, save->dt_unit);
                    while ((i=Cmp_time(&ttime, &time)) < 0) {
                        missing++;
                        Add_time(&ttime, save->dt, save->dt_unit);
                    }
                    if (i != 0) new_type = 1;
            }
        }
    }


    if (mode == 98) fprintf(stderr, "time_processing: code 4.11 %d compare ref time new_type = %d missing=%d\n", 
                            save->code_table_4_11,new_type, missing);

    if (new_type == 0) {
        // at this time, reference time is ok, check sections 1-3
        if (same_sec0(sec,save->first_sec) == 0 || same_sec1_not_time(mode,sec,save->first_sec) == 0 ||
                same_sec3(sec,save->first_sec) == 0) {
            new_type = 1;
            if (mode == 98) fprintf(stderr, "time_processing: testsec same_sec0=%d same_sec1_not_time=%d same_sec3=%d\n", 
                same_sec0(sec,save->first_sec),
                same_sec1_not_time(1,sec,save->first_sec),
                same_sec3(sec,save->first_sec));
        }
    }
    if (new_type == 0) {
        if (same_sec4_not_time(mode, sec,save->first_sec) == 0) {
            new_type = 1;
            if (mode == 98) fprintf(stderr, "time_processing: testsec same_sec4_not_time=%d\n", 
                                    same_sec4_not_time(0, sec,save->first_sec));
        }
    }

    if (mode == 98) fprintf(stderr, "time_processing: passed sec check new_type %d\n", new_type);

    // if data is the same as the previous, update the sum
    if (new_type == 0) {		// update sum
        if (mode == 98) fprintf(stderr, "time_processing: update\n");
        add_to_ave_struct(save, sec, data, ndata, missing);
        return 0;
    }

    // new field, do grib output and save current data
    if (save->has_val == 1) {
        do_ave(save);
    }
    init_ave_struct(save, ndata);
    add_to_ave_struct(save, sec, data, ndata, 0);
    copy_sec(sec, save->first_sec);
    copy_sec(sec, save->next_sec);

    // ref_time0 = reference time of 1st file (lowest ref time)
    // ref_time = current reference time
    // verf_time = verification time

    Get_time(sec[1]+12,&(save->ref_time0));
    save->ref_time = save->ref_time0;
    if (Verf_time(sec, &(save->verf_time)) != 0) 
        fatal_error("time_processing: could not determine the verification time","");
    return 0;
}
