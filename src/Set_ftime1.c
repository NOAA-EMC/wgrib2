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
 * set_ftime, change forecast time
 *
 * 3/2008 Public Domain by Wesley Ebisuzaki
 * 9/2013 updated for more pdts, sets verf_time, Wesley Ebisuzaki
 *
 */

extern int header, decode;
extern const char *level_table[192];
extern int use_scale, dec_scale, bin_scale;

/*
 * HEADER:100:set_ftime1:misc:1:X='N hour fcst'   (deprecated)
 */

int f_set_ftime1(ARG1) {
    int len, pdt;
    char string[STRING_SIZE];
    char string2[STRING_SIZE];
    unsigned char *code_4_4, *verf_time, *code_1_2, *code_1_4;
    int old_ftime, old_ftime_units;
    int new_ftime, new_ftime_units, dt, units;
    int year, month, day, hour, minute, second;
    int year0, month0, day0, hour0, minute0, second0;

    if (mode < 0) return 0;

    code_4_4 = code_table_4_4_location(sec);
    if (code_4_4 == NULL) fatal_error("set_ftime: Code Table 4.4 not present or defined","");
    pdt = GB2_ProdDefTemplateNo(sec);

    old_ftime_units = (int) *code_4_4;
    old_ftime = int4(code_4_4 + 1);
    verf_time = stat_proc_verf_time_location(sec);

    new_ftime = 0;
    new_ftime_units  = -1;
    if (strcmp(arg1 ,"anl") == 0) {
	new_ftime = 0;
	new_ftime_units = 1;
    }
    else {
        if (sscanf(arg1,"%d %s %s%n", &new_ftime , string, string2, &len) == 3 && 
		len == strlen(arg1)) {
    	    if (strcmp(string2,"forecast") == 0 || strcmp(string2,"fcst") == 0) {
		new_ftime_units = a2time_range(string);
	    }
	}
    }
    if (new_ftime_units == -1) fatal_error("set_ftime: unknown option %s", arg1);

    *code_4_4 = (unsigned char) new_ftime_units;
    if (pdt != 44) { int_char(new_ftime,code_4_4 + 1); }
    else { int2_char(new_ftime,code_4_4 + 1); }


    // significance of reference time
    // type of grib data
    code_1_2 = code_table_1_2_location(sec);
    code_1_4 = code_table_1_4_location(sec);
    if (new_ftime == 0) {
	if (code_1_2 != NULL) *code_1_2 = 0;					// start of analysis
	if (code_1_4 != NULL && (int) *code_1_4 == 1) *code_1_4 = 0;		// fcst product -> analysis product
    }
    else {
	if (code_1_2 != NULL) *code_1_2 = 1;					// start of forecast 
	if (code_1_4 != NULL && (int) *code_1_4 == 0) *code_1_4 = 1;		// analysis product -> fcst product
    }

    // if stat processing .. change verf_time
    if (verf_time == NULL) return 0;

    // old_verf_time = ref_time + old_ftime + stat_proc_dt
    // new_verf_time = ref_time + new_ftime + stat_proc_dt
    
    // stat_proc_dt = old_verf_time - (ref_time + old_ftime);

    reftime(sec, &year0, &month0, &day0, &hour0, &minute0, &second0);
    add_time(&year0, &month0, &day0, &hour0, &minute0, &second0, old_ftime, old_ftime_units);

    get_time(verf_time, &year, &month, &day, &hour, &minute, &second);

    sub_time(year, month, day, hour, minute, second, year0, month0, day0, hour0, 
         minute0, second0, &dt, &units);

    reftime(sec, &year0, &month0, &day0, &hour0, &minute0, &second0);
    add_time(&year0, &month0, &day0, &hour0, &minute0, &second0, new_ftime, new_ftime_units);
    add_time(&year0, &month0, &day0, &hour0, &minute0, &second0, dt, units);
    save_time(year0, month0, day0, hour0, minute0, second0, verf_time);

    return 0;
}

