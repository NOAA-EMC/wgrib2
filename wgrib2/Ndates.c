#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Ndates.c
 *
 * print out a list of date codes
 *   for (datecode = INITIAL_DATECODE; datecode < INITIAL_DATECODE + DT1; datecode += DT2)
 *
 * 1/2019 Public Domain by Wesley Ebisuzaki
 *
 */

#define max(a,b) (a >= b) ? (a) : (b)

char ndates_fmt[NAMELEN];

/*
 * HEADER:100:ndates:misc:3:X=date0 Y=(date1|dt1) Z=dt2  for (date=date0; date<(date1|date0+dt1); date+=dt2) print date
 */

int f_ndates(ARG3) {

    int year, month, day, hour, minute, second, i;
    int year_end, month_end, day_end, hour_end, minute_end, second_end;
    int dtime, dt_unit, strlen_arg1, strlen_arg2, format;
    char string[3], out[15];
    /* only does calculation in initialization phase */

    if (mode == -1) {

        /* get starting datecode */
        strlen_arg1 = strlen(arg1);
	for (i = 0; i < strlen_arg1; i++) {
	    if (isdigit(arg1[i]) == 0) break;
	}
	if (i != strlen_arg1) fatal_error("ndates: illegal character, starting date %s", arg1);
	day = month = 1;
        hour = minute = second = 0;
	
	/* format =	1 YYYY
			2 YYYYMM
			3 YYYYMMDD
			4 YYYYMMDDHH
			5 YYYYMMDDHHmm
			6 YYYYMMDDHHmmss 
	 */
	format = (strlen_arg1-2)/2;
        if (strlen_arg1 % 2 == 1 || strlen_arg1 < 4 || strlen_arg1 > 14) 
	   fatal_error("ndates: arg1 (YYYY|YYYYMM|YYYYMMDD|YYYYMMDDHH|YYYYMMDDHHmm|YYYYMMDDHHmmss)","");
	sscanf(arg1, "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);

	/* get ending date */

        strlen_arg2 = strlen(arg2);
 
	for (i = 0; i < strlen_arg2; i++) {
	    if (isdigit(arg2[i]) == 0) break;
	}

	// ending date = date code

	if (i == strlen_arg2) {
	    day_end = month_end = 1;
            hour_end = minute_end = second_end = 0;
            if (strlen_arg2 % 2 == 1 || strlen_arg2 < 4 || strlen_arg2 > 14) 
	        fatal_error("ndates: arg2 (YYYY|YYYYMM|YYYYMMDD|YYYYMMDDHH|YYYYMMDDHHmm|YYYYMMDDHHmmss)","");
	    sscanf(arg2, "%4d%2d%2d%2d%2d%2d", &year_end, &month_end, &day_end, &hour_end, &minute_end, &second_end);
	}

	// ending date = starting date + offset

	else {
            year_end = year;
	    month_end = month;
	    day_end = day;
	    hour_end = hour;
	    minute_end = minute;
	    second_end = second;
            i = sscanf(arg2, "%d%2s", &dtime, string);
            string[2] = 0;
            if (i != 2 || dtime < 0) fatal_error("ndates: bad (int)(mn|hr|dy|mo|yr)","");
            dt_unit = -1;
            if (strcmp(string,"hr") == 0) dt_unit = 1;
            else if (strcmp(string,"dy") == 0) dt_unit = 2;
            else if (strcmp(string,"mo") == 0) dt_unit = 3;
            else if (strcmp(string,"yr") == 0) dt_unit = 4;
            else if (strcmp(string,"mn") == 0) dt_unit = 0;
            if (dt_unit == -1) fatal_error("ndates: unsupported time unit %s", string);
	    i = add_dt(&year_end, &month_end, &day_end, &hour_end, &minute_end, &second_end, dtime, dt_unit);
	}

	/* get step_dt */

        i = sscanf(arg3, "%d%2s", &dtime, string);
        string[2] = 0;
        if (i != 2 || dtime < 0) fatal_error("ndates: bad (int)(mn|hr|dy|mo|yr)","");
        dt_unit = -1;


        if (strcmp(string,"yr") == 0) dt_unit = 4;
        else if (strcmp(string,"mo") == 0) {
	    dt_unit = 3;
	    format = max(format,2);
	}
        else if (strcmp(string,"dy") == 0) {
	    dt_unit = 2;
	    format = max(format,3);
	}
        else if (strcmp(string,"hr") == 0) {
	    dt_unit = 1;
	    format = max(format,4);
	}
        else if (strcmp(string,"mn") == 0) {
	    dt_unit = 0;
	    format = max(format,5);
	}

        if (dt_unit == -1) fatal_error("ndates: unsupported time unit %s", string);
        
	while (cmp_time(year, month, day, hour, minute, second,
		year_end, month_end, day_end, hour_end, minute_end, second_end) == -1) {

            if (format == 1) {
		sprintf(out, "%.4d", year);
	    }
            else if (format == 2) {
		sprintf(out, "%.4d%.2d", year, month);
	    }
            else if (format == 3) {
		sprintf(out, "%.4d%.2d%.2d", year, month, day);
	    }
	    else if (format == 4) {
                sprintf(out, "%.4d%.2d%.2d%.2d", year, month, day, hour);
	    }
	    else if (format == 5) {
		sprintf(out, "%.4d%.2d%.2d%.2d%.2d", year, month, day, hour, minute);
	    }
	    else {
		sprintf(out, "%.4d%.2d%.2d%.2d%.2d%.2d", year, month, day, hour, minute, second);
	    }
	    sprintf(inv_out, ndates_fmt, out);
	    inv_out += strlen(inv_out);
	    add_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
        }
    }
    return 0;
}

/*
 * HEADER:100:ndates_fmt:misc:1:X = C format for ndates option
 */

int f_ndates_fmt(ARG1) {
    const char *in;
    char *out;

    if (mode == -2) return 0;
    if (mode == -1 && strlen(arg1) > NAMELEN-1) 
            fatal_error("ndates_fmt: format too long: %s", arg1);

    in = arg1;
    out = &(ndates_fmt[0]);
    while (*in) {
	if (*in != '\\') {
	    *out++ = *in++;
	}
	else {
	    if (in[1] == 'n') {
		*out++ = '\n';
		in += 2;
	    }
	    else if (in[1] == '\\') {
		*out++ = '\\';
		in += 2;
	    }
	    else if (in[1] == 't') {
		*out++ = '\t';
		in += 2;
	    }
	    else {
		*out++ = *in++;
  	    }
	}
    }
    *out = 0;
    return 0;
}
