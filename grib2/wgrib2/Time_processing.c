#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * time_processing
 *
 *  v 0.1 experimental
 *
 * 4/2009: Public Domain: Wesley Ebisuzaki
 * 4/2010: add means of means
 * 4/2013: added pdt 4.11 (ensemble)
 * 12/2014: set use_scale to zero, optimizations
 * 1/2015: removed set use_scale
 * 3/2016: added pdt 2 and 12
 * 9/2017: reborn as time processing
 */

/*  from http://jonisalonen.com/2013/deriving-welfords-method-for-computing-variance/
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
 */


// #define DEBUG

/*
 * HEADER:000:ave:output:2:average X=time step Y=output v2
 */
int f_ave(ARG2) {
    return f_time_processing(call_ARG4(inv_out,local,"0","1",arg1,arg2));
}

/*
 * HEADER:000:fcst_ave:output:2:average X=time step Y=output v2
 */
int f_fcst_ave(ARG2) {
    return f_time_processing(call_ARG4(inv_out,local,"0","2",arg1,arg2));
}

/* supported code table 4.10 */
#define AVE	0
#define MAX	2
#define MIN	3
#define DIFF1	4
#define RMS	5
#define STD_DEV	6
#define DIFF2	8

extern int decode, file_append, nx, ny, save_translation;

extern int flush_mode;
extern unsigned int *translation;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

struct ave_struct {
        double *sum, *M, *S, *first, *last;
        int *n;					/* n[], number of times for sum, etc */
	unsigned int npnts;
        int has_val, n_fields, n_missing;
	int dt, dt_unit, nx, ny;
        unsigned char *first_sec[9];
        unsigned char *next_sec[9];
	int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
	enum output_grib_type grib_type;
	int code_table_4_10;
	int code_table_4_11;
        struct full_date ref_time0, ref_time, verf_time;
	struct seq_file out;
};

static int do_ave(struct ave_struct *save);
static int free_ave_struct(struct ave_struct *save);
static int init_ave_struct(struct ave_struct *save, unsigned int ndata);
static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing);


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

static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing) {

    unsigned int i, ii;
    double x, oldM;

    if (save->npnts != ndata) fatal_error("time_processing: add_to_ave dimension mismatch","");

    /* the data needs to be translated from we:sn to raw, need to 
       do it now, translation[] may be different if called from finalized phase */

    if (save->code_table_4_10 == AVE) {
#pragma omp parallel for private(i,ii)
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
		ii = translation == NULL ? i : translation[i];
	        save->sum[ii] += data[i];
	        save->n[ii]++;
	    }
	}
    }
    else if (save->code_table_4_10 == MAX) {
#pragma omp parallel for private(i,ii)
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
#pragma omp parallel for private(i,ii)
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
#pragma omp parallel for private(i,ii)
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i])) {
		ii = translation == NULL ? i : translation[i];
	        save->sum[ii] += data[i]*data[i];
	        save->n[ii]++;
	    }
	}
    }
    else if (save->code_table_4_10 == STD_DEV) {
#pragma omp parallel for private(i,ii,x,oldM)
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
#pragma omp parallel for private(i,ii)
            for (i = 0; i < ndata; i++) {
		ii = translation == NULL ? i : translation[i];
		save->first[ii] = data[i];
	    }
	}
#pragma omp parallel for private(i,ii)
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

/* pdt has a value from 0..65535 */
/* two cases for ave_pdt:
 *  ave_pdt is different from pdt
 *  ave_pdt is the same as pdt .. extend time specification
 *
 * case 1: ave_pdt < PDT_TYPE2
 * case 2: ave_pdt = (ave_pdt + PDT_TYPE2)
 */

#define PDT_TYPE2		131072
#define PDT_MIN			0
#define PDT_MAX			65535

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
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
    	    data[i] = (save->n[i] == save->n_fields) ? factor * save->sum[i] : UNDEFINED;
        }
    }
    else if (save->code_table_4_10 == RMS) {
        factor = 1.0 / save->n_fields;
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
    	    data[i] = (save->n[i] == save->n_fields) ? sqrt(factor * save->sum[i]) : UNDEFINED;
        }
    }
    else if (save->code_table_4_10 == STD_DEV) {
	if (save->n_fields > 1) {
#pragma omp parallel for private(i)
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
#pragma omp parallel for private(i)
	for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(save->first[i]) && DEFINED_VAL(save->last[i])) {
		data[i] = save->last[i] - save->first[i];
	    }
	    else data[i] = UNDEFINED;
	}
    }
    else if (save->code_table_4_10 == DIFF2) {
#pragma omp parallel for private(i)
	for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(save->first[i]) && DEFINED_VAL(save->last[i])) {
		data[i] = save->first[i] - save->last[i];
	    }
	    else data[i] = UNDEFINED;
	}
    }
    else {
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
    	    data[i] = (save->n[i] != save->n_fields) ? UNDEFINED : save->sum[i];
        }
    }

    pdt = GB2_ProdDefTemplateNo(save->first_sec);
    for (i = 0; i < 9; i++) sec[i] = save->first_sec[i];
    sec[4] = sec4;
//fprintf(stderr,"doave 0: pdt=%d\n", pdt);

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
	i = new_pdt(save->first_sec, sec4, ave_pdt, -1, 1);

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
	i = new_pdt(save->first_sec, sec4, ave_pdt, ave_len, 1);

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
	else fatal_error("time_processing: code_table_4.11 must be 1/2 or analyses/forecast not $s", arg2);

	i = sscanf(arg3, "%d%2s", &save->dt,string);
	if (i != 2) fatal_error("time_processing: delta-time: (int)(2 characters) %s", arg3);
	save->dt_unit = -1;
	if (strcmp(string,"hr") == 0) save->dt_unit = 1;
	else if (strcmp(string,"dy") == 0) save->dt_unit = 2;
	else if (strcmp(string,"mo") == 0) save->dt_unit = 3;
	else if (strcmp(string,"yr") == 0) save->dt_unit = 4;
	else if (strcmp(string,"mn") == 0) save->dt_unit = 0;
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
                same_sec1_not_time(0,sec,save->first_sec),
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
