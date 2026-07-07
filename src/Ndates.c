/** @file
 * @brief Print out a list of date codes
 * 
 *      for (datecode = INITIAL_DATECODE; datecode < INITIAL_DATECODE + DT1; datecode += DT2)
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 1/2019 | W. Ebisuzaki | Initial
 * 6/2021 | W. Ebisuzaki | direct write to inv_file, to avoid inv_out overflows
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 1/2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Get the maximum of two values. */
#define max(a,b) (a >= b) ? (a) : (b)

/** Format for date output .*/
char ndates_fmt[NAMELEN];

/** seq_file struct for inventory file */
extern struct seq_file inv_file;

/*
 * HEADER:100:ndates:setup:3:X=date0 Y=[=](date1|dt1) Z=dt2  for (date=X; date <[=] (date1|date0+dt1); date+=Z) print date
 */

/**
 * Creates a list of date codes.
 * 
 * This option has nothing to do with grib, but adding it to wgrib2 was trivial, and this option has been so helpful in scripting. Anyways, if a Swiss army knife can have a bottle opener, wgrib2 can have a date code routine. Anyways one less program to port is helpful. 
 * 
 * Note that until wgrib2 v3.0.3, the number of date codes was limited by the stdout buffer size. (See wgrib2 -config.) Wgrib2 v3.1.4 allows the optional equal sign option to include the terminating date code. 
 * 
 * The -ndate and -ndates options are initialization routines. They are run when all the options are initialized. Therefore they are run before reading the grib file. The -ndates_fmt option has to preceed the -ndates option to alter the output format. 
 * 
 * The wgrib2 command line can have multple -ndates options.
 * 
 * -ndates DATE DATE2 DT2     lt form
 * -ndates DATE DT1 DT2       lt form
 * -ndates DATE =DATE2 DT2    le form     wgrib2 v3.1.4
 * -ndates DATE =DT1 DT2      le form     wgrib2 v3.1.4
 * 
 * DATE, DATE2 = YYYY, YYYYMM, YYYYMMDD, YYYYMMDDHH, YYYYMMDDHHmm, YYYYMMDDHHmmss
 * DT1, DT2 = (integer)(unit)  unit=yr,mo,dy,hr,mn   (same as GrADS)
 * 
 * Note: DT must be positive and greater than zero.
 * 
 * If DT1 is given, then DATE2=DATE+DT1
 * 
 * lt form
 * 
 * C-meta:
 * @code{}
 * for (date=DATE; date < DATE2; date += DT2) print date;
 * @endcode
 * 
 * Fortran-meta
 * @code{.f}
 * date=DATE
 * while (date < DATE2)
 *      print date
 *      date=date+DT2
 * endwhile
 * @endcode
 * 
 * le form
 * 
 * C-meta:
 * @code{}
 * for (date=DATE; date<= DATE2; date += DT2) print date;
 * @endcode
 * 
 * Fortran-meta
 * @code{.f}
 * date=DATE
 * while (date <= DATE2)
 *      print date
 *      date=date+DT2
 * endwhile
 * @endcode
 * 
 * The only differences between the lt and le forms is the do-loop test clause.
 * 
 * DATE=integer of the form YYYY, YYYYMM, YYYYMMDD, YYYYMMDDHH, YYYYMMDDHHmm, YYYYMMDDHHmmss
 *      note: leading zeros cannot be replaced by spaces
 *      YYY = year (0000..9999), there were changes in the calendar (ex in 1752) which are ignored
 *      MM = month (01..12)
 *      DD = day (01..31)
 *      HH = hour (00..23)
 *      mm = minute (00..59)
 *      ss = second (00..59), there are no provisions for leap seconds
 * 
 * DT = (integer)(time unit)           "time unit" follows the GrADS convention
 *      integer > 0
 *      time unit = yr (year)
 *                  mo (month)
 *                  dy (day)
 *                  hr (hour)
 *                  mn (minute)
 *                  there is no seconds time unit defined by GrADS
 * 
 * One useful feature of -ndates is the format of the output date is appropriate for the input date 
 * code and DT2. The format has the most precision for both the input date code and DT2. 
 * 
 * Priority of the output format of the date codes (high to low)
 *  output date code: YYYYMMDDHHmmss      if input date code is YYYYMMDDHHmmss
 *  output date code: YYYYMMDDHHmm        if input date code is YYYYMMDDHHmm   or DT2 is in minutes (mn)
 *  output date code: YYYYMMDDHH          if input date code is YYYYMMDDHH     or DT2 is in hours (hr)
 *  output date code: YYYYMMDD            if input date code is YYYYMMDD       or DT2 is in days (dy)
 *  output date code: YYYYMM              if input date code is YYYYMM         or DT2 is in months (mo)
 *  output date code: YYYY                if input date code is YYYY           or DT2 is in years (yr)
 * 
 * ## Usage
 * ### lt form
 * -ndates DATE DATE2 DT2
 * -ndates DATE DT1 DT2
 * 
 * ### le form
 * -ndates DATE =DATE2 DT2
 * -ndates DATE =DT1 DT2
 * 
 * The output date code has the precision necessary for DATE and DT2 and is converted into a 
 * string. The date is printed out using the "ndates_fmt" format.  The default format is 
 * " %s".  (C format)
 *
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 1/2019
 */
int f_ndates(ARG3) {

    int year, month, day, hour, minute, second, i;
    int year_end, month_end, day_end, hour_end, minute_end, second_end;
    int dtime, dt_unit, strlen_arg1, strlen_arg2, format;
    char string[3], out[15], fmt_out[STRING_SIZE];
    int upto;	/* controls do-loop:   -1: upto   0: upto and equal */

    upto = -1;
    if (mode == -1) {

        /* get starting datecode */
        strlen_arg1 = strlen(arg1);
        for (i = 0; i < strlen_arg1; i++) {
            if (isdigit((unsigned char) arg1[i]) == 0) break;
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
        if (check_time(year,month,day,hour,minute,second))
            fatal_error("ndates: bad initial date","");

        /* get ending date */

        /* if first char is =, then change loop to upto and equal */
        if (arg2[0] == '=' ) {
            upto=0;	/* set to upto and including */
            arg2++;	    
        }

        strlen_arg2 = strlen(arg2);

        for (i = 0; i < strlen_arg2; i++) {
            if (isdigit((unsigned char) arg2[i]) == 0) break;
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
            dt_unit = string2time_unit(string);
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

        fmt_out[STRING_SIZE-1] = 0;

        while (cmp_time(year, month, day, hour, minute, second,
                year_end, month_end, day_end, hour_end, minute_end, second_end) <= upto) {

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

            snprintf(fmt_out, STRING_SIZE-1, ndates_fmt, out);
            if (fwrite_file(fmt_out, strlen(fmt_out), 1, &inv_file) != 1)
                fatal_error("ndates: write to inventory file");
            
            add_dt(&year, &month, &day, &hour, &minute, &second, dtime, dt_unit);
        }
    }
    return 0;
}

/*
 * HEADER:100:ndates_fmt:setup:1:X = C format for ndates option ex. 'date=%s'
 */

/**
 * Set the default format that the -ndates prints the date codes.
 * 
 * The default format is written in C as " %s". (The date code is converted into a string, 
 * and a blank and the string is printed out.) 
 * 
 * The -ndates_fmt option is an initialization option, so it runs prior to the processing 
 * of the grib file. The -ndates_fmt option needs to preceed the -ndates option. 
 * 
 * Default ndates format is " %s"
 * 
 * @code{.sh}
 * $ wgrib2 /dev/null -ndates 201802 1dy 6hr
 * 2018020100 2018020106 2018020112 2018020118$ 
 * @endcode
 * 
 * A list of files on one line
 * 
 * @code{.sh}
 * $ wgrib2 /dev/null -ndates_fmt " pgb%s" -ndates 201802 1dy 6hr
 * pgb2018020100 pgb2018020106 pgb2018020112 pgb2018020118$ 
 * @endcode
 * 
 * A list of files, one file per line
 * 
 * @code{.sh}
 * $ wgrib2 /dev/null -ndates_fmt "pgb%s\n" -ndates 201802 1dy 6hr
 * pgb2018020100
 * pgb2018020106
 * pgb2018020112
 * pgb2018020118
 * $
 * @endcode
 * 
 * Making a script to process a list of files
 * 
 * @code{.sh}
 * bash-4.1$ wgrib2 /dev/null -ndates_fmt "cp pgb%s ~/data\n" -ndates 201802 1dy 6hr >cmd
 * bash-4.1$ cat cmd
 * cp pgb2018020100 ~/data
 * cp pgb2018020106 ~/data
 * cp pgb2018020112 ~/data
 * cp pgb2018020118 ~/data
 * bash-4.1$
 * @endcode
 * 
 * The -ndates_fmt option understands three back-slash characters. 
 *      \n gets converted into a new-line character
 *      \t gets converted into tab character
 *      \\ gets converted into a back-slash character
 * 
 * Windows will be addressed later with respect to the end of line termination.
 * 
 * ## Usage
 * -ndates_fmt C_FORMAT
 * 
 * C_FORMAT is a C-language format which includes a %s to print the date code. The only back 
 * slash sequences allowed are \n, \t, and \\. Only one %s is supported.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 1/2019
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
