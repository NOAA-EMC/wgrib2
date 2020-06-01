#include <stdio.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

static const char *months = "janfebmaraprmayjunjulaugsepoctnovdec";
extern int warn_nonzero_min_sec;

/*
 * HEADER:100:t:inv:0:reference time YYYYMMDDHH, -v2 for alt format
 */

int f_t(ARG0) {

    int year, month, day, hour, minute, second;
    const char *m;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
	if (warn_nonzero_min_sec && (minute != 0 || second != 0)) {
	    fprintf(stderr,"** Warning: reference time includes non-zero minutes/seconds **\n");
	    warn_nonzero_min_sec = 0;
	}
	if (mode != 2) {
	   sprintf(inv_out,"d=%4.4d%2.2d%2.2d%2.2d", year, month, day, hour);
	}
	else {
	   m = months + (month-1)*3;
	   sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour, day, m[0],m[1],m[2],year);
	}
    }
    return 0;
}

/*
 * HEADER:101:T:inv:0:reference time YYYYMMDDHHMMSS
 */

int f_T(ARG0) {
    int year, month, day, hour, minute, second;
    const char *m;
    if (mode == -1) warn_nonzero_min_sec = 0;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
	if (mode != 2) {
	    sprintf(inv_out,"D=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year, month,day,hour,minute,second);
	}
	else {
	   m = months + (month-1)*3;
	   sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour, minute, day, m[0],m[1],m[2],year);
	}
    }
    return 0;
}
/*
 * HEADER:103:YY:inv:0:reference time YYYY
 */
 
int f_YY(ARG0) {
    int year, month, day, hour, minute, second;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        sprintf(inv_out, mode == 0 ? "YY=%4.4d" : "%4.4d", year);
    }
    return 0;
}

/*
 * HEADER:105:MM:inv:0:reference time MM
 */
 
int f_MM(ARG0) {
    int year, month, day, hour, minute, second;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        sprintf(inv_out,"MM=%2.2d",month);
    }
    return 0;
} 

/*
 * HEADER:110:RT:inv:0:type of reference Time
 */
 
int f_RT(ARG0) {
    if (mode >= 0) {
	switch (sec[1][11]) {
	case 0: sprintf(inv_out,"RT=analysis");
		break;
	case 1: sprintf(inv_out,"RT=Start of fcst");
		break;
	case 2: sprintf(inv_out,"RT=Verf time of fcst");
		break;
	case 3: sprintf(inv_out,"RT=Obs time");
		break;
	default:
		break;
	}
    }
    return 0;
}

/*
 * HEADER:110:center:inv:0:center 
 */

int f_center(ARG0) {
    int ctr;
    char tmp[20];
    const char *string;
 
    if (mode >= 0) {
        ctr = GB2_Center(sec);
        switch (ctr) {
#include "code_table0.dat"
	}
	if (mode == 0) {
	    sprintf(inv_out,"center=%d", ctr);
	}
	else {
	    sprintf(inv_out,"center=%s", string);
	}
    }
    return 0;
}

/*
 * HEADER:110:subcenter:inv:0:subcenter 
 */

int f_subcenter(ARG0) {
    int ctr, subctr;
    char tmp[20];
    const char *string;
    
    if (mode >= 0) {
        ctr = GB2_Center(sec);
        subctr = GB2_Subcenter(sec);
	string = NULL;
	if (ctr == 7) {
           switch (subctr) {
#include "ncep_tableC.dat"
	   }
	}
	if (mode == 0 || string == NULL) {
	    sprintf(inv_out,"subcenter=%d", subctr);
	}
	else {
	    sprintf(inv_out,"subcenter=%s", string);
	}
    }
    return 0;
}
