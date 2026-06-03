/** @file
 * @brief Functions for Ensemble Quality Control (QC) processing in wgrib2.
 * @author Public Domain: Wesley Ebisuzaki @date 01/2020
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 01/2020 | W. Ebisuzaki | Initial public release
 * 09/2023 | W. Ebisuzaki | Faster by using SIMD
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * ens_qc
 *
 *  v1.0  process ensemble for mean, spread, min, max and scaled extremes
 *        which wgrib2 defines as max((max-mean), abs(min-mean)) / spread
 *        other people can define it differently.
 *
 *        If you want mean and spread, this is rooutine is faster than 
 *        -ens_processing because that routine sorts the data.
 *
 *        This routine was written because an ensemble data assimilation had one member 
 *        that had ozone many orders of magnitude larger than expected.  The reqults
 *        were not reproducable, and not seen in sixty years of data assimilation.
 *        So it must have been a glitch in one of the nodes.  So this routine was
 *        written to scan for ensemble members that were N times greater than spread.
 *
 * Output:
 *       arg1: grib2 of ensemble min, max, mean and spread
 *       arg2: grib2 of scaled extreme value, max((max-mean), (mean-min)) / spread
 *           saved in 8 bit precision
 *       arg3: text file with the maximum value on the grid of the scaled extremee value
 *
 * Note: arg2 is stored in grib2 with EXTREME_FORECAST_INDEX
 *       EXTREME_FORECAST_INDEX is a local NCEP definition, so the scaled extremes will 
 *       only bw written in grib format if the center is NCEP.
 *
 * Input:
 *       arg4: N  for future use, such as different types of QC
 *             1  is the only currently supported value
 *
 * Note: Code saves each grid before doing the QC code.  This is not needed for the
 *       current code.  However, future versions of the QC code may want access to the
 *       individual grids.
 *
 * 12/2021: Public Domain: Wesley Ebisuzaki
 *  1/2020: Initial public release
 *  9/2023: faster by using SIMD
 *  9/2025: fix race condition: could not observe race condition in multiple attempts
 *             found by code examination, vulnerability may have been reduced
 *             by code optimizer.
 */

/** Code Figure for Average in Code Table 4.7 */
#define AVE	0

/** Code Figure for Spread in Code Table 4.7 */
#define SPREAD	4

/** Code Figure for Maximum in Code Table 4.7 */
#define MAX	9

/** Code Figure for Minimum in Code Table 4.7 */
#define MIN	8

/** Extreme Forecast Index (only for NCEP files) */
#define EXTREME_FORECAST_INDEX 199

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Grid size in x direction. */
extern int nx;

/** Grid size in y direction. */
extern int ny;

/** Save translation flag. */
extern int save_translation;

/** Flush output flag. */
extern int flush_mode;

/** Translation array for grid points. */
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

/** Output GRIB type. */
extern enum output_grib_type grib_type;

/** Struct for ensemble quality control */
struct ens_qc_struct {
    unsigned int npnts;                 /**< Number of points in the grid. */
    unsigned int nx;                    /**< Grid size in x direction. */
    unsigned int ny;                    /**< Grid size in y direction. */
    int has_val;                        /**< Flag indicating if values are present. */
    int n_ens;                          /**< Number of ensemble members. */
    unsigned char *first_sec[9];        /**< Array containting first section. */
    struct full_date verf_date;         /**< Verification date. */
    int use_scale;                      /**< Use scaling flag. */
    int dec_scale;                      /**< Decimal scaling. */
    int bin_scale;                      /**< Binary scaling. */
    int wanted_bits;                    /**< Number of bits wanted. */
    int max_bits;                       /**< Maximum number of bits. */
    enum output_grib_type grib_type;    /**< Output GRIB type. */
    struct seq_file out;                /**< Output sequence file. */
    struct seq_file extreme_grb;        /**< Output sequence file for extreme GRIB. */
    struct seq_file extreme_txt;        /**< Output sequence file for extreme TXT. */
    float *grids;                       /**< Array holding grids for median-type calculations. */
    int ngrids;                         /**< Number of grids allocated. */
    double max_err;                     /**< Maximum error. */
};

static int wrt_ens_qc(unsigned char **sec, struct ens_qc_struct *save);
static int free_ens_qc_struct(struct ens_qc_struct *save);
static int init_ens_qc_struct(struct ens_qc_struct *save, unsigned char **sec, float *data, unsigned int ndata);
static int update_ens_qc_struct(struct ens_qc_struct *save, unsigned char **sec, float *data, unsigned int ndata);

/**
 * Free memory allocated for ensemble quality control structure.
 * 
 * @param save Pointer to the ensemble quality control structure to be freed.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2020
 */
static int free_ens_qc_struct(struct ens_qc_struct *save) {
    free_sec(save->first_sec);
    if (save->has_val == 1) {
        if (save->ngrids) {
            free(save->grids);
        }
    }
    free(save);
    return 0;
}

/**
 * Initialize ensemble quality control structure with given parameters.
 *
 * @param save Pointer to the ensemble quality control structure to be initialized.
 * @param sec Pointer to the section data.
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2020
 */
static int init_ens_qc_struct(struct ens_qc_struct *save, unsigned char **sec, 
            float *data, unsigned int ndata) {
    unsigned int i;

    /* if allocated but wrong size, free all */
    if (save->has_val == 1 && save->npnts != ndata) {
        if (save->ngrids) {
            free(save->grids);
            save->ngrids=0;
        }
        save->has_val = 0;
    }

    /* if not allocated, allocate */
    if (save->has_val == 0) {
        save->grids = malloc(((size_t) ndata) * ENS_PROCESSING_NGRID0 * sizeof(float));
        if (save->grids == NULL) fatal_error("ens_qc: memory allocation problem","");
        save->ngrids = ENS_PROCESSING_NGRID0;
    }

    save->npnts = ndata;
    save->has_val = 1;
    save->nx = nx;
    save->ny = ny;
    save->use_scale = use_scale;
    save->dec_scale = dec_scale;
    save->bin_scale = bin_scale;
    save->wanted_bits = wanted_bits;
    save->max_bits = max_bits;
    save->grib_type = grib_type;
    free_sec(save->first_sec);
    copy_sec(sec, save->first_sec);
    Verf_time(sec, &(save->verf_date));
//    fprintf(stderr,"verf_date %d-%d %d %d\n", save->verf_date.year, save->verf_date.month,
//       save->verf_date.day, save->verf_date.hour);

    if (translation == NULL) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
        for (i = 0; i < ndata; i++) {
            save->grids[i] = data[i];
        }
    }
    else {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
        for (i = 0; i < ndata; i++) {
            save->grids[translation[i]] = data[i];
        }
    }
    save->n_ens = 1;
    return 0;
}

/**
 * Save grid in memory.
 * 
 * @param save Pointer to the ensemble quality control structure.
 * @param sec Pointer to the section data.
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 *
 * @return 0 on success.
 *
 * @author Wesley Ebisuzaki @date 01/2020
 */
static int update_ens_qc_struct(struct ens_qc_struct *save, unsigned char **sec, 
        float *data, unsigned int ndata) {

    unsigned int i;

    if (save->npnts != ndata) fatal_error("ens_qc: size mismatch in update","");

    if (save->n_ens == save->ngrids) {		/* need to make save->grids bigger */
        save->ngrids *= ENS_PROCESSING_NGRID_FACTOR;
        save->grids = realloc(save->grids, ((size_t) save->ngrids) * save->npnts * sizeof (float));
        if (save->grids == NULL) {
            /* if realloc fails, original memory is retained .. some memory is lost here, don't care */
            save->ngrids = 0;
            save->has_val = 0;
            fatal_error("ens_qc: memory allocation in update","");
        }
    }

    /* the data needs to be translated from we:sn to raw, need to 
       do it now because translation[] may be different in finalized phase */

    if (translation == NULL) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
        for (i = 0; i < ndata; i++) {
            save->grids[i+save->n_ens*ndata] = data[i];
        }
    }
    else {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
    for (i = 0; i < ndata; i++) {
        save->grids[translation[i]+save->n_ens*ndata] = data[i];
    }
    }
    save->n_ens++;
    return 0;
}

/**
 * Write ensemble statistics to output.
 *
 * @param sec Pointer to the section data.
 * @param save Pointer to the ensemble quality control structure.
 *
 * @return 0 on success.
 *
 * @author Wesley Ebisuzaki @date 01/2020
 */
static int wrt_ens_qc(unsigned char **sec, struct ens_qc_struct *save) {
    int pdt, pdt_ens, k, k_0;
    unsigned int i, ndata;
    float *datamin, *datamax, *datavar, *datamean, *dataextreme, maxextreme;
    float minval, maxval, tmpf;
    double tmp, sum, sq;
    int n, n_grids;
    unsigned char sec4[SET_PDT_SIZE];
    char string[3*STRING_SIZE];
    unsigned char *new_sec[9], *p;
    unsigned char *table_4_7;
    char name[STRING_SIZE], level[STRING_SIZE];

    /* used by call to f_lev */
    int mode;
    float *data;

    if (save->has_val == 0 || save->n_ens == 0) return 0;

    pdt = GB2_ProdDefTemplateNo(save->first_sec);

    switch(pdt) {
        case 0:
        case 1:
                pdt_ens = 2; break;
        case 8:
        case 11:
                pdt_ens = 12; break;
        default:
                pdt_ens = -1; break;
    }
    if (pdt_ens == -1) return 0;

    /* create new_pdt (sec4) */

    if (new_pdt(save->first_sec, sec4, pdt_ens, -1, 1, NULL)) 
        fatal_error("ens_qc: new_pdt failed","");

    /* make a new sec[][] */
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
    for (i = 0; i < 9; i++) new_sec[i] = save->first_sec[i];
    new_sec[4] = sec4;

    p = number_of_forecasts_in_the_ensemble_location(new_sec);
    if (p != NULL) *p = save->n_ens < 254 ? save->n_ens : 255;

    /* do not change code table 4_3 */
    table_4_7 = code_table_4_7_location(new_sec);
    if (table_4_7 == NULL) fatal_error("ens_qc: program error missing table 4.7","");

    getName(save->first_sec, 0, NULL, name, NULL, NULL);
    *level = 0;
    mode = 0;
    data = NULL;
    ndata = save->npnts;            /* to remove compiler complaints */
    f_lev(call_ARG0(level,NULL));

    ndata = save->npnts;
    n_grids = save->n_ens; 
    maxextreme = 0.0;

    datamean = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datavar = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datamin = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datamax = (float *) malloc(sizeof(float) * ((size_t) ndata));
    dataextreme = (float *) malloc(sizeof(float) * ((size_t) ndata));

    if (datamean == NULL || datavar == NULL || datamin == NULL || datamax == NULL || dataextreme == NULL)
            fatal_error("ens_qc: memory allocation","");

#ifdef USE_OPENMP
#pragma omp parallel for private(i, k, sum, sq, k_0, n, minval, maxval, tmp, tmpf), reduction (max:maxextreme)
#endif
    for (i = 0; i < ndata; i++) {
        sum = sq = 0;
        minval = maxval = UNDEFINED;
        datamean[i] = datamax[i] = datamin[i] = dataextreme[i] = UNDEFINED;

        /* find first defined value */
        for (k_0 = 0; k_0 < n_grids; k_0++) {

	    if (DEFINED_VAL(save->grids[i+k_0*ndata])) break;
	}
	if (k_0 < n_grids) {
	   minval = maxval = sum = save->grids[i+k_0*ndata];
	   n = 1;

	   /* calculate sum, min and max */
#ifdef IS_OPENMP_4_0
#pragma omp simd private(tmpf) reduction(+:sum,n) reduction(min: minval) reduction(max: maxval)
#endif
	   for (k = k_0 + 1; k < n_grids; k++) {
		tmpf = save->grids[i+k*ndata];
		if (DEFINED_VAL(tmpf)) {
		    maxval = maxval > tmpf ? maxval : tmpf;
		    minval = minval < tmpf ? minval : tmpf;
		    sum += tmpf;
		    n++;
		}
	    }
	    datamean[i] = sum = sum / n;
	    datamin[i] = minval;
	    datamax[i] = maxval;

	    /* calculate variance */

	    sq = 0;

#ifdef IS_OPENMP_4_0
#pragma omp simd private(tmpf) reduction(+:sq)
#endif
            for (k = k_0; k < n_grids; k++) {

	        tmpf = save->grids[i+k*ndata];
                if (DEFINED_VAL(tmpf)) {
		    sq += (tmpf - sum)*(tmpf - sum);
	        }

            }
            datavar[i] = sq = sqrt(sq/n);
            if (sq > 0) {
                tmp = (datamax[i] - sum) >= -(datamin[i] - sum) ? datamax[i] - sum : -(datamin[i] - sum);
                dataextreme[i] = tmp/sq;
                maxextreme = maxextreme >= tmp/sq ? maxextreme : tmp/sq;
            }
            else {
                dataextreme[i] = UNDEFINED;
            }
        }
    } 

    *table_4_7 = MIN;
    grib_wrt(new_sec, datamin, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *table_4_7 = MAX;
    grib_wrt(new_sec, datamax, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *table_4_7 = AVE;
    grib_wrt(new_sec, datamean, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *table_4_7 = SPREAD;
    grib_wrt(new_sec, datavar, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    if (GB2_Center(new_sec) == NCEP) {
        /* EXTREME_FORECAST_INDEX has no WMO equivalent */
        *table_4_7 = EXTREME_FORECAST_INDEX;
        grib_wrt(new_sec, dataextreme, ndata, save->nx, save->ny, 
        0, 0, 0,
        8, 8, save->grib_type, &(save->extreme_grb));
    }
    sprintf(string,"%s:%s:max scaled extreme=%f\n", name, level, maxextreme);
    fwrite_file(string,1,strlen(string), &(save->extreme_txt));

    free(datamean);
    free(datavar);
    free(datamin);
    free(datamax);
    free(dataextreme);
    return 0;
}

/*
 * HEADER:000:ens_qc:output:4:simple qc ensemble members X=stats.grb Y=extreme.grb  Z=extreme.txt A=1 (qc_version)
 */

/**
 * Tests the ensemble members looking for extreme values (values that are extreme 
 * relative the spread of the ensemble members).  This test is done for each grid 
 * point. An unreasonable scaled extreme value provides a simple quality control 
 * for an egregiously bad ensemble member. 
 * 
 * During the production of the Conventional Observations REanalsyis (CORe), a 
 * glitch was found where the upper-level ozone was two orders of magnitude larger 
 * than expected. The problem was caused by a single ensemble member which had an 
 * irreproducible problem which showed up in a single layer of a tracer. The -ens_qc 
 * option was written to detect extreme values of the ensemble members for quality 
 * control. In addition, -ens_qc can be used to calculate the ensemble mean and 
 * spread faster than -ens_processing. 
 * 
 * This option will QC ensemble members which limits this option to PDTs of 0, 1, 
 * 8, and 11. PDTs of 1 and 11 are explicity ensemble members. PDTs of 0 and 8 are 
 * often used by the control runs. The code should be updated to include other 
 * ensemble PDTs. 
 * 
 * The contents of the output files will vary depending the qc_version. As of 1/2022, 
 * only qc_version 1 has been defined. 
 * 
 * 1) ens_mean,  sum(x(i))/n,  i=1..n where n is the number of ensemble members
 * 2) ens_spread, sqrt(sum((x(i)-em)**2)/n)  note: n is used rather than n-1
 * 3) ens_min = minimum value over all ensemble members (for each grid point)
 * 4) ens_max = maximum value over all ensemble members (for each grid point)
 * 5) scaled extreme value, max((ens_max-ens_mean),(ens_mean-ens_min))/ens_spread
 * 6) grid maximum of the scaled extreme value (single number)
 * 
 * (1)-(4) are written to arg1, order: (3), (4), (1), (2)
 * (5) is written to arg2
 * (6) is written to arg3
 * 
 * The calculations are done grid point by grid point. If all the members have 
 * UNDEFINED values for a particular grid point, then variables 1-5 are set to UNDEFINED 
 * for that grid point.  If the ensemble spread is zero, then the scaled extreme value 
 * is set to zero.  The calculation differs from -ens_processing which requires no missing 
 * values for all the ensemble members before calculating the various products. This
 * will affect the calculation of the mean cloud top temperature some ensemble members 
 * are missing clouds.
 * 
 * ## Memory Usage
 * This option is unlike most wgrib2 options in that this option can use large amounts 
 * of memory. Suppose that you have an 80 member ensemble and are processing the tmp500 
 * field. This option stores all 80 tmp500 fields in memory. As the size of the grid 
 * and the number of ensemble member increases, the required memory will increase. 
 * 
 * ## Code Table 4.7
 * Grib Code Table 4.7 is used to specify whether the field is the ensemble mean, spread, 
 * min, max or scaled extreme value. The ensemble mean, spread, min and max are WMO defined 
 * values. For the scaled extreme value, wgrib2 is using the locally defined NCEP "extreme 
 * forecast index". Since the NCEP documentation doesn't define the index, wgrib2 will be 
 * using the scaled extreme value. So other uses of the "extreme forecast index" will differ.
 * The scaled extreme values are stored in 8 bits precision (two digits), and can only be 
 * written if the original file is from NCEP because there is no equivalent WMO code for 
 * extreme value (1/2022). 
 * 
 * ## Definition of an Ensemble Member
 * The -ens_qc option uses the same definition of ensemble member as -ens_processing. Any grib 
 * message which is not identified as an ensemble member is ignored by -ens_qc. So you cannot 
 * use -ens_qc on the ensemble-mean. The wgrib2 code [ed. written 3/2022 v3.1.1] that identifies 
 * ensemble member is unreasonably simplistic. Ensemble members are identified by having a PDT of 
 * 0, 1, 8 and 11. As a result, aerosols, chemical tracers, simulated satellite data, and 
 * post-processing of ensemble members are ignored.
 * 
 * ## Order of Fields
 * The -ens_qc requires fields to be in a specific order, the same order as in -ens_processing. 
 * Please see the documentation for -ens_processing. 
 * 
 * ## Usage
 * -ens_qc FILE1 FILE2 FILE3 QC_VERSION
 * 
 * FILE1 = ensemble min, ensemble max, ensemble mean, ensemble spread
 *      output in grib2 format
 * FILE2 = if center == NCEP, (ensemble) scaled extreme value
 *      output in grib2 format
 * FILE3 = grid maximum of the (ensemble) scaled extreme value
 *      text, single line per field
 * QC_VERSION = the type of QC to be run
 *      1  only acceptable value (1/2021)
 * 
 * @param ARG4 List of function arguments set by wgrib2's main() function (see @ref ARG4). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ gmerge - sfg_2014060500_fhr06_mem0* | wgrib2 - -ens_qc out1 out2 out3 1
 * 1:0:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ENS=+1
 * 2:183334:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ENS=+2
 * 3:366283:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ENS=+3
 * 4:549841:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ENS=+4
 * 5:733631:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ENS=+5
 * ...
 * @endcode
 * 
 * <pre>
 *   gmerge - (list of ensemble members)
 *           The output file is "-" which is a convention for stdin/stdout depending 
 *             on the expectation. Since we expecting an output file, the "-" is stdout.
 *           Takes the 1st grib message from each ensemble member and writes it to stdout.
 *           Takes the 2nd grib message from each ensemble member and writes it to stdout.
 *           etc
 *           The gmerge step puts the data into proper order assuming,
 *              the grib file contains no sub-messages,
 *              the indivdual grib files are in the same order.
 *           gmerge is not Windows compatilble.
 * 
 *   wgrib2 - -ens_qc out1 out2 out3 1
 *           The input file is "-" which is a convention for stdin/stdout depending 
 *             on the expectation. Since we expecting an input file, the "-" is stdin.
 *           writes min, max, mean and spread to out1
 *           writes scaled extreme values to out2
 *           writes grid maximum scale extreme value to out3
 * </pre>
 * 
 * Note: piping grib data to stdout and from stdin should not work in Windows. I have seen it work, and I have 
 * seen it fail. My standard practice is to avoid pipes in Windows.
 * 
 * @code{.sh}
 * $ wgrib2 out1
 * 1:0:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:min all members
 * 2:262325:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:max all members
 * 3:524650:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ens mean
 * 4:786975:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:ens spread
 * 5:1000148:d=2014060412:UGRD:1-2 hybrid pressure layer:12 hour fcst:min all members
 * ...
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 out2
 * 1:0:d=2014060412:UGRD:0-1 hybrid pressure layer:12 hour fcst:extreme forecast index
 * 2:131253:d=2014060412:UGRD:1-2 hybrid pressure layer:12 hour fcst:extreme forecast index
 * 3:262506:d=2014060412:UGRD:2-3 hybrid pressure layer:12 hour fcst:extreme forecast index
 * ...
 * @endcode
 * 
 * @code{.sh}
 * $ cat out3
 * UGRD:0-1 hybrid pressure layer:max scaled extreme=7.454012
 * UGRD:1-2 hybrid pressure layer:max scaled extreme=8.029185
 * UGRD:2-3 hybrid pressure layer:max scaled extreme=7.936052
 * ...
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 01/2020
 */
int f_ens_qc(ARG4) {
    struct ens_qc_struct *save;
    struct full_date verf_date;
    int pdt, new_type;

    if (mode == -1) {
        save_translation = decode = 1;

        if (atoi(arg4) != 1) fatal_error("ens_qc: this version wgrib2 does not supported qc_version=%s",arg4);
        
        // allocate static structure

        *local = save = (struct ens_qc_struct *) malloc( sizeof(struct ens_qc_struct));

        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("ens_qc: Could not open arg1 %s", arg1);
        }
        if (fopen_file(&(save->extreme_grb), arg2, file_append ? "ab" : "wb") != 0) {
            fclose_file(&(save->out));
            free(save);
            fatal_error("ens_qc: Could not open arg2 %s", arg2);
        }
        if (fopen_file(&(save->extreme_txt), arg3, file_append ? "ab" : "wb") != 0) {
            fclose_file(&(save->out));
            fclose_file(&(save->extreme_grb));
            free(save);
            fatal_error("ens_qc: Could not open arg3 %s", arg3);
        }

        save->has_val = 0;
        save->n_ens = 0;
        save->grids = NULL;
        save->ngrids = 0;
        init_sec(save->first_sec);
    
        return 0;
    }
    save = (struct ens_qc_struct *) *local;
    if (mode == -2) {    /* cleanup */
        wrt_ens_qc(save->first_sec, save);
        fclose_file(&(save->out));
        fclose_file(&(save->extreme_grb));
        fclose_file(&(save->extreme_txt));
        free_ens_qc_struct(save);
        return 0;
    }

    if (mode < 0) return 0;

    /* processing a record */
    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt != 0 && pdt != 1 && pdt != 8 && pdt != 11) return 0;

    // check to see continuation of previous averaging

    new_type = (save->has_val == 0) ? 1 : 0;

    /* get verification date */
    if (new_type == 0) {
        Verf_time(sec, &(verf_date));
        if (Cmp_time(&(verf_date), &(save->verf_date)) != 0) {
            new_type = 1;
// fprintf(stderr,"failed verf date %d-%d %d %d\n", verf_date.year, verf_date.month, verf_date.day, verf_date.hour);
        }
    }

    if (new_type == 0) {
        if (same_sec4_but_ensemble(mode, sec, save->first_sec) == 0) new_type = 1;
    }

    if (mode == 98) fprintf(stderr,": pdt=%d, new_type=%d\n",pdt, new_type);
    if (new_type == 1) {
        if (mode == 98) fprintf(stderr,": >> wrt a\n");
        wrt_ens_qc(save->first_sec, save);
        if (mode == 98) fprintf(stderr,": << wrt a\n");
        init_ens_qc_struct(save, sec, data, ndata);
    }
    else {
        if (mode == 98) fprintf(stderr,": >> update\n");
        update_ens_qc_struct(save, sec, data, ndata);
    }
    return 0;
}
