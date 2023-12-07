/*
 This file is part of wgrib2 and is distributed under terms of the GNU General 
 Public License.  For details see, Free Software Foundation, Inc., 
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 (C) 2009  Pablo Romero
	first version 24/03/2009
		requirements: POSIX
        v1.2 1/2022 Manfred Schwarb, W. Ebisuzaki
		v1.0 needed fix for bug in glibc, 
		in code review found:
		   restore of $TZ wasn't robust (not needed for linux and windows), 
		   year 2038 bug (32-bit integer overflow)
		   if program error, print -1 which is a valid unix time
		   if forecast time is not in template (ex. radar), the
		      verification time is printed as -1
		use get_unixtime(..) from the netcdf_sup.c
		if (program error or integer overflow) fatal_error
		if (forecast_time is undefined) use verf_time = ref_time
		change requirements from POSIX to C89
		remove the conditional compile for POSIX code

Requirements: 
    ansi C (C89)

Note: v1.0 and v1.2 have different responses to errors.

 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:unix_time:inv:0:print unix timestamp for rt & vt
 */

int f_unix_time(ARG0) {
    int year, month, day, hour, minute, second;
    double ref_time, verf_time;
    int ref_err_code, verf_err_code;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
	ref_time = get_unixtime(year, month, day, hour, minute, second, &ref_err_code);
	if (ref_err_code) fatal_error("unix_time: program error 1","");

	if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
	    verf_time = get_unixtime(year, month, day, hour, minute, second, &verf_err_code);
	    if (verf_err_code) fatal_error("unix_time, program error 2","");
	}
	else verf_time = ref_time; /* radar, satellite do not have fcst hours, use ref_time */

        sprintf(inv_out,"unix_rt=%.0lf:unix_vt=%.0lf", ref_time, verf_time);
    }
    return 0;
}
