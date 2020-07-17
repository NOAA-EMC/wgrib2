#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 This file is part of wgrib2 and is distributed under terms of the GNU General 
 Public License.  For details see, Free Software Foundation, Inc., 
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 (C) 2009  Pablo Romero
	first version 24/03/2009

 */

#ifndef DISABLE_TIMEZONE
time_t my_timegm(struct tm *tm);


/*
 * HEADER:100:unix_time:inv:0:print unix timestamp for rt & vt
 */

int f_unix_time(ARG0) {
    time_t rtx, vtx;
    struct tm timeinfo;
    int year, month, day, hour, minute, second;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
	timeinfo.tm_year = year - 1900;
	timeinfo.tm_mon = month- 1;
	timeinfo.tm_mday = day;
	timeinfo.tm_hour = hour;
	timeinfo.tm_min = minute;
	timeinfo.tm_sec = second;
	rtx=my_timegm(&timeinfo);

	if (verftime(sec, &(timeinfo.tm_year), &(timeinfo.tm_mon), &(timeinfo.tm_mday), &(timeinfo.tm_hour), 
		&(timeinfo.tm_min), &(timeinfo.tm_sec)) == 0) {
	    timeinfo.tm_year -= 1900;
	    timeinfo.tm_mon -= 1;
	    vtx=my_timegm(&timeinfo);
	}
	else vtx=-1;
	sprintf(inv_out,"unix_rt=%d:unix_vt=%d",(int) rtx, (int) vtx);
    }
    return 0;
}

/*
 * this can be used instead of timegm, for portability
 * since timegm apparently might not be supported on all platforms
 * my_timegm
 * make a GMT timestamp from a GMT tm struct
 */

time_t my_timegm(struct tm *tm)
{
	time_t ret;
	char *tz;

	tz = getenv("TZ");
	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	if (tz)
	   setenv("TZ", tz, 1);
	else
	   unsetenv("TZ");
	tzset();
	return ret;
}

#else

int f_unix_time(ARG0) {
   if (mode == -1) {fprintf(stderr,"unix_time was not installed\n"); return 1;}
   return 1;
}

#endif
