#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "pds4.h"
#include "grib.h"

/*
 * PDS_date.c  v1.2 wesley ebisuzaki
 *
 * prints a string with a date code
 *
 * PDS_date(pds,option, v_time)
 *   options=0  .. 2 digit year
 *   options=1  .. 4 digit year
 *
 *   v_time=0   .. initial time
 *   v_time=1   .. verification time
 *
 * assumption: P1 and P2 are unsigned integers (not clear from doc)
 *
 * v1.2 years that are multiple of 400 are leap years, not 500
 * v1.2.1  make the change to the source code for v1.2
 * v1.2.2  add 3/6/12 hour forecast time units
 * v1.2.3  Jan 31 + 1 month => Feb 31 .. change to Feb 28/29
 */

static int msg_count = 0;
extern int minute;

int PDS_date(unsigned char *pds, int option, int v_time) {

    int year, month, day, hour, min;

    if (v_time == 0) {
        year = PDS_Year4(pds);
        month = PDS_Month(pds);
        day  = PDS_Day(pds);
        hour = PDS_Hour(pds);
    }
    else {
        if (verf_time(pds, &year, &month, &day, &hour) != 0) {
	    if (msg_count++ < 5) fprintf(stderr, "PDS_date: problem\n");
	}
    }
    min =  PDS_Minute(pds);

    switch(option) {
	case 0:
	    printf("%2.2d%2.2d%2.2d%2.2d", year % 100, month, day, hour);
	    if (minute) printf("-%2.2d", min);
	    break;
	case 1:
	    printf("%4.4d%2.2d%2.2d%2.2d", year, month, day, hour);
	    if (minute) printf("-%2.2d", min);
	    break;
	default:
	    fprintf(stderr,"missing code\n");
	    exit(8);
    }
    return 0;
}

#define  FEB29   (31+29)
static int monthjday[13] = {
        0,31,59,90,120,151,181,212,243,273,304,334,365};

static int leap(int year) {
	if (year % 4 != 0) return 0;
	if (year % 100 != 0) return 1;
	return (year % 400 == 0);
}


int add_time(int *year, int *month, int *day, int *hour, int dtime, int unit) {
    int y, m, d, h, jday, i, days_in_month;

    y = *year;
    m = *month;
    d = *day;
    h = *hour;

    if (unit == YEAR) {
	*year = y + dtime;
	return 0;
    }
    if (unit == DECADE) {
	*year =  y + (10 * dtime);
	return 0;
    }
    if (unit == CENTURY) {
	*year = y + (100 * dtime);
	return 0;
    }
    if (unit == NORMAL) {
	*year = y + (30 * dtime);
	return 0;
    }
    if (unit == MONTH) {
        if (dtime < 0) {
           i = (-dtime) / 12 + 1;
           y -= i;
           dtime += (i * 12);
        }
	dtime += (m - 1);
	*year =  y = y + (dtime / 12);
	*month = m = 1 + (dtime % 12);

        /* check if date code if valid */
	days_in_month = monthjday[m] - monthjday[m-1];
	if (m == 2 && leap(y)) {
	    days_in_month++;
	}
	if (days_in_month < d) *day = days_in_month;

	return 0;
    }

    if (unit == SECOND) {
	dtime /= 60;
	unit = MINUTE;
    }
    if (unit == MINUTE) {
	dtime /= 60;
	unit = HOUR;
    }

    if (unit == HOURS3) {
        dtime *= 3;
        unit = HOUR;
    }
    else if (unit == HOURS6) {
        dtime *= 6;
        unit = HOUR;
    }
    else if (unit == HOURS12) {
        dtime *= 12;
        unit = HOUR;
    }

    if (unit == HOUR) {
	dtime += h;

        *hour = dtime % 24;
        dtime = dtime / 24;
        if (*hour < 0) {
            *hour += 24;
            dtime--;
        }
        unit = DAY;
    }

    /* this is the hard part */

    if (unit == DAY) {
	/* set m and day to Jan 0, and readjust dtime */
	jday = d + monthjday[m-1];
	if (leap(y) && m > 2) jday++;
        dtime += jday;

        while (dtime < 1) {
            y--;
	    dtime += 365 + leap(y);
        }

	/* one year chunks */
	while (dtime > 365 + leap(y)) {
	    dtime -= (365 + leap(y));
	    y++;
	}

	/* calculate the month and day */

	if (leap(y) && dtime == FEB29) {
	    m = 2;
	    d = 29;
	}
	else {
	    if (leap(y) && dtime > FEB29) dtime--;
	    for (i = 11; monthjday[i] >= dtime; --i);
	    m = i + 1;
	    d = dtime - monthjday[i];
	}
	*year = y;
	*month = m;
	*day = d;
	return 0;
   }
   fprintf(stderr,"add_time: undefined time unit %d\n", unit);
   return 1;
}


/*
 * verf_time:
 *
 * this routine returns the "verification" time
 * should have behavior similar to gribmap
 *
 */

int verf_time(unsigned char *pds, int *year, int *month, int *day, int *hour) {
    int tr, dtime, unit;

    *year = PDS_Year4(pds);
    *month = PDS_Month(pds);
    *day  = PDS_Day(pds);
    *hour = PDS_Hour(pds);

    /* find time increment */

    dtime = PDS_P1(pds);
    tr = PDS_TimeRange(pds);
    unit = PDS_ForecastTimeUnit(pds);

    if (tr == 10) dtime = PDS_P1(pds) * 256 + PDS_P2(pds);
    if (tr > 1 && tr < 6 ) dtime = PDS_P2(pds);
    if (tr == 6 || tr == 7) dtime = - PDS_P1(pds);

    if (dtime == 0) return 0;

    return add_time(year, month, day, hour, dtime, unit);
}

