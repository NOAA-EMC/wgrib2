#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * ave_var
 *
 *  v 0.1
 *
 * 12/2016: Public Domain: Wesley Ebisuzaki
 *         based on Ave_test.c public domain (Wesley Ebisuzaki)
 *
 * ave_var, computes the mean and variance using the Welford method (one-pass)
 */

/*  from http://jonisalonen.com/2013/deriving-welfords-method-for-computing-variance/

variance(samples):
  M := 0
  S := 0
  for k from 1 to N:
    x := samples[k]
    oldM := M
    M := M + (x-M)/k
    S := S + (x-M)*(x-oldM)
  return S/(N-1)

 */

// #define DEBUG

extern int decode, file_append, nx, ny, save_translation;
extern int flush_mode;
extern unsigned int *translation;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

struct ave_var_struct {
	double *M, *S;
	float *min, *max;
	unsigned int ndata;

	struct full_date time0, time1, time2; // time2 = verf time

        int has_val, n_fields, n_missing;
	int dt, dt_unit, nx, ny;
        unsigned char *first_sec[9];
        unsigned char *next_sec[9];
	int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
	enum output_grib_type grib_type;
	struct seq_file out;
};

static int write_ave_var(struct ave_var_struct *save);
static int free_ave_var_struct(struct ave_var_struct *save);
static int init_ave_var_struct(struct ave_var_struct *save, unsigned int ndata);
static int update_ave_var_struct(struct ave_var_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing);


static int free_ave_var_struct(struct ave_var_struct *save) {
    if (save->has_val == 1) {
	free(save->M);
	free(save->S);
	free(save->min);
	free(save->max);
        free_sec(save->first_sec);
        free_sec(save->next_sec);
    }
    free(save);
    return 0;
}

static int init_ave_var_struct(struct ave_var_struct *save, unsigned int ndata) {
    unsigned int i;

    if (ndata == 0) fatal_error("ave_var_var_struct: ndata = 0","");
    if (save->ndata != ndata) {
	if (save->ndata != 0) {
	    free(save->M);
	    free(save->S);
	    free(save->min);
	    free(save->max);
	}
        if ((save->M = (double *) malloc( ((size_t) ndata) * sizeof(double))) == NULL)
          fatal_error("ave_ave_var: memory allocation problem: val","");
        if ((save->S = (double *) malloc( ((size_t) ndata) * sizeof(double))) == NULL)
          fatal_error("ave_ave_var: memory allocation problem: val","");
        if ((save->min = (float *) malloc( ((size_t) ndata) * sizeof(float))) == NULL)
          fatal_error("ave_ave_var: memory allocation problem: val","");
        if ((save->max = (float *) malloc( ((size_t) ndata) * sizeof(float))) == NULL)
          fatal_error("ave_ave_var: memory allocation problem: val","");
	save->ndata = ndata;
    }

    for (i=0; i < ndata; i++) {
	save->M[i] = save->S[i] = 0.0;
	save->min[i] = save->max[i] = UNDEFINED;
    }
    save->has_val = 1;
    save->n_fields = 0;
    save->n_missing = 0;
    free_sec(save->first_sec);
    free_sec(save->next_sec);
    return 0;
}

static int update_ave_var_struct(struct ave_var_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing) {

    unsigned int i, ii;
    double oldM, x;

    if (save->ndata != ndata) fatal_error("ave_var: dimension mismatch","");

    /* the data needs to be translated from we:sn to raw, need to 
       do it now, translation[] may be different if called from finalized phase */

    save->n_fields += 1;
#pragma omp parallel for private(i,ii,x,oldM)
    for (i = 0; i < ndata; i++) {
	ii = translation == NULL ? i : translation[i];
        if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->M[ii])) {
	    x = data[i];
	    oldM = save->M[ii];
	    save->M[ii] += (x-save->M[ii]) / (double) save->n_fields;
	    save->S[ii] += (x-save->M[ii]) * (x-oldM);
	}
	else {
	    save->M[ii] = UNDEFINED;
	    save->S[ii] = UNDEFINED;
	}
    }
    if (save->n_fields == 1) {
	for (i = 0; i < ndata; i++) {
	    ii = translation == NULL ? i : translation[i];
	    save->max[ii] = save->min[ii] = data[i];
	}
    }
    else {
#pragma omp parallel for private(i,ii)
	for (i = 0; i < ndata; i++) {
	    ii = translation == NULL ? i : translation[i];
            if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->M[ii])) {
	        save->max[ii] = data[i] > save->max[ii] ? data[i] : save->max[ii];
	        save->min[ii] = data[i] < save->min[ii] ? data[i] : save->min[ii];
	    }
	}
    }
    
    if (save->n_fields == 1) {
	save->nx = nx;
	save->ny = ny;
	save->use_scale = use_scale;
	save->dec_scale = dec_scale;
	save->bin_scale = bin_scale;
	save->wanted_bits = wanted_bits;
	save->max_bits = max_bits;
	save->grib_type = grib_type;
    }
    save->n_missing += missing;

    // update current reference time
    Get_time(sec[1]+12,&(save->time1));

    // update current verf time
    if (Verf_time(sec, &(save->time2)) != 0) {
	fatal_error("update_ave_var_struct: could not find verification time","");
    }
    return 0;
}

static int write_ave_var(struct ave_var_struct *save) {
    int j, n, pdt, i_ave;
    unsigned int i, ndata;
    float *data;
    unsigned char *p, *sec4;

    sec4 = NULL;
    if (save->has_val == 0) return 0; 

    ndata = save->ndata;
    if ((data = (float *) malloc(sizeof(float) * ((size_t) ndata))) == NULL) fatal_error("ave_var: memory allocation","");

    /* mean value */
#pragma omp parallel for private(i)
    for (i = 0; i < ndata; i++) {
	data[i] = (UNDEFINED_VAL(save->M[i])) ? UNDEFINED : save->M[i];
    }

    pdt = GB2_ProdDefTemplateNo(save->first_sec);

    // average of an analysis or forecast

    i_ave= 0;
    if (pdt == 0) {
        sec4 = (unsigned char *) malloc(58 * sizeof(unsigned char));
	if (sec4 == NULL) fatal_error("ave: memory allocation","");
	for (i = 0; i < 34; i++) {
	    sec4[i] = save->first_sec[4][i];
	}
	uint_char((unsigned int) 58, sec4);		// length
	sec4[8] = 8;			// pdt
	// verification time
	Save_time(&(save->time2), sec4+34);
	sec4[41] = 1;
	uint_char(save->n_missing, sec4+42);
	sec4[i_ave = 46] = 0;			// average
	sec4[47] = 1;			// rt++
	sec4[48] = save->dt_unit;					// total length of stat processing
	uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+49);
	sec4[53] = save->dt_unit;					// time step
	uint_char(save->dt, sec4+54);
    }

    // average of an ensemble forecast, use pdt 4.11

    else if (pdt == 1) {
        sec4 = (unsigned char *) malloc(61 * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("ave_var: memory allocation","");
        for (i = 0; i < 37; i++) {
            sec4[i] = save->first_sec[4][i];
        }
        uint_char((unsigned int) 61, sec4);             // length
        sec4[8] = 11;                    		// pdt
        // verification time
	Save_time(&(save->time2), sec4+37);
        sec4[44] = 1;                                   // 1 time range
        uint_char(save->n_missing, sec4+45);
        sec4[i_ave = 49] = 0;                                   // average
        sec4[50] = 1;                                   // rt=constant
        sec4[51] = save->dt_unit;                                       // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+52);
        sec4[56] = save->dt_unit;                                       // time step
        uint_char(save->dt, sec4+57);
    }

    // average of derived fcst base on all ens members, use pdt 4.12

    else if (pdt == 2) {
        sec4 = (unsigned char *) malloc(60 * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("fcst_ave: memory allocation","");
        for (i = 0; i < 36; i++) {
            sec4[i] = save->first_sec[4][i];
        }
        uint_char((unsigned int) 60, sec4);             // length
        sec4[8] = 12;                    		// pdt
        // verification time
	Save_time(&(save->time2), sec4+36);
        sec4[43] = 1;                                   // 1 time range
        uint_char(save->n_missing, sec4+44);
        sec4[i_ave = 48] = 0;                                   // average
        sec4[49] = 1;                                   // rt=constant
        sec4[50] = save->dt_unit;                       // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+51);
        sec4[55] = save->dt_unit;                       // time step
        uint_char(save->dt, sec4+56);
    }

    // average of an average or accumulation

    else if (pdt == 8) {
	i = GB2_Sec4_size(save->first_sec);
	n = save->first_sec[4][41];
	if (i != 46 + 12*n) fatal_error("ave: invalid sec4 size for pdt=8","");

        // keep pdt == 8 but make it 12 bytes bigger
        sec4 = (unsigned char *) malloc( (i+12) * sizeof(unsigned char));
	if (sec4 == NULL) fatal_error("ave: memory allocation","");

	uint_char((unsigned int) i+12, sec4);		// new length

	for (i = 4; i < 34; i++) {			// keep base of pdt
	    sec4[i] = save->first_sec[4][i];
	}
	
	// new verification time
	Save_time(&(save->time2), sec4+34);

	// number of stat-proc loops is increased by 1
	sec4[41] = n + 1;

	// copy old stat-proc loops 
	// for (j = n*12-1;  j >= 0; j--) sec4[58+j] = save->first_sec[4][46+j];
	for (j = 0; j < n*12; j++) sec4[46+12+j] = save->first_sec[4][46+j];

	uint_char(save->n_missing, sec4+42);
	sec4[i_ave = 46] = 0;			// average
	sec4[47] = 1;			// rt++
	sec4[48] = save->dt_unit;						// total length of stat processing
	uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+49);	// processing number
	sec4[53] = save->dt_unit;						// time step
	uint_char(save->dt, sec4+54);
    }

    // pdt 4.12 -> pdt 4.12 ave -> ave of ave

    else if (pdt == 12) {
        i = GB2_Sec4_size(save->first_sec);
        n = save->first_sec[4][43];			// number of time ranges
        if (i != 48 + 12*n) fatal_error("ave: invalid sec4 size for pdt=12","");

        // keep pdt == 12 but make it 12 bytes bigger
        sec4 = (unsigned char *) malloc( (i+12) * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("ave: memory allocation","");

        uint_char((unsigned int) i+12, sec4);           // new length

        for (i = 4; i < 36; i++) {                      // keep base of pdt
            sec4[i] = save->first_sec[4][i];
        }

        // new verification time
	Save_time(&(save->time2), sec4+36);

        // number of stat-proc loops is increased by 1
        sec4[43] = n + 1;

        // copy old stat-proc loops
        for (j = 0; j < n*12; j++) sec4[48+12+j] = save->first_sec[4][48+j];

        uint_char(save->n_missing, sec4+44);
        sec4[i_ave = 48] = 0;                   // average
        sec4[49] = 1;                   // rt++
        sec4[50] = save->dt_unit;                                               // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+51);        // processing number
        sec4[55] = save->dt_unit;                                               // time step
        uint_char(save->dt, sec4+56);
    }


    else {
	fatal_error_i("ave_var with pdt %d is not supported",pdt);
    }


    // write grib file
    p = save->first_sec[4];
    save->first_sec[4] = sec4;

    grib_wrt(save->first_sec, data, ndata, save->nx, save->ny, 
	save->use_scale, save->dec_scale, save->bin_scale, 
	save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    if (flush_mode) fflush_file(&(save->out));

    /* Sample Standard Deviation */
    if (save->n_fields > 1) {
#pragma omp parallel for private(i)
        for (i = 0; i < ndata; i++) {
	    data[i] = (UNDEFINED_VAL(save->M[i])) ? UNDEFINED : sqrt(save->S[i]/(save->n_fields - 1));
        }
    }
    else {
        for (i = 0; i < ndata; i++) {
	    data[i] = UNDEFINED;
        }
    }
    if (i_ave == 0) fatal_error("ave_var: i_ave not defined","");
    sec4[i_ave] = 6;			// (sample) standard deviation

    /* note standard devation can be writen out in same scaling as mean */
    grib_wrt(save->first_sec, data, ndata, save->nx, save->ny, 
	save->use_scale, save->dec_scale, save->bin_scale, 
	save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

    if (flush_mode) fflush_file(&(save->out));

    sec4[i_ave] = 3;			// min
    /* note standard devation can be writen out in same scaling as mean */
    grib_wrt(save->first_sec, save->min, ndata, save->nx, save->ny, 
	save->use_scale, save->dec_scale, save->bin_scale, 
	save->wanted_bits, save->max_bits, save->grib_type, &(save->out));
    if (flush_mode) fflush_file(&(save->out));

    sec4[i_ave] = 2;			// max
    /* note standard devation can be writen out in same scaling as mean */
    grib_wrt(save->first_sec, save->max, ndata, save->nx, save->ny, 
	save->use_scale, save->dec_scale, save->bin_scale, 
	save->wanted_bits, save->max_bits, save->grib_type, &(save->out));
    if (flush_mode) fflush_file(&(save->out));

    save->first_sec[4] = p;
    free(data);
    free(sec4);
    return 0;
}

/*
 * HEADER:000:ave_var:output:2:average/std dev/min/max X=time step, Y=output
 */
int f_ave_var(ARG2) {

    struct ave_var_struct *save;

    int i, pdt, new_type;
    struct full_date time, ttime;
    int missing;
    char string[10];

    // initialization

    if (mode == -1) {
        save_translation = decode = 1;

	// allocate static structure

        *local = save = (struct ave_var_struct *) malloc( sizeof(struct ave_var_struct));
        if (save == NULL) fatal_error("memory allocation f_ave_var","");

	i = sscanf(arg1, "%d%2s", &save->dt, string);
	if (i != 2) fatal_error("ave_var: delta-time: (int)(2 characters) %s", arg1);
	save->dt_unit = -1;
	if (strcmp(string,"hr") == 0) save->dt_unit = 1;
	else if (strcmp(string,"dy") == 0) save->dt_unit = 2;
	else if (strcmp(string,"mo") == 0) save->dt_unit = 3;
	else if (strcmp(string,"yr") == 0) save->dt_unit = 4;
	if (save->dt_unit == -1) fatal_error("ave_var: unsupported time unit %s", string);

        if (fopen_file(&(save->out), arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("ave_var: could not open %s", arg2);
        }
	save->has_val = 0;
	save->ndata = 0;
	save->n_fields = 0;
	save->M = save->S = NULL;

        init_sec(save->first_sec);
        init_sec(save->next_sec);

	return 0;
    }

    save = (struct ave_var_struct *) *local;

    if (mode == -2) {			// cleanup
	if (save->has_val == 1) {
	    write_ave_var(save);
	}
	fclose_file(&(save->out));
	free_ave_var_struct(save);
	return 0;
    }

    if (mode < 0) fatal_error("ave_var: programming error","");

    pdt = GB2_ProdDefTemplateNo(sec);
    // only support pdt == 0, 1, 2, 8, 12
    if (pdt != 0 && pdt != 1 && pdt != 2 && pdt != 8 && pdt != 12) return 0;

    // first time through .. save data and return

    if (save->has_val == 0) {		// new data: write and save
	init_ave_var_struct(save, ndata);
	update_ave_var_struct(save, sec, data, ndata, 0);

	// copy sec
        copy_sec(sec, save->first_sec);
        copy_sec(sec, save->next_sec);

	// time0 = lowest reference time
	// time1 = current reference time
	// time2 = verification time

	Get_time(sec[1]+12,&(save->time0));
	save->time1 = save->time0;
	if (Verf_time(sec, &(save->time2)) != 0) {
	    fatal_error("ave_var: could not determine the verification time","");
	}

	save->has_val = 1;
	return 0;
    }

    // check to see if new variable

    new_type = 0;

    // get the reference time of field
    Get_time(sec[1]+12, &time);

    // get the reference time of last field
    ttime = save->time1;

    Add_time(&ttime, save->dt, save->dt_unit);
    missing = 0;
    while ((i=Cmp_time(&time, &ttime)) > 0) {
	missing++;
        Add_time(&ttime, save->dt, save->dt_unit);
    }

    if (i != 0) new_type = 1;
    if (mode == 98) fprintf(stderr, "ave: compare ref time new_type = %d missing=%d\n", new_type, missing);

    if (new_type == 0) {
	if (same_sec0(sec,save->first_sec) == 0 ||
            same_sec1_not_time(mode, sec,save->first_sec) == 0 ||
            same_sec3(sec,save->first_sec) == 0 ||
            same_sec4_not_time(mode, sec,save->first_sec) == 0) 
	    new_type = 1;
        if (mode == 98) fprintf(stderr, "ave: testsec %d %d %d %d\n", same_sec0(sec,save->first_sec),
            same_sec1_not_time(0, sec,save->first_sec),
            same_sec3(sec,save->first_sec),
            same_sec4_not_time(0, sec,save->first_sec));
        if (mode == 98) fprintf(stderr, "ave_var: new_type %d\n", new_type);
    }

    // finished determining whether new_type

    if (new_type == 0) {		// update sum
        if (mode == 98) fprintf(stderr, "ave_var: update sum\n");
	update_ave_var_struct(save, sec, data, ndata, missing);
	return 0;
    }

    // new field, do grib output and save current data

    write_ave_var(save);
    init_ave_var_struct(save, ndata);
    update_ave_var_struct(save, sec, data, ndata, 0);
    copy_sec(sec, save->first_sec);
    copy_sec(sec, save->next_sec);
    return 0;
}
