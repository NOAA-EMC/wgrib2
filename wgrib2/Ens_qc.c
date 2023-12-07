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
 */

/* code table 4.7 */
#define AVE	0
#define SPREAD	4
#define MAX	9
#define MIN	8
/* this is only for NCEP files */
#define EXTREME_FORECAST_INDEX 199

extern int decode, file_append, nx, ny, save_translation;

extern int flush_mode;
extern unsigned int *translation;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

struct ens_qc_struct {
    unsigned int npnts, nx, ny;
    int has_val, n_ens;
    unsigned char *first_sec[9];
    struct full_date verf_date;
    int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
    enum output_grib_type grib_type;
    struct seq_file out, extreme_grb, extreme_txt;
    float *grids;		/* hold grids for median-type calculations */
    int ngrids;
    double max_err;
};

static int wrt_ens_qc(unsigned char **sec, struct ens_qc_struct *save);
static int free_ens_qc_struct(struct ens_qc_struct *save);
static int init_ens_qc_struct(struct ens_qc_struct *save, unsigned char **sec, float *data, unsigned int ndata);
static int update_ens_qc_struct(struct ens_qc_struct *save, unsigned char **sec, float *data, unsigned int ndata);

/* routines to initialized and free ens_qc_struct */

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

static int init_ens_qc_struct(struct ens_qc_struct *save, 
    unsigned char **sec, float *data, unsigned int ndata) {
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
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
	    save->grids[i] = data[i];
	}
    }
    else {
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
	    save->grids[translation[i]] = data[i];
	}
    }
    save->n_ens = 1;
    return 0;
}

/* update_ens_qc_struct: save grid in memory */

static int update_ens_qc_struct(struct ens_qc_struct *save, 
    unsigned char **sec, float *data, unsigned int ndata) {

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
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
	    save->grids[i+save->n_ens*ndata] = data[i];
	}
    }
    else {
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
	    save->grids[translation[i]+save->n_ens*ndata] = data[i];
	}
    }
    save->n_ens++;
    return 0;
}

/* routine is called when you want to write the ensemble statistics */

static int wrt_ens_qc(unsigned char **sec, struct ens_qc_struct *save) {
    int pdt, pdt_ens, k, k_0;
    unsigned int i, ndata;
    float *datamin, *datamax, *datavar, *datamean, *dataextreme, maxextreme;
    float minval, maxval;
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

#pragma omp parallel for private(i, k, sum, sq, k_0, n, minval, maxval), reduction (max:maxextreme)
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
	   for (k = k_0 + 1; k < n_grids; k++) {
		tmp = save->grids[i+k*ndata];
		if (DEFINED_VAL(tmp)) {
		    maxval = maxval > tmp ? maxval : tmp;
		    minval = minval < tmp ? minval : tmp;
		    sum += tmp;
		    n++;
		}
	    }
	    datamean[i] = sum = sum / n;
	    datamin[i] = minval;
	    datamax[i] = maxval;

	    /* calculate variance */

	    sq = 0;
#ifdef IS_OPENMP_4_0
#pragma omp simd private(tmp) reduction(+:sq)
#endif
            for (k = k_0; k < n_grids; k++) {
	        tmp = save->grids[i+k*ndata];
                if (DEFINED_VAL(tmp)) {
		    sq += (tmp - sum)*(tmp - sum);
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
