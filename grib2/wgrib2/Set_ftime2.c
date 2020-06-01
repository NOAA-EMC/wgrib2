#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * the set options
 *
 * set_ftime2, change forecast time
 *
 * 4/2016 rewrite of -set_ftime and -set_ave
 *
 */


struct stat_proc {
	int code_4_10;				/* ave, min, max, etc */
	int code_4_11;				/* fcst_ave, analysis_ave, LAF+, LAF- */
	int code_4_4a;				/* time units such as hour, day, etc */ 	
	int d_timea;				/* length of time for statistical processing */
	int code_4_4b;				/* time units for sucessive fields */
	int d_timeb;				/* length of time for sucessive fields */
};
	
#define N_MAX 10

struct stat_time {
   struct full_date ref_time;		/* reference time */
   struct full_date verf_time;		/* end of overall processing time */
   int code_4_4_fcst;
   int d_time_fcst;
   int n;				/* number of stat proc specifications 0.. */
   int n_max;				/* max number of stat proc specifications */
   unsigned int missing;		/* 4 byte value */
   struct stat_proc stat_procs[N_MAX];
};
    
int parse_time_range(const char *tr, struct stat_time *new_stat_time, int n);
int parse_time_range2(const char *tr, struct stat_time *new_stat_time, int n);

/*
 * HEADER:100:set_ftime2:misc:1:set ftime2 .. will be replace -set_ftime/ave in the future -- TESTING ---
 */

int f_set_ftime2(ARG1) {

    struct stat_time new_stat_time;

    int i, len, len_arg1, pdt, n, old_n;
    char string[STRING_SIZE], string2[STRING_SIZE];
    char pdt_string[STRING_SIZE];
    unsigned char *code_4_4, *code_1_2, *code_1_4, *verf_time;
    int new_ftime, new_ftime_units;

    if (mode < 0) return 0;

    len_arg1 = strlen(arg1);
    pdt = GB2_ProdDefTemplateNo(sec);

    new_ftime = 0;
    new_ftime_units  = -1;

    /* for a point in time:    see if "anl" or "N hours fcst" */

    if (strcmp(arg1 ,"anl") == 0) {
	new_ftime = 0;
	new_ftime_units = 1;
    }
    else {
        if (sscanf(arg1,"%d %s %s%n", &new_ftime , string, string2, &len) == 3 && 
		len == len_arg1) {
    	    if (strcmp(string2,"forecast") == 0 || strcmp(string2,"fcst") == 0) {
		new_ftime_units = a2time_range(string);
	    }
	}
    }

    /* update metadata for with a PDT for a point in time */

    if (new_ftime_units != -1) {

	/* change PDTs with time range to to PDT with point in time */

	if (pdt == 8) f_set_pdt(call_ARG1(inv_out, NULL, "+0"));
	else if (pdt == 9) f_set_pdt(call_ARG1(inv_out, NULL, "+5"));
	else if (pdt == 10) f_set_pdt(call_ARG1(inv_out, NULL, "+6"));
	else if (pdt == 11) f_set_pdt(call_ARG1(inv_out, NULL, "+1"));
	else if (pdt == 12) f_set_pdt(call_ARG1(inv_out, NULL, "+2"));
	else if (pdt == 13) f_set_pdt(call_ARG1(inv_out, NULL, "+3"));
	else if (pdt == 14) f_set_pdt(call_ARG1(inv_out, NULL, "+4"));
	else if (pdt == 34) f_set_pdt(call_ARG1(inv_out, NULL, "+33"));
	else if (pdt == 42) f_set_pdt(call_ARG1(inv_out, NULL, "+40"));
	else if (pdt == 43) f_set_pdt(call_ARG1(inv_out, NULL, "+41"));
	else if (pdt == 45) f_set_pdt(call_ARG1(inv_out, NULL, "+44"));
	else if (pdt == 46) f_set_pdt(call_ARG1(inv_out, NULL, "+48"));
	else if (pdt == 61) f_set_pdt(call_ARG1(inv_out, NULL, "+60"));

        code_4_4 = code_table_4_4_location(sec);
        if (code_4_4 == NULL) fatal_error("set_ftime2: Code Table 4.4 not present or defined","");	

        verf_time = stat_proc_verf_time_location(sec);
        if (verf_time != NULL) fatal_error_i("set_ftime2: could not convert to point in time pdt %d", pdt);

	*code_4_4 = (unsigned char) new_ftime_units;
        code_1_2 = code_table_1_2_location(sec);
        code_1_4 = code_table_1_4_location(sec);

	if (pdt != 44) { int_char(new_ftime,code_4_4 + 1); }
	else { int2_char(new_ftime,code_4_4 + 1); }
    	if (new_ftime == 0) {
	    if (code_1_2 != NULL) *code_1_2 = 0;					// start of analysis
	    if (code_1_4 != NULL && (int) *code_1_4 == 1) *code_1_4 = 0;		// fcst product -> analysis product
	}
	else {
	    if (code_1_2 != NULL) *code_1_2 = 1;					// start of forecast 
	    if (code_1_4 != NULL && (int) *code_1_4 == 0) *code_1_4 = 1;		// analysis product -> fcst product
	}
	return 0;
    }

    /* must be a time range: */

    i = Ref_time(sec, &(new_stat_time.ref_time));
    new_stat_time.verf_time = new_stat_time.ref_time;

    i = parse_time_range(arg1, &new_stat_time, 0);

    if (i) fatal_error("set_ftime2: could not understand forecast timing (%s)",arg1);

    /* change PDT with point in time to PDT with a time range */
    n = new_stat_time.n + 1;	/* value in struct starts at 0.. */
    verf_time = stat_proc_verf_time_location(sec);
    if (verf_time == NULL) old_n=-1;
    else old_n = (int) verf_time[7];
    
//fprintf(stderr,">>> test if new pdt n=%d old_n=%d\n", n,old_n);
    if (old_n != n) {
	pdt_string[0] = '\0';
//fprintf(stderr,">>need new change pdt\n");
        if (pdt == 0 || pdt == 8) sprintf(pdt_string,"+8:%d", 46+12*n);
        else if (pdt == 1 || pdt == 11) sprintf(pdt_string,"+11:%d", 49+12*n);
	else if (pdt == 2 || pdt == 12) sprintf(pdt_string,"+12:%d", 48+12*n);
	else if (pdt == 3 || pdt == 13) sprintf(pdt_string,"+13:%d", 80+12*n);
	else if (pdt == 4 || pdt == 14) sprintf(pdt_string,"+14:%d", 76+12*n);
	else if (pdt == 5 || pdt == 9) sprintf(pdt_string,"+9:%d", 59+12*n);
	else if (pdt == 6 || pdt == 10) sprintf(pdt_string,"+10:%d", 47+12*n);
	else if (pdt == 41 || pdt == 43) sprintf(pdt_string,"+43:%d", 51+12*n);
	else if (pdt == 48 || pdt == 46) sprintf(pdt_string,"+46:%d", 59+12*n);
	else if (pdt == 60 || pdt == 61) sprintf(pdt_string,"+43:%d", 56+12*n);
//fprintf(stderr,">>pdt_string (%s)\n", pdt_string);
	if (pdt_string[0] != 0) f_set_pdt(call_ARG1(inv_out, NULL, pdt_string));
    }

    pdt = GB2_ProdDefTemplateNo(sec);

    /* fcst time */
    code_4_4 = code_table_4_4_location(sec);
    *code_4_4 = new_stat_time.code_4_4_fcst;
    int_char(new_stat_time.d_time_fcst, code_4_4 + 1);

    if ((verf_time = stat_proc_verf_time_location(sec)) == NULL) fatal_error_i("set_ftime2: could not handle pdt %d", pdt);

    Save_time(&(new_stat_time.verf_time), verf_time);
    verf_time[7] = n;
    uint_char(new_stat_time.missing, verf_time + 8);

//fprintf(stderr,">> fill in pdt n=%d\n", n);


    for (i = 0; i < n; i++) {
	verf_time[12 + i*12] = new_stat_time.stat_procs[i].code_4_10;
	verf_time[13 + i*12] = new_stat_time.stat_procs[i].code_4_11;
	verf_time[14 + i*12] = new_stat_time.stat_procs[i].code_4_4a;
	int_char(new_stat_time.stat_procs[i].d_timea, verf_time + 15 + i*12);
	verf_time[19 + i*12] = new_stat_time.stat_procs[i].code_4_4b;
	int_char(new_stat_time.stat_procs[i].d_timeb, verf_time + 20 + i*12);
    }
    return 0;
}
/*
 * int parse_time_range(const char *arg, int n)
 *  removes ",missing=N"  n should be zero
 */ 

int parse_time_range(const char *arg, struct stat_time *new_stat_time, int n) {
    int i, len;
    char newarg[STRING_SIZE];
    const char *s;

    if (n != 0) fatal_error("parse_time_range: prog error","");

    new_stat_time->missing = 0;

    strncpy(newarg, arg, STRING_SIZE-1);
    newarg[STRING_SIZE-1] = '\0';

    /* remove ,missing=N  suffix */
    /* note: ,missing=N is always included in output but can be optional in input */

    s = arg;
    while (*s) {
	if (s[0] == ',' && s[1] == 'm' && s[2] == 'i' && s[3] == 's' &&
	    s[4] == 's' && s[5] == 'i' && s[6] == 'n' && s[7] == 'g' &&
	    s[8] == '=') {
	    new_stat_time->missing = atoi(s+9);
            len = s - arg;
	    for (i = 0; i < len; i++) {
		newarg[i] = arg[i];
	    }
	    newarg[len] = '\0';
	    break;
        }
        s++;
    }
    i = parse_time_range2(newarg, new_stat_time, n);
    return i;
}

/*
 *   parse_time_range2
 *
 *   recursive parser - fills in new_stat_time->stat_procs[n];
 *     the statistical processing format is nested, stat_procs[0] is the outer definition
 */

int parse_time_range2(const char *arg, struct stat_time *new_stat_time, int n) {

    int i, j, m, k, len, len_arg;
    int tr, tr2, code_4_10, anl_fcst;
    char string1[STRING_SIZE], string2[STRING_SIZE], string3[STRING_SIZE], newarg[STRING_SIZE];

//    fprintf(stderr,">> parse_time_range2 [%s] n=%d\n", arg, n);
    len_arg = strlen(arg);

    // 1-5 hour ave fcst or 1-5 hour ave anl

    i = sscanf(arg,"%d-%d %s %s %s%n",&j,&k,string1,string2,string3,&len);
    if (len != len_arg) i = 0;
    if (i == 5 && ((tr = a2time_range(string1)) >= 0) && ((code_4_10 = a2code_4_10(string2)) >= 0) &&
        ((anl_fcst = a2anl_fcst(string3)) >= 0) ) {
//        fprintf(stderr," >> %d %d %s(%d) %s (%d) %s(%d)\n", j,k,string1, tr, string2, code_4_10, string3, anl_fcst);
//        fprintf(stderr,"::: n=%d\n", n);
	new_stat_time->n = n;

	if (anl_fcst == 0 && j != 0) fatal_error("set_ftime2: illegal format %s, only 0-N allowed", arg);
	/* forecast time */
        new_stat_time->code_4_4_fcst = tr;
        new_stat_time->d_time_fcst = j;

	new_stat_time->stat_procs[n].code_4_10 = code_4_10;
	new_stat_time->stat_procs[n].code_4_11 = anl_fcst == 0 ? 1 : 2;
	new_stat_time->stat_procs[n].code_4_4a = tr;
	new_stat_time->stat_procs[n].d_timea = k-j;
	new_stat_time->stat_procs[n].code_4_4b = 255;
	new_stat_time->stat_procs[n].d_timeb = 0;
//        fprintf(stderr,"::: time range units %d dt=%d \n", tr,k);
	Add_time(&(new_stat_time->verf_time), k, tr);
        return 0;
    }

    // 4.11=1 124@6 hour ave(1 hour fcst)

    i = sscanf(arg,"%d@%d %s %[^(](%d %s fcst)%n",&j,&k,string1,string2, &m, string3, &len);
    if (len != len_arg) i = 0;
    if (i == 6 && ((tr = a2time_range(string1)) >= 0) && ((code_4_10 = a2code_4_10(string2)) >= 0) &&
		((tr2 = a2time_range(string3)) >= 0) ) {

//         fprintf(stderr,"::2: %d %d %s(%d) %s(%d) ( %d %s(%d) )\n", 
//		j,k,string1,tr, string2, code_4_10, m, string3, tr2);

	new_stat_time->n = n;

	/* forecast time */
        new_stat_time->code_4_4_fcst = tr2;
        new_stat_time->d_time_fcst = m;

	new_stat_time->stat_procs[n].code_4_10 = code_4_10;
	new_stat_time->stat_procs[n].code_4_11 = 1;	/* same length of forecast, start of fcst increased */
	new_stat_time->stat_procs[n].code_4_4a = tr;
	new_stat_time->stat_procs[n].d_timea = (j-1) * k;
	new_stat_time->stat_procs[n].code_4_4b = tr;
	new_stat_time->stat_procs[n].d_timeb = k;
	Add_time(&(new_stat_time->verf_time), m, tr2);
	Add_time(&(new_stat_time->verf_time), (j-1)*k, tr);

        return 0;
    }

    // 4.11=1 124@6 hour ave(anl)

    i = sscanf(arg,"%d@%d %s %[^(](anl)%n",&j,&k,string1,string2,&len);
    if (len != len_arg) i = 0;
    if (i == 4 && ((tr = a2time_range(string1)) >= 0) && ((code_4_10 = a2code_4_10(string2)) >= 0) ) {
//        fprintf(stderr,"::3: %d %d %s(%d) %s(%d)\n", j,k,string1,tr, string2, code_4_10);
	new_stat_time->n = n;

	/* forecast time */
        new_stat_time->code_4_4_fcst = 1;	/* hours */
        new_stat_time->d_time_fcst = 0;		/* 0 hours */

	new_stat_time->stat_procs[n].code_4_10 = code_4_10;
	new_stat_time->stat_procs[n].code_4_11 = 1;	/* same length of forecast, start of fcst increased */
	new_stat_time->stat_procs[n].code_4_4a = tr;
	new_stat_time->stat_procs[n].d_timea = (j-1) * k;
	new_stat_time->stat_procs[n].code_4_4b = tr;
	new_stat_time->stat_procs[n].d_timeb = k;
	Add_time(&(new_stat_time->verf_time), (j-1)*k, tr);

        return 0;
    }

    // should be multiple processing

    if (arg[len_arg-1] != ')') {
	fprintf(stderr,"parse_time_range2: not recognized (%s)\n", arg);
	return 1;
    }

    // 124@6 hour ave(something)

    i = sscanf(arg,"%d@%d %s %[^(](%n",&j,&k,string1,string2,&len);
//fprintf(stderr,"::4 n=%d nargs = i=%d j=%d k=%d\n",n,i,j,k);
    if (i == 4 && ((tr = a2time_range(string1)) >= 0) && ((code_4_10 = a2code_4_10(string2)) >= 0) ) {
	for (i = len; i < len_arg-1; i++) {
	    newarg[i-len] = arg[i];
	}
	newarg[i-len] = '\0';
//	fprintf(stderr, ">> recursive [%s] n=%d tr=%d (%s)\n", newarg, n,tr, string1);
	i =  parse_time_range2(newarg, new_stat_time, n+1);
//	fprintf(stderr, "<< recursive [%s]: n=%d i=%d j=%d k=%d tr=%d\n", newarg,n,i,j,k, tr);

	new_stat_time->stat_procs[n].code_4_10 = code_4_10;
	new_stat_time->stat_procs[n].code_4_11 = 1;	/* same length of forecast, start of fcst increased */
	new_stat_time->stat_procs[n].code_4_4a = tr;
	new_stat_time->stat_procs[n].d_timea = (j-1) * k;
	new_stat_time->stat_procs[n].code_4_4b = tr;
	new_stat_time->stat_procs[n].d_timeb = k;
	Add_time(&(new_stat_time->verf_time), (j-1)*k, tr);

	/* update verf time = ref time + k */
	return i;        
    }

    // 4.11=2 1-3 hour ave@(fcst,dt=1 hour)
    i = sscanf(arg,"%d-%d %s %[^@]@(fcst,dt=%d %[^)])", &j,&k, string1, string2, &m, string3);
// fprintf(stderr,"::5 nargs = i=%d %d-%d %s %s (%d unit=%s)\n",i,j,k,string1,string2 ,m, string3);
// fprintf(stderr,"::string1 (%s) string2(%s) m=%d string3(%s)\n",string1,string2, m, string3);
    if (i == 6 && ((tr = a2time_range(string1)) >= 0) && ((code_4_10 = a2code_4_10(string2)) >= 0) 
		&& ((tr2 = a2time_range(string3)) >= 0)) {

//fprintf(stderr,"forecast = %d %s tr=%d", j, string1, tr);
	/* forecast time */
        new_stat_time->d_time_fcst = j;		/* # hours */
        new_stat_time->code_4_4_fcst = tr;	/* hours */

	new_stat_time->stat_procs[n].code_4_10 = code_4_10;
	new_stat_time->stat_procs[n].code_4_11 = 2;	/* length of forecast in, start of fcst same */
	new_stat_time->stat_procs[n].code_4_4a = tr;
	new_stat_time->stat_procs[n].d_timea = (k-j);
	new_stat_time->stat_procs[n].code_4_4b = tr2;
	new_stat_time->stat_procs[n].d_timeb = m;
	/* verification time += k tr1 */
        Add_time(&(new_stat_time->verf_time), k, tr);

        new_stat_time->n = n;
	return 0;
    }





    fatal_error("set_ftime2: no match string=%s", arg);


    // 30@1 year ave(124@6 hour ave(anl)),missing=0:

   // 4.11=1 124@6 hour ave(6 hour fcst),missing=0:
   // 4.11=2 6-744 hour ave@(fcst,dt=6 hour),missing=0:
   // 4.11=3 ensemble ave-3 valid 6 hour,missing=0:
   // 4.11=4 ensemble ave-4 valid 6 hour,missing=0:
   // 4.11=5 6 hour dt 738 hour124@6 hour ave,missing=0:
    // 30@1 year ave(124@6 hour ave(anl)),missing=0:



    return 0;
}
