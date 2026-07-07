/** @file
 * @brief Change the reference date of a GRIB message.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2008 | W. Ebisuzaki | Initial
 * 3/2016 | W. Ebisuzaki | allow +(dt) and -(dt)
 * @author Public Domain: Wesley Ebisuzaki @date 3/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:set_date:misc:1:changes date code, X=(+|-)N(mn|hr|dy|mo|yr), YYYYMMDDHHmmSS, u(UNIX TIME)
 */

/**
 * Changes the reference date of the in-memory grib (sub-)message.
 * 
 *  You can write out the message with the new date by the -grib option. If you use -grib_out 
 * option to write out the changed grib message, you will write the grib message with the 
 * compression method and precision that is in effect at the time. This is slower and may lose 
 * precision. Of course if the in-memory grid values have been modified, you have to use the 
 * -grib_out. 
 * 
 * The option -set_date X sets the reference time to "date code" X, where X is YYYY, YYYYMM, 
 * YYYYMMDD, YYYYMMDDHH, YYYYMMDDHHmm or YYYYMMDDHHmmSS. (YYYY=year, MM=month, DD=day, HH=hour, 
 * mm=minute, SS=second) If MM/DD/HH/mm/SS is missing, the original MM/DD/mm/SS is unaltered. 
 * 
 * You can set the reference time to a unix time (number of seconds after the start of January 1, 
 * 1970 by the argument "u(signed integer)". This is convenient for converting netcdf files to 
 * grib because netcdf files use unix time. 
 * 
 * You can change the reference time by an offset. To change the reference time by an offset, 
 * the argument has to start with either a negative sign or positive sign, followed by an integer 
 * and ending with a units (yr, mo, dy, hr, or mn). Note: minutes abreviation "mn" is easily 
 * confused as the abreviation for month and was added with wgrib2 v2.0.8. 
 * 
 * ## Usage
 * -set_date X            X = reference time, usually starting date
 * 
 * X=YYYY, YYYYMM, YYYYMMDD, YYYYMMDDHH, YYYYMMDDHHmm or YYYYMMDDHHmmSS, where YYYY=year, MM=month, 
 * DD=day, HH=hour, mm=minute, SS=second, if MM/DD/HH/mm/SS is missing, the original MM/DD/mm/SS is 
 * unaltered
 *
 * -set_date uN           N is a signed integer containing the number of seconds after the start of 
 * January 1, 1970.  N follows the local OS convention of unix time such as ignoring leap seconds. 
 * The limitation on N is OS dependent.
 * 
 * Newer linux: N is signed 64 bit integer. However N is limited by the wgrib2 implementation which 
 * stores N in a double precision variable.
 * 
 * Older linux: N is a signed 32 bit integer (problem in 2037)
 * 
 * Other OS:  varies, one OS uses N as a unsigned-32 bit integer.
 * 
 * -set_date -N(units)    negative offset
 * 
 * -set_date +N(units)    positive offset
 * 
 * N = unsigned integer
 * 
 * units = mn, hr, dy, mo or yr  (GrADS time units for minutes, hour, day, month and year)
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set_date(ARG1) {

    int year, month, day, hour, minute, second, i, j, units, n;
    double unix_time;
    int code_4_11,idx;
    int dtime, dt_unit;
    struct tm *tmp_tm;
    time_t unix_time_code;
    char string[10];

    if (mode < 0) return 0;

    reftime(sec, &year, &month, &day, &hour, &minute, &second);

    if (arg1[0] == '+' || arg1[0] == '-') {
        i = sscanf(arg1+1, "%d%2s", &dtime, string);
        if (i != 2 || dtime < 0) fatal_error("set_date: delta-time: (+|-)(int)(mn|hr|dy|mo|yr)","");
        dt_unit = string2time_unit(string);
        if (dt_unit == -1) fatal_error("set_date: unsupported time unit %s", string);
        if (arg1[0] == '+')
            i = add_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
        else
            i = sub_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
    }
    else if (arg1[0] == 'u') {
        /* reading unix time into double precision rather than time_t  or long long */
        /* long long is C99 and double precision is good enough */
        i = sscanf(arg1+1, "%lf", &unix_time);
        if (i == 0) fatal_error("set_date: u(unix_time)","");

        /* convert unix time to standard time, and set date */
        /* this is C89 code, not thread safe */
        unix_time_code = (time_t) unix_time;
        tmp_tm = gmtime(&unix_time_code);
        if (tmp_tm == NULL) fatal_error("set_date: u(unix_time) failed","");

        year = tmp_tm->tm_year + 1900;
        month = tmp_tm->tm_mon + 1;
        day = tmp_tm->tm_mday;
        hour = tmp_tm->tm_hour;
        minute = tmp_tm->tm_min;
        second = tmp_tm->tm_sec;
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
