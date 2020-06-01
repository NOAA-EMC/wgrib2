#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"


/* some routines to check whether two fields are the same */


int same_sec0(unsigned char **sec_a, unsigned char **sec_b) {

    unsigned char *a, *b;
    int i;
    a = sec_a[0];
    b = sec_b[0];
    for (i = 0; i < 8; i++) {
	if (*a++ != *b++) return 0;
    }
    return 1;
}

int same_sec1(unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int i;
    i = GB2_Sec1_size(sec_a);
    if (GB2_Sec1_size(sec_b) != i) return 0;
    a = sec_a[1];
    b = sec_b[1];
    while (i--) {
	if (*a++ != *b++) return 0;
    }
    return 1;
}

// test to see if same sec1 but don't do time stamp

int same_sec1_not_time(int mode, unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int i, j;
    i = GB2_Sec1_size(sec_a);
    if (GB2_Sec1_size(sec_b) != i) return 0;
    a = sec_a[1];
    b = sec_b[1];
    for (j = 0; j < 12; j++) {
	if (a[j] != b[j]) {
	    if (mode) fprintf(stderr,"same_sec1_not_time: sec 1 octet %d = %u vs %u\n", j+1, a[j],b[j]);
	    return 0;
	}
    }
    for (j = 19; j < i; j++) {
	if (a[j] != b[j]) {
	    if (mode) fprintf(stderr,"same_sec1_not_time: sec 1 octet %d = %u vs %u\n", j+1, a[j],b[j]);
	    return 0;
	}
    }
    return 1;
}

int same_sec2(unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int i;
    i = GB2_Sec2_size(sec_a);
    if (GB2_Sec2_size(sec_b) != i) return 0;
    a = sec_a[2];
    b = sec_b[2];
    while (i--) {
        if (*a++ != *b++) return 0;
    }
    return 1;
}


int same_sec3(unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int i;
    i = GB2_Sec3_size(sec_a);
    if (GB2_Sec3_size(sec_b) != i) return 0;
    a = sec_a[3];
    b = sec_b[3];
    while (i--) {
        if (*a++ != *b++) return 0;
    }
    return 1;
}

int same_sec4(unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int i;

    i = GB2_Sec4_size(sec_a);
    if (GB2_Sec4_size(sec_b) != i) return 0;
    a = sec_a[4];
    b = sec_b[4];
    while (i--) {
        if (*a++ != *b++) return 0;
    }
    return 1;
}

/*
   check to see if the two section 4 are the same
   this version ignores time code (fcst hour and the time code in the stat processing)
   returns 1 if the same.
   modified 8/2013
 */

int same_sec4_not_time(int mode, unsigned char **sec_a, unsigned char **sec_b) {
 
    unsigned char *a, *b, *p;
    unsigned int i, j;
    int pdt;
    static int warning = 0; 
    int code_4_4, stat_time;

    /* check size of sec4 */
    i = GB2_Sec4_size(sec_a);
    if (GB2_Sec4_size(sec_b) != i) return 0;

    /* check pdt match */
    pdt = GB2_ProdDefTemplateNo(sec_a);
    if (GB2_ProdDefTemplateNo(sec_b) != pdt) return 0;

    a = sec_a[4];
    b = sec_b[4];

    p = stat_proc_verf_time_location(sec_a);
    stat_time = p ? p - a : 0;			/* stat_time == 0 -> not available */

    /* statistical processing */
    if (stat_time) {
        p = code_table_4_4_location(sec_a);
	if (p == NULL) fatal_error_i("same_sec4_not_time, prog error 1 pdt=%d", pdt);
	code_4_4 = p - a;

        if (code_4_4 > stat_time) fatal_error_i("same_sec4_not_time, prog error 2 pdt=%d", pdt);

	for (j = 0; j < code_4_4; j++) {
	    if (a[j] != b[j]) {
	        if (mode) fprintf(stderr,"same_sec4_not_time: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
		return 0;
	    }
	}
	for (j = code_4_4 + 5; j < stat_time; j++) {
	     if (a[j] != b[j]) {
	        if (mode) fprintf(stderr,"same_sec4_not_time: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
		return 0;
             }
	}
	for (j = stat_time + 7; j < i; j++) {
	    if (a[j] != b[j]) {
	        if (mode) fprintf(stderr,"same_sec4_not_time: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
		return 0;
	    }
	}
        return 1;
    }


    p = code_table_4_4_location(sec_a);
    code_4_4 = p ? p - a : 0;

    if (code_4_4 == 0) {		/* no code table 4.4 (forecast time) ex radar, satellite prod*/
	for (j = 0; j < i; j++) {
	    if (a[j] != b[j]) {
	        if (mode) fprintf(stderr,"same_sec4_not_time: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
		return 0;
	    }
	}
	return 1;
    }
    else {				/* has forecast time but not stat processing */


	for (j = 0; j < code_4_4; j++) {
	    if (a[j] != b[j]) {
	        if (mode) fprintf(stderr,"same_sec4_not_time: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
		return 0;
	    }
	}

	for (j = code_4_4 + 5; j < i; j++) {
	    if (a[j] != b[j]) {
	        if (mode) fprintf(stderr,"same_sec4_not_time: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
		return 0;
	    }
	}
	return 1;
    }

    if (warning == 0) {
	warning = 1;
	fprintf(stderr,"same_sec4_not_time does not handle pdt=%d",pdt);
    }
    return 0;
}

/*
 *  check to see if sec4 is the same except allowing for different ave/acc period
 *   if different, return 0
 *   if not statistically processed PDT, return 0
 *   if same, return 1
 */

int same_sec4_diff_ave_period(unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int size, j;
    int idx;

    if (GB2_Sec4_size(sec_b) != (size = GB2_Sec4_size(sec_a)) ) return 0;
    if (GB2_ProdDefTemplateNo(sec_a) != GB2_ProdDefTemplateNo(sec_b) ) return 0;
    if ((idx = stat_proc_n_time_ranges_index(sec_a)) < 0) return 0;
    a = sec_a[4];
    b = sec_b[4];

    // check for data up to year of end of overall time interval
    for (j = 0; j < idx - 7 ; j++) {
	if (a[j] != b[j]) return 0;
    }

    //  skip time of end of overall time interval * 7 bytes

    // check n time ranges to secoded code table 4.4
    for (j = idx; j < idx+8;  j++) {
	if (a[j] != b[j]) return 0;
    }

    //  skip length of time range

    for (j = idx+12; j < size; j++) {
	if (a[j] != b[j]) return 0;
    }
    return 1;
}

// test for same sec4 merge

int same_sec4_for_merge(int mode, unsigned char **sec_a, unsigned char **sec_b) {
    unsigned char *a, *b;
    unsigned int j, size, code_4_4;
    int pdt, idx;

    if (GB2_Sec4_size(sec_b) != (size = GB2_Sec4_size(sec_a)) ) return 0;
    if (GB2_ProdDefTemplateNo(sec_a) != GB2_ProdDefTemplateNo(sec_b) ) return 0;
    if ((idx = stat_proc_n_time_ranges_index(sec_a)) < 0) return 0;

    a = sec_a[4];
    b = sec_b[4];

    code_4_4 = code_table_4_4_location(sec_a) - a;
    pdt = GB2_ProdDefTemplateNo(sec_a);

    // up to and including code table 4.4
    for (j = 0; j <= code_4_4; j++) {
	if (a[j] != b[j]) {
	     if (mode) fprintf(stderr,"same_sec4_merge: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
	     return 0;
	}
    }

    // ignore forecast hour
    for (j = code_4_4+5; j < idx + 35 - 42; j++) {
	if (a[j] != b[j]) {
	     if (mode) fprintf(stderr,"same_sec4_merge: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
	     return 0;
	}
    }

    // ignore time of end of overall time interval

    for (j = idx; j < size;  j++) {
	if (a[j] != b[j]) {
	     if (mode) fprintf(stderr,"same_sec4_merge: sec 4 octet %d = %u vs %u pdt=%d\n", j+1, a[j],b[j],pdt);
	     return 0;
	}
    }

    return 1;
}

// test for same sec4 but different ensemble member
//
// v1: true if same execept for ensemble member number
//     or true if same (no ensemble number)
//
// v2: support for LAF ensembles
//     code that calls must check for same verification time
//     true if same except for ensemble member number or forecast time


int same_sec4_but_ensemble(int mode, unsigned char **sec_a, unsigned char **sec_b) {
    unsigned int i, size;
    unsigned char *p, *fcst_time, *f1, *f2, *f3, *f4, *pert;
    unsigned char *code_4_4;
    int fcst_size;

    if (GB2_Sec4_size(sec_b) != (size = GB2_Sec4_size(sec_a)) ) return 0;
    if (GB2_ProdDefTemplateNo(sec_a) != GB2_ProdDefTemplateNo(sec_b) ) return 0;

    pert = perturbation_number_location(sec_a);
    code_4_4 = code_table_4_4_location(sec_a);
    fcst_time =  forecast_time_in_units_location(sec_a, &fcst_size);
    f1 = f2 = f3 = f4 = NULL;
    if (fcst_time) {
	/* size should be 2 or 4 */
	f1 =  fcst_time + 0;
	f2 =  fcst_time + 1;
        if (fcst_size == 4) {
	   f3 =  fcst_time + 2;
	   f4 =  fcst_time + 3;
	}
    }
    if (mode == 98) {
	 if (pert) fprintf(stderr,"same_sec4_but_ensemble: pert=%ld\n",pert-sec_a[4]);
	 if (code_4_4) fprintf(stderr,"same_sec4_but_ensemble: code_4.4=%ld\n",code_4_4-sec_a[4]);
	 fprintf(stderr,"same_sec4_but_ensemble: size=%d\n",size);
    }

    for (i = 0; i < size; i++) {
        p = sec_a[4]+i;
        if (p != pert && p != code_4_4 && p != f1 && p != f2 && p != f3 && p != f4) {
	    if (*p != sec_b[4][i]) {
	        if (mode) fprintf(stderr,"same_sec4_but_ensemble: i=%d", i);
		return 0;
	    }
            if (mode == 98) fprintf(stderr,"same_sec4_but_ensemble: byte %d is ok\n",i);
	}
    } return 1;
}
