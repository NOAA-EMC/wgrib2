#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Fcst_ave
 *
 *  v 0.1 experimental
 *
 * based on Ave_test.c 4/2010 (public domain Wesley Ebisuzaki)
 * v 0.2 4/2013 added PDT=4.1
 * v 0.3 12/2014 set use_scale = 0, optimize
 * v 0.4 1/2015 remove set use_scale = 0
 * 
 */

// #define DEBUG

extern int decode, file_append, nx, ny, save_translation;
extern unsigned int *translation;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

struct ave_struct {
        double *sum;
        int *n;
	unsigned int n_sum;
        int has_val, n_fields, n_missing;
	int dt, dt_unit, nx, ny;
	int full_dt;
        unsigned char *first_sec[9];
        unsigned char *next_sec[9];
	int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
	enum output_grib_type grib_type;
        int ref_year, ref_month, ref_day, ref_hour, ref_minute, ref_second;
        int fcst_year, fcst_month, fcst_day, fcst_hour, fcst_minute, fcst_second;
        int year2, month2, day2, hour2, minute2, second2;  // verification time
	struct seq_file out;
};

static int do_ave(struct ave_struct *save);
static int free_ave_struct(struct ave_struct *save);
static int init_ave_struct(struct ave_struct *save, unsigned int ndata);
static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing);

static int free_ave_struct(struct ave_struct *save) {
#ifdef DEBUG
printf(" free ");
#endif
    if (save->has_val == 1) {
	free(save->sum);
        free(save->n);
        free_sec(save->first_sec);
        free_sec(save->next_sec);
    }
    free(save);
    return 0;
}

static int init_ave_struct(struct ave_struct *save, unsigned int ndata) {
    unsigned int i;
#ifdef DEBUG
printf(" init ");
#endif
    if (save->has_val == 0 || save->n_sum != ndata) {
	if (save->has_val == 1) {
	    free(save->sum);
	    free(save->n);
	}
        if ((save->sum = (double *) malloc(((size_t) ndata) * sizeof(double))) == NULL)
          fatal_error("ave: memory allocation problem: val","");
        if ((save->n = (int *) malloc(((size_t) ndata) * sizeof(int))) == NULL)
          fatal_error("ave: memory allocation problem: val","");
    }

    for (i=0; i < ndata; i++) {
	save->n[i] = 0;
	save->sum[i] = 0.0;
    }
    save->n_sum = ndata;
    save->has_val = 1;
    save->n_fields = 0;
    save->n_missing = 0;
    free_sec(save->first_sec);
    free_sec(save->next_sec);
    return 0;
}

static int add_to_ave_struct(struct ave_struct *save, unsigned char **sec, float *data, unsigned int ndata,int missing) {

    unsigned int i;

    if (save->n_sum != ndata) fatal_error("add_to_ave: dimension mismatch","");

   /* the data needs to be translated from we:sn to raw, need to
       do it now, translation[] may be different if called from finalized phase */

    if (translation == NULL) {
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->sum[i])) {
                save->sum[i] += data[i];
                save->n[i]++;
            }
        }
    }
    else {
        for (i = 0; i < ndata; i++) {
            if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->sum[i])) {
                save->sum[translation[i]] += data[i];
                save->n[translation[i]]++;
            }
        }
    }

    save->n_fields += 1;
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

    // update current verf time
    if (verftime(sec, &save->year2, &save->month2, &save->day2, &save->hour2, &save->minute2, &save->second2) != 0) {
	fatal_error("add_to_ave_struct: could not find verification time","");
    }
    return 0;
}

static int do_ave(struct ave_struct *save) {
    int j, n, pdt;
    unsigned int i, ndata;
    float *data;
    unsigned char *p, *sec4;
    double factor;

    sec4 = NULL;
    if (save->has_val == 0) return 0; 
#ifdef DEBUG
printf(" ave nfields=%d missing=%d\n",save->n_fields,save->n_missing);
#endif

    ndata = save->n_sum;
    if ((data = (float *) malloc(((size_t) ndata) * sizeof(float))) == NULL) fatal_error("ave: memory allocation","");
    factor = 1.0 / save->n_fields;
    for (i = 0; i < ndata; i++) {
    	if (save->n[i] != save->n_fields) data[i] = UNDEFINED;
    	else data[i] = factor * save->sum[i];
#ifdef DEBUG
        if (i < 10) printf("data[%d]=%lf n[%d]=%d, sum[%d]=%lf\n",
	    i,data[i],i,save->n[i],i,save->sum[i]);
#endif
    }

    pdt = GB2_ProdDefTemplateNo(save->first_sec);

    // average of a forecast

    if (pdt == 0) {
        sec4 = (unsigned char *) malloc(58 * sizeof(unsigned char));
	if (sec4 == NULL) fatal_error("fcst_ave: memory allocation","");
	for (i = 0; i < 34; i++) {
	    sec4[i] = save->first_sec[4][i];
	}
	uint_char((unsigned int) 58, sec4);		// length
	sec4[8] = 8;			// pdt
	// verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+34);
	sec4[41] = 1;					// 1 time range
	uint_char(save->n_missing, sec4+42);
	sec4[46] = 0;					// average
	sec4[47] = 2;					// rt=constant, ft++
	sec4[48] = save->dt_unit;					// total length of stat processing
	uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+49);
	sec4[53] = save->dt_unit;					// time step
	uint_char(save->dt, sec4+54);
    }

    // average of an ensemble forecast, use pdt 4.11

    else if (pdt == 1) {
        sec4 = (unsigned char *) malloc(61 * sizeof(unsigned char));
        if (sec4 == NULL) fatal_error("fcst_ave: memory allocation","");
        for (i = 0; i < 37; i++) {
            sec4[i] = save->first_sec[4][i];
        }
        uint_char((unsigned int) 61, sec4);             // length
        sec4[8] = 11;                    // pdt
        // verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+37);
        sec4[44] = 1;                                   // 1 time range
        uint_char(save->n_missing, sec4+45);
        sec4[49] = 0;                                   // average
        sec4[50] = 2;                                   // rt=constant, ft++
        sec4[51] = save->dt_unit;                                       // total length of stat processing
        uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+52);
        sec4[56] = save->dt_unit;                                       // time step
        uint_char(save->dt, sec4+57);
    }

    // average of an average or accumulation

    else if (pdt == 8) {
	i = GB2_Sec4_size(save->first_sec);
	n = save->first_sec[4][41];
	if (i != 46 + 12*n) fatal_error("ave: invalid sec4 size for pdt=8","");

        // keep pdt == 8 but make it 12 bytes bigger
        sec4 = (unsigned char *) malloc( (i+12) * sizeof(unsigned char));
	if (sec4 == NULL) fatal_error("fcst_ave: memory allocation","");

	uint_char((unsigned int) i+12, sec4);		// new length

	for (i = 4; i < 34; i++) {			// keep base of pdt
	    sec4[i] = save->first_sec[4][i];
	}
	
	// new verification time
        save_time(save->year2,save->month2,save->day2,save->hour2,save->minute2,save->second2, sec4+34);

	// number of stat-proc loops is increased by 1
	sec4[41] = n + 1;

	// copy old stat-proc loops 
	// for (j = n*12-1;  j >= 0; j--) sec4[58+j] = save->first_sec[4][46+j];
	for (j = 0; j < n*12; j++) sec4[46+12+j] = save->first_sec[4][46+j];

#ifdef DEBUG
printf("save->n_missing =%d save->n_fields=%d\n",save->n_missing,save->n_fields);
#endif
	uint_char(save->n_missing, sec4+42);
	sec4[46] = 0;			// average
	sec4[47] = 2;			// fcst++
	sec4[48] = save->dt_unit;						// total length of stat processing
	uint_char(save->dt*(save->n_fields+save->n_missing-1), sec4+49);	// missing
	sec4[53] = save->dt_unit;						// time step
	uint_char(save->dt, sec4+54);

    }
    else {
	fatal_error_i("ave with pdt %d is not supported",pdt);
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
    free(sec4);
    return 0;
}

/*
 * HEADER:000:fcst_ave0:output:2:average X=time step, Y=output grib file needs file is special order
 */

int f_fcst_ave0(ARG2) {

    struct ave_struct *save;

    int i, pdt, new_type;
    int year, month, day, hour, minute, second;
    int tyear, tmonth, tday, thour, tminute, tsecond;
    int missing;
    char string[10];

    // initialization

    if (mode == -1) {
        save_translation = decode = 1;

	// allocate static structure

        *local = save = (struct ave_struct *) malloc( sizeof(struct ave_struct));
        if (save == NULL) fatal_error("memory allocation fcst_ave","");

	i = sscanf(arg1, "%d%2s", &save->dt,string);
	if (i != 2) fatal_error("fcst_ave: delta-time: (int)(2 characters) %s", arg1);
	save->dt_unit = -1;
	if (strcmp(string,"hr") == 0) save->dt_unit = 1;
	else if (strcmp(string,"dy") == 0) save->dt_unit = 2;
	else if (strcmp(string,"mo") == 0) save->dt_unit = 3;
	else if (strcmp(string,"yr") == 0) save->dt_unit = 4;
	if (save->dt_unit == -1) fatal_error("fcst_ave: unsupported time unit %s", string);

        if (fopen_file(&(save->out), arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }

	save->has_val = 0;
	save->n = NULL;
	save->sum = NULL;
        init_sec(save->first_sec);
        init_sec(save->next_sec);

	return 0;
    }

    save = (struct ave_struct *) *local;

    if (mode == -2) {			// cleanup
	if (save->has_val == 1) {
	    do_ave(save);
	}
	fclose_file(&(save->out));
	free_ave_struct(save);
	return 0;
    }

    // if data
    if (mode >= 0) {
	// 1/2015 use_scale = 0;
	pdt = GB2_ProdDefTemplateNo(sec);

if (mode == 98) fprintf(stderr,"fcst_ave: pdt=%d\n",pdt);
	// only support pdt == 0, 1 and 8
	if (pdt != 0 && pdt != 1 && pdt != 8) return 0;

	// first time through .. save data and return


	if (save->has_val == 0) {		// new data: write and save
	    init_ave_struct(save, ndata);
	    add_to_ave_struct(save, sec, data, ndata, 0);

	    // copy sec
            copy_sec(sec, save->first_sec);
            copy_sec(sec, save->next_sec);

	    // get reference time and save it
            get_time(sec[1]+12,&save->ref_year, &save->ref_month, &save->ref_day, &save->ref_hour, &save->ref_minute, &save->ref_second);

	    if (start_ft(sec, &save->fcst_year, &save->fcst_month, &save->fcst_day, &save->fcst_hour, 
			&save->fcst_minute, &save->fcst_second) != 0) {
		fatal_error("fcst_ave: could not determine the start FT time","");
	    }

	    // get verf time and save it
	    if (verftime(sec, &save->year2, &save->month2, &save->day2, &save->hour2, &save->minute2, &save->second2) != 0) {
		fatal_error("fcst_ave: could not determine the verification time","");
	    }

	    save->has_val = 1;
	    return 0;
	}

	// check to see if new variable

        new_type = 0;

	// get the reference time of field
	get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

	// see if reference time has not changed
        if (year != save->ref_year) new_type = 1;
        else if (month != save->ref_month) new_type = 1;
        else if (day != save->ref_day) new_type = 1;
        else if (hour != save->ref_hour) new_type = 1;
        else if (minute != save->ref_minute) new_type = 1;
        else if (second != save->ref_second) new_type = 1;

        if (new_type == 0) {
	    if (same_sec0(sec,save->first_sec) == 0 ||
                same_sec1_not_time(mode,sec,save->first_sec) == 0 ||
                same_sec3(sec,save->first_sec) == 0 ||
                same_sec4_not_time(mode,sec,save->first_sec) == 0) 
	        new_type = 1;
if (mode == 98) fprintf(stderr, "fcst_ave: testsec %d %d %d %d\n", same_sec0(sec,save->first_sec),
                same_sec1_not_time(0,sec,save->first_sec),
                same_sec3(sec,save->first_sec),
                same_sec4_not_time(0,sec,save->first_sec));
        }
if (mode == 98) fprintf(stderr, "fcst_ave: new_type %d\n", new_type);

	// unlike f_ave, assume no missing .. it is a fcst not observations
	// check to see if verification date is expected value

	if (new_type == 0) {
	    tyear = save->fcst_year;
	    tmonth = save->fcst_month;
	    tday = save->fcst_day;
	    thour = save->fcst_hour;
	    tminute = save->fcst_minute;
	    tsecond = save->fcst_second;
            add_time(&tyear, &tmonth, &tday, &thour, &tminute, &tsecond, save->dt, save->dt_unit);
	    if (start_ft(sec, &year, &month, &day, &hour, &minute, &second) != 0) 
		fatal_error("fcst_ave: could not determine the start_ft time","");
            if (cmp_time(year,month,day,hour,minute,second,tyear,tmonth,tday,thour,tminute,tsecond)) {
		new_type = 1;
if (mode == 98) fprintf(stderr, "fcst_ave: unexpected verf time, new_type=1\n");
	    }
	    else {
	        save->fcst_year = year;
	        save->fcst_month = month;
	        save->fcst_day = day;
	        save->fcst_hour = hour;
	        save->fcst_minute = minute;
	        save->fcst_second = second;
	    }
	}

	// check to see if verification date is expected value

        missing = 0;
	if (new_type == 0) {
	    if (verftime(sec, &year, &month, &day, &hour, &minute, &second) != 0) 
		fatal_error("fcst_ave: could not determine the verification time","");
	    tyear = save->year2;
	    tmonth = save->month2;
	    tday = save->day2;
	    thour = save->hour2;
	    tminute = save->minute2;
	    tsecond = save->second2;
            add_time(&tyear, &tmonth, &tday, &thour, &tminute, &tsecond, save->dt, save->dt_unit);
            if (cmp_time(year,month,day,hour,minute,second,tyear,tmonth,tday,thour,tminute,tsecond) != 0) new_type = 1;
	    else {
	        save->year2 = year;
	        save->month2 = month;
	        save->day2 = day;
	        save->hour2 = hour;
	        save->minute2 = minute;
	        save->second2 = second;
	    }
	}

	// if data is the same as the previous, update the sum

if (mode == 98) fprintf(stderr, "fcst_ave ave: before update_sum  new_type %d\n", new_type);

	if (new_type == 0) {		// update sum
if (mode == 98) fprintf(stderr, "fcst_ave: update sum\n");
	    add_to_ave_struct(save, sec, data, ndata, missing);
	    return 0;
	}

	// new field, do grib output and save current data

	do_ave(save);
        init_ave_struct(save, ndata);
	add_to_ave_struct(save, sec, data, ndata, 0);
        copy_sec(sec, save->first_sec);
        copy_sec(sec, save->next_sec);

        // get reference time and save it
        get_time(sec[1]+12,&save->ref_year, &save->ref_month, &save->ref_day, &save->ref_hour, &save->ref_minute, &save->ref_second);
        if (start_ft(sec, &save->fcst_year, &save->fcst_month, &save->fcst_day, &save->fcst_hour,
                     &save->fcst_minute, &save->fcst_second) != 0) {
           fatal_error("fcst_ave: could not determine the start FT time","");
        }

        // get verf time and save it
        if (verftime(sec, &save->year2, &save->month2, &save->day2, &save->hour2, &save->minute2, &save->second2) != 0) {
            fatal_error("fcst_ave: could not determine the verification time","");
        }

        save->has_val = 1;
	return 0;
    }
    return 0;
}
