#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * ens_processing
 *
 *  v 1.0 experimental based on time_processing.c
 *
 *
 * 1/2018: Public Domain: Wesley Ebisuzaki
 */

/* 
 * code table 4.3 used by percentile and prob fcsts, 
 *   WMO: ensemble forecast
 *  NCEP: ensemble forecast based on counting
 */
#define PROCESS 4
#define NCEP_PROCESS 199

/* code table 4.7 */
#define AVE	0
#define SPREAD	4
#define MAX	9
#define MIN	8

/* trace defined to be 0.1 mm  .. trace precip 0.1mm/3hours converted to mm/s */
#define TRACE	(0.1/86400.0/8)

static int testfloat(const void *a, const void *b);
static double percentile_index(float percent, int n);

extern int decode, file_append, nx, ny, save_translation;

extern int flush_mode;
extern unsigned int *translation;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

struct ens_proc_struct {
    unsigned int npnts, nx, ny;
    int has_val, n_ens;
    unsigned char *first_sec[9];
    struct full_date verf_date;
    int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
    enum output_grib_type grib_type;
    struct seq_file out;
    float *grids;		/* hold grids for median-type calculations */
    int ngrids;
    int option;
};

static int wrt_ens_proc(unsigned char **sec, struct ens_proc_struct *save);
static int free_ens_proc_struct(struct ens_proc_struct *save);
static int init_ens_proc_struct(struct ens_proc_struct *save, unsigned char **sec, float *data, unsigned int ndata);
static int update_ens_proc_struct(struct ens_proc_struct *save, unsigned char **sec, float *data, unsigned int ndata);

/* routines to initialized and free ens_proc_struct */

static int free_ens_proc_struct(struct ens_proc_struct *save) {
    if (save->has_val == 1) {
        free_sec(save->first_sec);
	if (save->ngrids) {
	    free(save->grids);
	    save->ngrids=0;
	}
    }
    free(save);
    return 0;
}

static int init_ens_proc_struct(struct ens_proc_struct *save, 
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

/* update_ens_proc_struct: save grid in memory */

static int update_ens_proc_struct(struct ens_proc_struct *save, 
    unsigned char **sec, float *data, unsigned int ndata) {

    unsigned int i;

    if (save->npnts != ndata) fatal_error("ens_processing: size mismatch","");

    if (save->n_ens == save->ngrids) {		/* need to make save->grids bigger */
	save->ngrids *= ENS_PROCESSING_NGRID_FACTOR;
	save->grids = realloc(save->grids, ((size_t) save->ngrids) * save->npnts * sizeof (float));
	if (save->grids == NULL) {
	    /* if realloc fails, original memory is retained .. some memory is lost here, don't care */
	    save->ngrids = 0;
	    fatal_error("ens_processing: memory allocation in update","");
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

static int wrt_ens_proc(unsigned char **sec, struct ens_proc_struct *save) {
    int pdt, pdt_ens, pdt_probability, pdt_percentile;
    unsigned int i, ndata, k, k2;
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
            (strcmp(name, "TMP") == 0 || strcmp(name,"TMIN") == 0 || strcmp(name,"TMAX") == 0)) extra = 1;
        else if (strcmp(level,"surface") == 0 && (strcmp(name, "APCP") == 0)) {
		extra = 2;
	}
        else if (strcmp(level,"surface") == 0 && (strcmp(name, "PRATE") == 0)) {
		extra = 2;
	}
        else if (strcmp(level,"10 m above ground") == 0 && strcmp(name,"WIND") == 0) extra = 3;
    }

    isNCEP = GB2_Center(save->first_sec) == NCEP;

    /* create new_pdt (sec4) */

    if (new_pdt(save->first_sec, sec4, pdt_ens, -1, 1)) 
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
        if (new_pdt(save->first_sec, sec4_probability, pdt_probability, -1, 1)) 
            fatal_error("ens_processing: new_pdt probability failed","");
        for (i = 0; i < 9; i++) new_sec_probability[i] = save->first_sec[i];
        new_sec_probability[4] = sec4_probability;
	p = code_table_4_3_location(new_sec_probability);
	if (p == NULL) fatal_error("ens_processing: program error 4.3","");
        *p = isNCEP ? NCEP_PROCESS : PROCESS;
	table_4_9_probability = code_table_4_9_location(new_sec_probability);
	if (table_4_9_probability == NULL) fatal_error("ens_processing: program error 4.9","");
    }

    /* creaate new_pdt_percentile */
    if (new_pdt(save->first_sec, sec4_percentile, pdt_percentile, -1, 1)) 
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
    d75 = x25-i25;

    x90 = percentile_index(90.0, save->n_ens);
    i90 = floor(x90);
    d90 = x90-i90;

    x95 = percentile_index(95.0, save->n_ens);
    i95 = floor(x95);
    d95 = x95-i95;

#pragma omp parallel private(i,k,sum,sq)
    {
	float ens[save->n_ens];
        int j;
  
#pragma omp for schedule(dynamic)
        for (i = 0; i < ndata; i++) {
	    /* make vector of ensemble member grid points */
	    for (k = 0; k < save->n_ens; k++) {
    	        if (DEFINED_VAL(save->grids[i+k*ndata])) {
		   ens[k] = save->grids[i+k*ndata];
		}
		else { 
		    break;
		}
	    }
	    if (k == save->n_ens) {
	        /* sort */
	        qsort(&(ens[0]), save->n_ens, sizeof(float), &testfloat);

		/* find the various percentiles */	    

                data10[i] = ens[i10]*(1.0-d10) + ens[i10+1]*d10;
                data25[i] = ens[i25]*(1.0-d25) + ens[i25+1]*d25;
                data50[i] = ens[i50]*(1.0-d50) + ens[i50+1]*d50;
                data75[i] = ens[i75]*(1.0-d75) + ens[i75+1]*d75;
                data90[i] = ens[i90]*(1.0-d90) + ens[i90+1]*d90;
		datamin[i] = ens[0];
		datamax[i] = ens[save->n_ens - 1];

		sum = sq = 0.0;
		for (j = 0; j < save->n_ens; j++) {
		    sum += ens[j];
		}
		sum = sum / save->n_ens;
		datamean[i] = sum;

		for (j = 0; j < save->n_ens; j++) {
		    sq += (ens[j]-sum)*(ens[j]-sum);
		}
                datavar[i] = sqrt(sq/save->n_ens);

		/* extra 1   TMP2m < 0C */
		if (extra == 1) {
		    for (k = j = 0; j < save->n_ens; j++) {
		        if (ens[j] < 273.15) k++;
		    }
		    dataextra[i] = k / (double) save->n_ens;
		}
		/* extra 2   precip > 0, precip > trace */
		else if (extra == 2) {
		    for (k = k2 = j = 0; j < save->n_ens; j++) {
		        if (ens[j] > 0.0) k++;
		        if (ens[j] > TRACE) k2++;
		    }
		    dataextra[i] = k / (double) save->n_ens;
		    dataextra2[i] = k2 / (double) save->n_ens;
		}
		/* extra 3   wind speed at 10m  95%  */
		else if (extra == 3) {
                    dataextra[i] = ens[i95]*(1.0-d95) + ens[i95+1]*d95;
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
 * HEADER:000:ens_processing:output:2:ave/min/max/spread X=output Y=future use
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
if (mode == 98) fprintf(stderr,"ens_processing: >> cleanup wrt\n");
        wrt_ens_proc(save->first_sec, save);
if (mode == 98) fprintf(stderr,"ens_processing: >> cleanup init\n");
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


/*
 * To find a percentile, you sort the values, get an index (floating) 
 * and find the value with interpolation. the Wiki for percentile lists 
 * 3 ways to get the index.  This routine find the index.
 * returns: -1 (not valid)
 *           x 1..N (float)
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

/* function to sort floats as used by qsort */

static int testfloat(const void *a, const void *b) {
        float  aa, bb;
        aa = *(float *)a;
        bb = *(float *)b;
        if (aa < bb) return -1;
        if (aa == bb) return 0;
        return 1;
}
