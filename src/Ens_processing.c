/** @file
 * @brief Functions for ensemble processing in wgrib2. Experimental. Based on 
 * time_processing.c.
 * @author Public Domain: Wesley Ebisuzaki @date 01/2018
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 
 * code table 4.3 used by percentile and prob fcsts, 
 *   WMO: ensemble forecast
 *  NCEP: ensemble forecast based on counting
 */

/** Code Figure for WMO Process in Code Table 4.3 */
#define PROCESS 4

/** Code Figure for NCEP Process in Code Table 4.3 (based on counting) */
#define NCEP_PROCESS 199

/** Code Figure for Average in Code Table 4.7 */
#define AVE	0

/** Code Figure for Spread in Code Table 4.7 */
#define SPREAD	4

/** Code Figure for Maximum in Code Table 4.7 */
#define MAX	9

/** Code Figure for Minimum in Code Table 4.7 */
#define MIN	8

/* Trace defined to be 0.1 mm  - trace precip 0.1mm/3hours converted to mm/s */

/** Trace precipitation converted to mm/s. */
#define TRACE	(0.1/86400.0/8)

static int testfloat(const void *a, const void *b);
static double percentile_index(float percent, int n);

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

/** Struct for ensemble processing */
struct ens_proc_struct {
    unsigned int npnts;             /**< Number of points in the grid. */
    unsigned int nx;                /**< Grid size in x direction. */
    unsigned int ny;                /**< Grid size in y direction. */
    int has_val;                    /**< Flag indicating if values are present. */
    int n_ens;                      /**< Number of ensemble members. */
    unsigned char *first_sec[9];    /**< Array containting first section. */
    struct full_date verf_date;     /**< Verification date. */
    int use_scale;                  /**< Use scaling flag. */
    int dec_scale;                  /**< Decimal scaling. */
    int bin_scale;                  /**< Binary scaling. */
    int wanted_bits;                /**< Number of bits wanted. */
    int max_bits;                   /**< Maximum number of bits. */
    enum output_grib_type grib_type; /**< Output GRIB type. */
    struct seq_file out;            /**< Output sequence file. */
    float *grids;                   /**< Array holding grids for median-type calculations. */
    int ngrids;                     /**< Number of grids allocated. */
    int option;                     /**< Processing option. */
};

static int wrt_ens_proc(unsigned char **sec, struct ens_proc_struct *save);
static int free_ens_proc_struct(struct ens_proc_struct *save);
static int init_ens_proc_struct(struct ens_proc_struct *save, unsigned char **sec, float *data, unsigned int ndata);
static int update_ens_proc_struct(struct ens_proc_struct *save, unsigned char **sec, float *data, unsigned int ndata);

/**
 * Free memory allocated for ensemble processing structure.
 * 
 * @param save Pointer to the ensemble processing structure to be freed.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2018
 */
static int free_ens_proc_struct(struct ens_proc_struct *save) {
    if (save->has_val == 1) {
        free_sec(save->first_sec);
        if (save->ngrids) {
            free(save->grids);
        }
    }
    free(save);
    return 0;
}

/**
 * Initialize ensemble processing structure with given parameters.
 * 
 * @param save Pointer to the ensemble processing structure to be initialized.
 * @param sec Pointer to the section data.
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2018
 */
static int init_ens_proc_struct(struct ens_proc_struct *save, unsigned char **sec,
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
        if (save->grids == NULL) fatal_error("ens_processing: memory allocation problem","");
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
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            save->grids[i] = data[i];
        }
    }
    else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
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
 * @param save Pointer to the ensemble processing structure.
 * @param sec Pointer to the section data.
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2018
 */
static int update_ens_proc_struct(struct ens_proc_struct *save, unsigned char **sec, 
        float *data, unsigned int ndata) {

    unsigned int i;

    if (save->npnts != ndata) fatal_error("ens_processing: size mismatch","");

    if (save->n_ens == save->ngrids) {		/* need to make save->grids bigger */
        save->ngrids *= ENS_PROCESSING_NGRID_FACTOR;
        save->grids = realloc(save->grids, ((size_t) save->ngrids) * save->npnts * sizeof (float));
        if (save->grids == NULL) {
            /* if realloc fails, original memory is retained .. some memory is lost here, don't care */
            save->ngrids = 0;
            save->has_val = 0;
            fatal_error("ens_processing: memory allocation in update","");
        }
    }

    /* the data needs to be translated from we:sn to raw, need to 
       do it now because translation[] may be different in finalized phase */

    if (translation == NULL) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
        for (i = 0; i < ndata; i++) {
            save->grids[i+save->n_ens*ndata] = data[i];
        }
    }
    else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
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
 * @param save Pointer to the ensemble processing structure.
 * 
 * @return 0 on success.
 * 
 * @author Wesley Ebisuzaki @date 01/2018
 */
static int wrt_ens_proc(unsigned char **sec, struct ens_proc_struct *save) {
    int pdt, pdt_ens, pdt_probability, pdt_percentile;
    unsigned int i, ndata, k, k2;
    int j;
    float *data, *data10, *data25, *data50, *data75, *data90;
    float *datamean, *datavar, *datamin, *datamax, *dataextra, *dataextra2;
    unsigned char sec4[SET_PDT_SIZE], sec4_probability[SET_PDT_SIZE], sec4_percentile[SET_PDT_SIZE];
    unsigned char *new_sec[9], *new_sec_probability[9], *new_sec_percentile[9], *p;
    unsigned char *table_4_7;
    unsigned char *table_4_9_probability, *value_percentile;
    double sum, sq;
    double x25, x75, x10, x50, x90, x95;
    int i25, i75, i10, i50, i90, i95;
    double d25, d75, d10, d50, d90, d95;
    char name[INV_STRING_SIZE], level[INV_STRING_SIZE];
    int mode, extra, isNCEP;
    int scale_factor, scale_value;

    if (save->has_val == 0 || save->n_ens == 0) return 0; 

    pdt = GB2_ProdDefTemplateNo(save->first_sec);
    switch(pdt) {
        case 0:
        case 1:
            pdt_ens = 2; pdt_probability = 5; pdt_percentile = 6; break;
        case 8:
        case 11:
            pdt_ens = 12; pdt_probability = 9; pdt_percentile = 10; break;
        default:
            pdt_ens = -1; break;
    }
    if (pdt_ens == -1) return 0;

    extra = 0;
    if (save->option == 1) {
        getName(save->first_sec, 0, NULL, name, NULL, NULL);
        *level = 0;
        mode = 0;
        data = NULL;
        ndata = save->npnts;		/* to remove compiler complaints */
        f_lev(call_ARG0(level,NULL));
        if (strcmp(level,"2 m above ground") == 0 &&
                (strcmp(name, "TMP") == 0 || strcmp(name,"TMIN") == 0 || strcmp(name,"TMAX") == 0)) {
            extra = 1;
        }
        else if (strcmp(level,"surface") == 0 && (strcmp(name, "APCP") == 0)) {
            extra = 2;
        }
        else if (strcmp(level,"surface") == 0 && (strcmp(name, "PRATE") == 0)) {
            extra = 2;
        }
        else if (strcmp(level,"10 m above ground") == 0 && strcmp(name,"WIND") == 0) {
            extra = 3;
        }
    }

    isNCEP = GB2_Center(save->first_sec) == NCEP;

    /* create new_pdt (sec4) */

    if (new_pdt(save->first_sec, sec4, pdt_ens, -1, 1, NULL)) 
        fatal_error("ens_processing: new_pdt failed","");
    /* make a new sec[][] */
    for (i = 0; i < 9; i++) new_sec[i] = save->first_sec[i];
    new_sec[4] = sec4;

    p = number_of_forecasts_in_the_ensemble_location(new_sec);
    if (p != NULL) *p = save->n_ens < 254 ? save->n_ens : 255;

    /* do not change code table 4_3 */
    table_4_7 = code_table_4_7_location(new_sec);
    if (table_4_7 == NULL) fatal_error("ens_processing: program error 4.7","");

    /* create new_pdt_probability */

    table_4_9_probability = NULL;
    if (extra) {		/* extra .. use probability template for extras */
            /* create new_pdt for probability sec4_probability */
        if (new_pdt(save->first_sec, sec4_probability, pdt_probability, -1, 1, NULL)) 
                fatal_error("ens_processing: new_pdt probability failed","");
        for (i = 0; i < 9; i++) new_sec_probability[i] = save->first_sec[i];
        new_sec_probability[4] = sec4_probability;
        p = code_table_4_3_location(new_sec_probability);
        if (p == NULL) fatal_error("ens_processing: program error 4.3","");
        *p = isNCEP ? NCEP_PROCESS : PROCESS;
        table_4_9_probability = code_table_4_9_location(new_sec_probability);
        if (table_4_9_probability == NULL) fatal_error("ens_processing: program error 4.9","");
    }

    /* create new_pdt_percentile */
    if (new_pdt(save->first_sec, sec4_percentile, pdt_percentile, -1, 1, NULL)) 
            fatal_error("ens_processing: new_pdt percentile failed","");
    for (i = 0; i < 9; i++) new_sec_percentile[i] = save->first_sec[i];
    new_sec_percentile[4] = sec4_percentile;
    p = code_table_4_3_location(new_sec_percentile);
    if (p == NULL) fatal_error("ens_processing: program error 4.3","");
    *p = isNCEP ? NCEP_PROCESS : PROCESS;
    value_percentile = percentile_value_location(new_sec_percentile);
    if (value_percentile == NULL) fatal_error("ens_processing: program error percentile","");

    ndata = save->npnts;

    data = (float *) malloc(sizeof(float) * ((size_t) ndata));
    data10 = (float *) malloc(sizeof(float) * ((size_t) ndata));
    data25 = (float *) malloc(sizeof(float) * ((size_t) ndata));
    data50 = (float *) malloc(sizeof(float) * ((size_t) ndata));
    data75 = (float *) malloc(sizeof(float) * ((size_t) ndata));
    data90 = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datamean = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datavar = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datamin = (float *) malloc(sizeof(float) * ((size_t) ndata));
    datamax = (float *) malloc(sizeof(float) * ((size_t) ndata));
    if (data == NULL || data10 == NULL || data25 == NULL || data50 == NULL || data75 == NULL || data90 == NULL
       || datamean == NULL || datavar == NULL || datamin == NULL || datamax == NULL)
            fatal_error("ens_processing: wrt_ens_proc memory allocation","");

    dataextra = dataextra2 = NULL;
    if (extra) {
        if ((dataextra = (float *) malloc(sizeof(float) * ((size_t) ndata))) == NULL) 
                fatal_error("ens_processing: wrt_ens_proc memory allocation","");
        if (extra == 2) {
            if ((dataextra2 = (float *) malloc(sizeof(float) * ((size_t) ndata))) == NULL) 
                    fatal_error("ens_processing: wrt_ens_proc memory allocation","");
        }
    }

    /* sort grids .. for statistics  */

    x10 = percentile_index(10.0, save->n_ens);
    i10 = floor(x10);
    d10 = x10-i10;

    x25 = percentile_index(25.0, save->n_ens);
    i25 = floor(x25);
    d25 = x25-i25;

    x50 = percentile_index(50.0, save->n_ens);
    i50 = floor(x50);
    d50 = x50-i50;

    x75 = percentile_index(75.0, save->n_ens);
    i75 = floor(x75);
    d75 = x75-i75;

    x90 = percentile_index(90.0, save->n_ens);
    i90 = floor(x90);
    d90 = x90-i90;

    x95 = percentile_index(95.0, save->n_ens);
    i95 = floor(x95);
    d95 = x95-i95;

#ifdef USE_OPENMP
#pragma omp parallel for private(i,j,k,k2,sum,sq)
#endif
    for (i = 0; i < ndata; i++) {
        float ens[save->n_ens];
            
        /* make vector of ensemble member grid points */
        k = 0;
#ifdef IS_OPENMP_4_0
#pragma omp simd reduction(+:k)
#endif
        for (j = 0; j < save->n_ens; j++) {
            ens[j] = save->grids[i+j*ndata];
                if (DEFINED_VAL(save->grids[i+j*ndata])) k++;
        }

        if (k == save->n_ens) {
            /* sort */
            qsort(&(ens[0]), save->n_ens, sizeof(float), &testfloat);

            /* find the various percentiles */	    
            data10[i] = i10 != save->n_ens - 1 ? ens[i10]*(1.0-d10) + ens[i10+1]*d10 : ens[i10];
            data25[i] = i25 != save->n_ens - 1 ? ens[i25]*(1.0-d25) + ens[i25+1]*d25 : ens[i25];
            data50[i] = i50 != save->n_ens - 1 ? ens[i50]*(1.0-d50) + ens[i50+1]*d50 : ens[i50];
            data75[i] = i75 != save->n_ens - 1 ? ens[i75]*(1.0-d75) + ens[i75+1]*d75 : ens[i75];
            data90[i] = i90 != save->n_ens - 1 ? ens[i90]*(1.0-d90) + ens[i90+1]*d90 : ens[i90];
            datamin[i] = ens[0];
            datamax[i] = ens[save->n_ens - 1];

            sum = sq = 0.0;
#ifdef IS_OPENMP_4_0
#pragma omp simd reduction(+:sum)
#endif
            for (j = 0; j < save->n_ens; j++) {
                sum += ens[j];
            }
            sum = sum / save->n_ens;
            datamean[i] = sum;

#ifdef IS_OPENMP_4_0
#pragma omp simd reduction(+:sq)
#endif
            for (j = 0; j < save->n_ens; j++) {
                sq += (ens[j]-sum)*(ens[j]-sum);
            }
            datavar[i] = sqrt(sq/save->n_ens);

            /* extra 1   TMP2m < 0C */
            if (extra == 1) {
                k = 0;
#ifdef IS_OPENMP_4_0
#pragma omp simd reduction(+:k)
#endif
                for (j = 0; j < save->n_ens; j++) {
                    if (ens[j] < 273.15) k++;
                }
                dataextra[i] = k / (double) save->n_ens;
            }
            /* extra 2   precip > 0, precip > trace */
            else if (extra == 2) {
                k = k2 = 0;
#ifdef IS_OPENMP_4_0
#pragma omp simd reduction(+:k,k2)
#endif
                for (j = 0; j < save->n_ens; j++) {
                    if (ens[j] > 0.0) k++;
                    if (ens[j] > TRACE) k2++;
                }
                dataextra[i] = k / (double) save->n_ens;
                dataextra2[i] = k2 / (double) save->n_ens;
            }
            /* extra 3   wind speed at 10m  95%  */
            else if (extra == 3) {
                dataextra[i] = i95 != save->n_ens - 1 ? ens[i95]*(1.0-d95) + ens[i95+1]*d95 : ens[i95];
            }
        }
        else {
            data10[i] = data25[i] = data50[i] = data75[i] = data90[i] = UNDEFINED;
            datamean[i] = datavar[i] = datamin[i] = datamax[i] = UNDEFINED;
            if (extra) {
                dataextra[i] = UNDEFINED;
                if (extra == 2) dataextra2[i] = UNDEFINED;
            }
        }
    }

    /* min */
    *table_4_7 = MIN;
    grib_wrt(new_sec, datamin, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    /* max */
    *table_4_7 = MAX;
    grib_wrt(new_sec, datamax, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    /* ave */
    *table_4_7 = AVE;
    grib_wrt(new_sec, datamean, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    /* spread */
    *table_4_7 = SPREAD;
    grib_wrt(new_sec, datavar, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *value_percentile = 10;
    grib_wrt(new_sec_percentile, data10, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *value_percentile = 25;
    grib_wrt(new_sec_percentile, data25, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *value_percentile = 50;
    grib_wrt(new_sec_percentile, data50, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *value_percentile = 75;
    grib_wrt(new_sec_percentile, data75, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    *value_percentile = 90;
    grib_wrt(new_sec_percentile, data90, ndata, save->nx, save->ny, 
            save->use_scale, save->dec_scale, save->bin_scale, 
            save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    /* extra */
    if (extra == 1) {
        table_4_9_probability[-2] = 1;
        table_4_9_probability[-1] = 1;
        table_4_9_probability[0] = 0;
        best_scaled_value(273.15, &scale_factor, &scale_value);
        table_4_9_probability[1] = scale_factor;
        int_char(scale_value, table_4_9_probability + 2);

        grib_wrt(new_sec_probability, dataextra, ndata, save->nx, save->ny, 
                save->use_scale, save->dec_scale, save->bin_scale, 
                save->wanted_bits, save->max_bits, save->grib_type, &(save->out));
    }
    else if (extra == 2) {
        table_4_9_probability[-2] = 1;
        table_4_9_probability[-1] = 2;
        table_4_9_probability[0] = 3;
        best_scaled_value(0.0, &scale_factor, &scale_value);
        table_4_9_probability[1] = scale_factor;
        int_char(scale_value, table_4_9_probability + 2);

        grib_wrt(new_sec_probability, dataextra, ndata, save->nx, save->ny, 
                save->use_scale, save->dec_scale, save->bin_scale, 
                save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

        table_4_9_probability[-2] = 2;
        table_4_9_probability[-1] = 2;
        table_4_9_probability[0] = 3;
        best_scaled_value(TRACE, &scale_factor, &scale_value);
        table_4_9_probability[1] = scale_factor;
        int_char(scale_value, table_4_9_probability + 2);
        grib_wrt(new_sec_probability, dataextra, ndata, save->nx, save->ny, 
                save->use_scale, save->dec_scale, save->bin_scale, 
                save->wanted_bits, save->max_bits, save->grib_type, &(save->out));
    }
    else if (extra == 3) {
        *value_percentile = 95;
        grib_wrt(new_sec_percentile, dataextra, ndata, save->nx, save->ny, 
	    save->use_scale, save->dec_scale, save->bin_scale, 
	    save->wanted_bits, save->max_bits, save->grib_type, &(save->out));
    }
    if (flush_mode) fflush_file(&(save->out));

    free(data);
    free(data10);
    free(data25);
    free(data50);
    free(data75);
    free(data90);
    free(datamean);
    free(datavar);
    free(datamin);
    free(datamax);
    if (extra) free(dataextra);
    if (extra == 2) free(dataextra2);
    return 0;
}

/*
 * HEADER:000:ens_processing:output:2:ave/min/max/spread X=output Y=0/1 default/CORe
 */

/**
 * Takes the ensemble member data and creates a pre-defined set of ensemble statistics.
 * 
 * This option is used to create various ensemble statistics such as minimum, maximum, 
 * average, spread, and various percentiles in order to describe the PDF. The values 
 * are calculated with respect to all the ensemble members. The calculations are 
 * different from the calculations used by the operational ensemble forecasts done at 
 * NCEP. So fields like the various percentiles and spread will have different values 
 * from the NCEP operational products. The results from the -ens_processing option should 
 * not be considered to be replacements for the official products because of the different 
 * algorithms used. (The exception will be the future climate reanalysis produced by CPC/NCEP.) 
 * 
 * The percentiles values were chosen because the future CPC/NCEP climate reanalysis (CORe) 
 * will be using 80 ensemble members.
 *
 * ## Calculated Variables
 *
 * 1) ensemble mean,  em = sum(x(i))/n,  i=1..n
 * 2) ensemble spread, RMSE = sqrt(sum((x(i)-em)**2)/n)  note: n is used rather than n-1
 * 3) minimum = minimum value over all ensemble members (for each grid point)
 * 4) maximum = maximum value over all ensemble members (for each grid point)
 * 5) 10 percentile
 * 6) 25 percentile
 * 7) 50 percentile (median)
 * 8) 75 percentile
 * 9) 90 percentile
 * 
 * These calculations are done when all the ensemble members have values.  This will affect 
 * parameters like the cloud-top temperature when some of the ensemble members are cloud free 
 * and have no cloud- top temperatures. The -enq_qc calculates the ensemble mean and spread 
 * ignoring the undefined values.
 * 
 * There are a few common ways to compute the percentile, [wgrib2 uses a method recommended 
 * by NIST](https://www.itl.nist.gov/div898/handbook/prc/section2/prc262.htm). For the probabilities, 
 * a count is used to determine them. 
 * 
 * As previously mentioned, the above calculations may differ from those used in deriving the 
 * operational products.
 * 
 * This option was developed for the future CORe reanalysis. For this reanalysis, additional 
 * fields are enabled by the second parameter set to "1". However, users are warned that these 
 * fields are expected to change. Generation of additional fields can be enabled by setting 
 * the second parameter to special values. 
 * 
 * 10) probability of precipitation > 0 *
 * 11) probability of more than a trace of precipitation (trace=TBD) *
 * 12) probability of 2m temperature greater than 273.15K *
 * 13) 95 percentile for WIND at 10m above ground *
 * 
 * (*) Optional, the definition of trace of precipitation is ad hoc. For wgrib2, trace is the 
 * accumulated precip < 0.xxx mm, or a rate < 0.xxx mm/day.
 * 
 * ## Memory Usage
 * The -ens_processing is unlike most wgrib2 options in that this option can use large amounts 
 * of memory. Suppose that you have an 80 member ensemble and are processing the tmp500 field. 
 * In order to calculate the percentiles of the tmp500, you need keep all 80 tmp500 fields in 
 * memory. As the size of the grid and the number of ensemble member increases, the required 
 * memory will increase. 
 * 
 * ## Code Table 4.3, Type of Generating Process
 * Grib contains metadata, and one piece is the "generating process". Wgrib2 tries its best to describe 
 * how the fields were created. This is documented in detail because if varies between PDT and center 
 * that created the grib file.
 * 
 * - Ensemble mean, ensemble spread, ensemble minimum, ensemble maximum: 
 *      Code Table 4.3 is preserved from the input file.
 * - Percentiles, Probabilities:
 *      NCEP: code table 4.3 = 199,  Ensemble forecast based on counting
 *      Other centers: code table 4.3 = 4, Ensemble forecast
 * 
 * The NCEP files are more descriptive because an entry in a local table was specially created for 
 * wgrib2.
 * 
 * ## Definition of an Ensemble Member
 * Wgrib2 v3.0.0 defined an ensemble as having the same Product Definition Template (PDT=0, 1, 8, 11) 
 * except for perturbation number. The initial time and forecast times could be different as long as
 * the verification time was the same. This allows lagged average forecast (LAF) ensembles. With wgrib2
 * v3.0.0, the type of ensemble forecast can be different. 
 * [(Code Table 4.6)](https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-6.shtml) This 
 * means that -ens_processing can now work with a combination of perturbed and control runs.
 * 
 * Limiting the PDTs to 0, 1, 8 and 11 is unnecessarily restrictive. As a result, -ens_processing does not 
 * work on aerosols, chemical tracers and simulated satellite data from ensemble members. 
 * 
 * ## Order of Fields
 * The -ens_processing requires fields to be in a specific order. The data has to be processed sequentially. 
 * If the -ens_processing finds a field that is unlike the previous fields (except for ensemble member id), 
 * it writes out the summary fields and starts processing a new variable. The number id of the ensemble members 
 * is ignored, and is not even necessary. 
 * 
 * ## Order of Fields: gmerge to the rescue
 * The -ens_processing requires the fields to be in a specific order (or processed in a specific order). Suppose 
 * we have forecasts from 3 ensemble members in files fcst1, fcst2 and fcst3. Now we are going to require that 
 * the fields have the same order in these files, and the files have no submessages. Then we can combine these 
 * files using gmerge, the resulting file will be in the correct order. (Gmerge has been included with wgrib2 
 * source code for many years.) There is a minor restriction, gmerge doesn't handle grib files which includes 
 * non-grib data.
 * 
 * @code{.sh}
 * gmerge output fcst1 fcst2 fcst3
 * @endcode
 * 
 * The requirements for output to be in the right order.

 * (1) fcstX must have fields in the same order
 * (2) number of forecasts &le 200 for wgrib2 v2.0.8 distribution
 * (3) fcstX must not use submessages
 * (4) the like forecasts must have same PDT except for ensemble number
 * (5) the ensemble number is optional
 * (6) no non-grib data in grib file
 *
 * You can find gmerge in the wgrib2 public distrbution under the aux_progs directory. If your initial files are 
 * not in identical order, you could combine the forecasts and sort the fields so that like fields are adjacent. 
 * However, sorting the individual forecasts and then running gmerge would probably be faster.
 * 
 * Now that "output" has the data in the correct order, the option -ens_processing can be used to create the 
 * min/max/ave/spread of the ensemble. 
 * 
 * ## Usage
 * -ens_processing FILE Option
 *
 *      FILE = output file, grib2 format
 *      Option = 0   default
 *              1   include probabilities (TMP2m, precip)
 *                  note: option 1 is intended for use by the future Conventional Observation REanalysis
 *                  (CORe).  The output will be determined the needs of this reanalysis.
 *              2   for future use or different output.
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example 
 * 
 * @code{.sh}
 * $ wgrib2 input -ens_processing output 0 
 * 1:0:d=2018020600:PRES:mean sea level:84 hour fcst:ENS=+1
 * 2:83628:d=2018020600:PRES:mean sea level:84 hour fcst:ENS=+2
 * 3:164290:d=2018020600:PRES:mean sea level:84 hour fcst:ENS=+3
 * 4:248531:d=2018020600:PRES:mean sea level:84 hour fcst:ENS=+4
 * 5:331723:d=2018020600:PRES:mean sea level:84 hour fcst:ENS=+5
 * 6:412226:d=2018020600:PRES:mean sea level:84 hour fcst:ENS=+6
 * ..
 * $ wgrib2 output
 * 1:0:d=2018020600:PRES:mean sea level:84 hour fcst:min all members
 * 2:130501:d=2018020600:PRES:mean sea level:84 hour fcst:max all members
 * 3:261002:d=2018020600:PRES:mean sea level:84 hour fcst:ens mean
 * 4:391503:d=2018020600:PRES:mean sea level:84 hour fcst:ens spread
 * 5:497569:d=2018020600:PRES:mean sea level:84 hour fcst:25%-75% range
 * 6:611780:d=2018020600:PRES:mean sea level:84 hour fcst:10% all members
 * 7:742281:d=2018020600:PRES:mean sea level:84 hour fcst:90% all members
 * 8:872782:d=2018020600:VIS:surface:84 hour fcst:min all members
 * 9:938123:d=2018020600:VIS:surface:84 hour fcst:max all members
 * ... 
 * @endcode
 *
 * @author Wesley Ebisuzaki @date 01/2018
 */
int f_ens_processing(ARG2) {
    struct ens_proc_struct *save;
    struct full_date verf_date;
    int pdt, new_type;

    if (mode == -1) {
        save_translation = decode = 1;

        // allocate static structure

        *local = save = (struct ens_proc_struct *) malloc( sizeof(struct ens_proc_struct));

        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("ens_processing: Could not open %s", arg1);
        }
        save->has_val = 0;
        save->n_ens = 0;
        save->grids = NULL;
        save->ngrids = 0;
        init_sec(save->first_sec);
        save->option = atoi(arg2);

        return 0;
    }
    save = (struct ens_proc_struct *) *local;
    if (mode == -2) {
        wrt_ens_proc(save->first_sec, save);
        fclose_file(&(save->out));
        free_ens_proc_struct(save);
        return 0;
    }

    /* processing a record */
    if (mode < 0) return 0;

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
    if (mode == 98) fprintf(stderr,"ens_processing: pdt=%d, new_type=%d\n",pdt, new_type);
    if (new_type == 1) {
        if (mode == 98) fprintf(stderr,"ens_processing: >> wrt a\n");
        wrt_ens_proc(save->first_sec, save);
        init_ens_proc_struct(save, sec, data, ndata);
    }
    else {
        if (mode == 98) fprintf(stderr,"ens_processing: >> update\n");
        update_ens_proc_struct(save, sec, data, ndata);
    }

    return 0;
}

/**
 * Get the index for the given percentile.
 * 
 * To find a percentile, you sort the values, get an index (floating)
 * and find the value with interpolation. The Wiki for percentile lists
 * 3 ways to get the index. This routine finds the index.
 * 
 * @param percent The desired percentile (0-100).
 * @param n The number of data points.
 * 
 * @return The index for the percentile, or -1 if not valid.
 * 
 * @author Wesley Ebisuzaki @date 01/2018
 */
static double percentile_index(float percent, int n) {
    double p;

    if (n < 1) return -1;
    p = percent * 0.01;
    if (p < 0.0 || p > 1.0) return -1;

    p = p*(n+1);
    if (p > n) return (double) n-1;
    if (p < 1) return 0.0;
    return (p-1.0);
}

/**
 * Function to sort floats as used by qsort.
 * 
 * @param a Pointer to the first float.
 * @param b Pointer to the second float.
 * 
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 * 
 * @author Wesley Ebisuzaki @date 01/2018
 */
static int testfloat(const void *a, const void *b) {
    float  aa, bb;
    aa = *(float *)a;
    bb = *(float *)b;
    if (aa < bb) return -1;
    if (aa == bb) return 0;
    return 1;
}
