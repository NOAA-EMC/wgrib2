/** @file
 * @brief Print out a date + offset.
 * @author Public Domain: Wesley Ebisuzaki @date 1/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Get the maximum of two values. */
#define max(a,b) (a >= b) ? (a) : (b)


/*
 * HEADER:100:ndate:setup:2:X=date Y=dt print date + dt
 */

/**
 * Print date + dt.
 * 
 * The NCEP utility, ndate, will add or subtract hours from a date code. Wgrib2 already has 
 * routines to do these calculations. So instead of porting the ndate fortran code and the 
 * NCEP libraries, how about a simple-to-write option that would allow wgrib2 to do the same 
 * calculation (and more)? In my case, installing wgrib2 is a probably a given. So adding 
 * -ndate saves me time. 
 * 
 * The ndate utility allows you to add or subtract a integer number of hours from a YYYYMMDDHH 
 * date code, The -ndate option allows you to add or subtract an integer number of minutes, 
 * hours, days, months or years from a date code. The output format has the precision necessary 
 * for the input date code and offset. Sure you could do all this with the gnu date program but 
 * the -ndate option is easier to use. 
 * 
 * The feature of the ndate utility that is not in wgrib2, is the ability to print the current 
 * UTC date code when there is no argument. Of course, you can do the same by "date -u +%Y%m%d%H". 
 * For users of the ndate utility, note that the order of arguments are reversed between the utility 
 * and wgrib2 option. 
 * 
 * The -ndate option is an odd option because it does its output in the initialization phase. In 
 * order to to trigger an "missing input file" error, you need to run wgrib2 on a valid file. For 
 * linux/unix systems, using the file /dev/null is a convenient, always present file. 
 * 
 * ## Usage
 * -ndate DATE OFFSET
 * 
 * DATE = YYYY, YYYYMM, YYYYMMDD, YYYYMMDDHH, YYYYMMDDHHmm, YYYYMMDDHHmmss
 * OFFSET=(integer)(yr|mo|dy|hr|mn)
 *      integer can be positive or negative
 *      yr = year  (0000..9999)
 *      mo = month (01..12)
 *      dy = day   (01..31)
 *      hr = hour  (00..23)
 *      mn = minute (not to be confused with month)  (00..59)
 * 
 * The output format has the precision to reflects the maximum precision of the date code and the 
 * offset.
 * 
 * Priority of the output format of the date code:
 *    output date code: YYYYMMDDHHmmss      if input date code is YYYYMMDDHHmmss
 *    output date code: YYYYMMDDHHmm        if input date code is YYYYMMDDHHmm   or OFFSET is in minutes (mn)
 *    output date code: YYYYMMDDHH          if input date code is YYYYMMDDHH     or OFFSET is in hours (hr)
 *    output date code: YYYYMMDD            if input date code is YYYYMMDD       or OFFSET is in days (dy)
 *    output date code: YYYYMM              if input date code is YYYYMM         or OFFSET is in months (mo)
 *    output date code: YYYY                if input date code is YYYY           or OFFSET is in years (yr)
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 1/2019
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

        if (check_time(year, month, day, hour, minute, second))
            fatal_error("ndate: bad initial date code","");

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
