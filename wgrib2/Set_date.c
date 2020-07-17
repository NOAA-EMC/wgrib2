#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Set_date.c
 *
 * routines to modify grib fields
 *
 * 3/2008 Public Domain by Wesley Ebisuzaki
 * 3/2016 Wesley Ebisuzaki: allow +(dt) and -(dt)
 *
 */


/*
 * HEADER:100:set_date:misc:1:changes date code, X=(+|-)N(hr|dy|mo|yr), YYYYMMDDHHmmSS
 */

int f_set_date(ARG1) {

    int year, month, day, hour, minute, second, i, j, units, n;
    int code_4_11,idx;
    int dtime, dt_unit;
    char string[10];

    if (mode < 0) return 0;

    reftime(sec, &year, &month, &day, &hour, &minute, &second);

    if (arg1[0] == '+' || arg1[0] == '-') {
        i = sscanf(arg1+1, "%d%2s", &dtime, string);
        if (i != 2 || dtime < 0) fatal_error("set_date: delta-time: (+|-)(int)(hr|dy|mo|yr)","");
        dt_unit = -1;
        if (strcmp(string,"hr") == 0) dt_unit = 1;
        else if (strcmp(string,"dy") == 0) dt_unit = 2;
        else if (strcmp(string,"mo") == 0) dt_unit = 3;
        else if (strcmp(string,"yr") == 0) dt_unit = 4;
        else if (strcmp(string,"mn") == 0) dt_unit = 0;
        if (dt_unit == -1) fatal_error("set_date: unsupported time unit %s", string);
	if (arg1[0] == '+')
    		i = add_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
	else
    		i = sub_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
    }

    else {
        i=strlen(arg1);
        if (i < 4 || i > 14 || i % 2 == 1) fatal_error("set_date: bad date code %s",arg1); 
	/* override date codes, if available */
        i = sscanf(arg1,"%4d%2d%2d%2d%2d%2d" , &year, &month, &day, &hour, &minute, &second);
        if (i < 1) fatal_error("set_date: bad date code %s",arg1); 

        if (check_datecode(year, month, day) != 0 || hour < 0 || hour >= 24 ||
	    minute < 0 || minute >= 60 || second < 0 || second >= 60) 
		fatal_error("set_date: bad date code %s",arg1);
    }

    // set reference time
    save_time(year,month,day,hour,minute,second, sec[1]+12);

    idx =  stat_proc_n_time_ranges_index(sec);
    if (idx < 0) return 0;		// not a stat processed template
    n = (int) sec[4][idx];		// number of stat proc elements
    j = idx + 35 - 42;

    // add forecast time to time

    units = code_table_4_4(sec);
    dtime = forecast_time_in_units(sec);
    add_time(&year, &month, &day, &hour, &minute, &second, (unsigned int) dtime, units);

    for (i = 0; i < n; i++) {
        // add statistical processing time to time
        code_4_11 = (int) sec[4][47-34+j+i*12];
        units = (int) sec[4][48-34+j+i*12];
        dtime = int4(sec[4]+49-34+j+i*12);
        if (code_4_11 == 3 || code_4_11 == 4) continue;
        if (code_4_11 == 1 || code_4_11 == 2) {
	    add_time(&year, &month, &day, &hour, &minute, &second, (unsigned int) dtime, units);
	}
        else {
	    fatal_error_i("set_date: code 4.11=%d is not supported", code_4_11);
	}
    }

    save_time(year,month,day,hour,minute,second, sec[4]+j);
    return 0;
}
