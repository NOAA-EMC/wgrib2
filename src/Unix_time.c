/** @file
 * @brief Print Unix timestamps. Requires ANSI C (C89).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/24/2009 | P. Romero | Initial - Required POSIX
 * 1/2022 | M. Schwarb, W. Ebisuzaki | version 2
 * @author Pablo Romero @date 3/24/2009
 */

/*
* This file is part of wgrib2 and is distributed under terms of the GNU General 
* Public License.  For details see, Free Software Foundation, Inc., 
* 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*
* (C) 2009  Pablo Romero
*	first version 24/03/2009
*		requirements: POSIX
*       v1.2 1/2022 Manfred Schwarb, W. Ebisuzaki
*   	v1.0 needed fix for bug in glibc, 
*		in code review found:
*		   restore of $TZ wasn't robust (not needed for linux and windows), 
*		   year 2038 bug (32-bit integer overflow)
*		   if program error, print -1 which is a valid unix time
*		   if forecast time is not in template (ex. radar), the
*		      verification time is printed as -1
*		use get_unixtime(..) from the netcdf_sup.c
*		if (program error or integer overflow) fatal_error
*		if (forecast_time is undefined) use verf_time = ref_time
*		change requirements from POSIX to C89
*		remove the conditional compile for POSIX code
*
* Requirements: 
*    ansi C (C89)
*
* Note: v1.0 and v1.2 have different responses to errors.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:unix_time:inv:0:print unix timestamp for rt & vt
 */

/**
 * Print Unix timestamps for reference and verification times.
 * 
 * Unix time is the number of seconds after 00:00 UTC 1/1/1970.
 * 
 * The -unix_time option writes unix time in the inventory. Unix time is defined by the number of seconds after 00:00 UTC Januay 1, 1970. Unix time is used by the many operating systems, and by NetCDF files. The -unix_time option depends on system/library functions to calculate the unix time. It requires ansi C compatibility and it is subject to overflows depnding on the definition of time_t. 
 * 
 * The unix time is stored in time_t integer-type variable. If time_t is a signed 32-bit integer, the integer will overflow in 1/19/2038 and is only valid between 12/13/1901 and 1/19/2038. One valid solution to the year 2038 overflow problem is to define time_t as an unsigned 32-bit integer. Then valid unix time are from 1/1/1970 to 2/7/2106. This solves the year 2038 overflow problem but makes time prior to 1970 invalid. The most common solution is to make time_t a signed 64-bit integer which limits the largest year to 2,147,485,547. That's enough time for dinosaurs to come and go. Here are the list of systems which may have problems with unix_time. 
 * 
 * - 32-bit linux systems, time_t is 32-bit signed int until kernel 5.6, API needs to be fixed
 * - 32-bit QNX: 32-bit QNX use unsigned 32-bit integer for time_t
 * - old versions of BSD varients use signed 32-bit int for time_t
 * - Windows: depends on the compiler
 * 
 * ## Usage
 * -unix_time
 * 
 * If the code detects a problem with the reference time, a fatal error occurs.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Pablo Romero @date 3/24/2009
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
