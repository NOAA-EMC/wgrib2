#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Ndate.c
 *
 * print out a date + offset
 *
 * 1/2019 Public Domain by Wesley Ebisuzaki
 *
 */

#define max(a,b) (a >= b) ? (a) : (b)


/*
 * HEADER:100:ndate:misc:2:X=date Y=dt print date + dt
 */

int f_ndate(ARG2) {

    int year, month, day, hour, minute, second, i;
    int dtime, dt_unit, strlen_arg1, format;
    char string[3];
    /* only does calculation in initialization phase */

    if (mode == -1) {

        /* get starting datecode */
        strlen_arg1 = strlen(arg1);
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
	   fatal_error("ndates: (YYYY|YYYYMM|YYYYMMDD|YYYYMMDDHH|YYYYMMDDHHmm|YYYYMMDDHHmmss)","");
	sscanf(arg1, "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);

	/* get dt */

        i = sscanf(arg2, "%d%2s", &dtime, string);
        string[2] = 0;
        if (i != 2) fatal_error("ndates: bad (int)(mn|hr|dy|mo|yr)","");
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
        
	if (dtime >= 0) 
	add_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
	else sub_dt(&year, &month, &day, &hour, &minute, &second, -dtime, dt_unit);


            if (format == 1) {
		sprintf(inv_out, "%.4d\n", year);
	    }
            else if (format == 2) {
		sprintf(inv_out, "%.4d%.2d\n", year, month);
	    }
            else if (format == 3) {
		sprintf(inv_out, "%.4d%.2d%.2d\n",  year, month, day);
	    }
	    else if (format == 4) {
                sprintf(inv_out, "%.4d%.2d%.2d%.2d\n", year, month, day, hour);
	    }
	    else if (format == 5) {
		sprintf(inv_out, "%.4d%.2d%.2d%.2d%.2d\n", year, month, day, hour, minute);
	    }
	    else {
		sprintf(inv_out, "%.4d%.2d%.2d%.2d%.2d%.2d\n",  year, month, day, hour, minute, second);
	    }
        
    }
    return 0;
}
