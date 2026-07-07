/** @file
 * @brief Routines for handling verification time.
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 9/2006  | W. Ebisuzaki | Initial
 * 1/2007  | W. Ebisuzaki | Check error code on verftime
 * 2/2008  | W. Ebisuzaki | For product definition template 8 (ave/acc/etc), return end of 
 *                          averaging interval -- like grads
 * 3/2008  | W. Ebisuzaki | Add verf time for ensemble products, add vt=
 * @author Public Domain: Wesley Ebisuzaki @date 9/2006
 */

/*
 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 Edition 2008.02.18

 Sergey Varlamov
 Kristian Nilssen
 Wesley Ebisuzaki
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Month name abbreviations. */
static const char *months = "janfebmaraprmayjunjulaugsepoctnovdec";

/*
 * HEADER:400:vt:inv:0:verf time = reference_time + forecast_time, -v2 for alt format
 */

/**
 * Prints the verification time (without minutes and seconds).
 *
 * The verification time = reference_time + forecast_time.
 * 
 * In conjunction with -v2 verbose mode, the format of the time will change to be GrADS
 * compatible. 
 * 
 * ## Usage
 * -vt
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
 * @author Wesley Ebisuzaki @date 9/2006
 */
int f_vt(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"vt=%4.4d%2.2d%2.2d%2.2d", year,month,day,hour);
            }
            else {
                sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
           sprintf(inv_out,"vt=?");
        }
    }
    return 0;
}

/*
 * HEADER:400:VT:inv:0:verf time = reference_time + forecast_time (YYYYMMDDHHMMSS)
 */

/**
 * Prints the verification time (YYYYMMDDHHMMSS).
 * 
 * The verification time = reference_time + forecast_time.
 * 
 * ## Usage
 * -VT
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
 * @author Wesley Ebisuzaki @date 9/2006
 */
int f_VT(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"vt=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year,month,day,hour,minute,second);
            }
            else {
                sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour,minute,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"vt=?");
        }
    }
    return 0;
}

/**
 * Returns the verification time: reference_time + forecast_time + statistical processing 
 * time (if any).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 9/2006  | W. Ebisuzaki | Initial
 * 1/2007  | W. Ebisuzaki | Return error code
 * 11/2007 | W. Ebisuzaki | Fixed code for non forecasts
 * 3/2008  | W. Ebisuzaki | Added code for ensemble processing
 * 4/2009  | W. Ebisuzaki | Test table 1.2 sign of reference time
 * 
 * @param sec Pointer to GRIB sections.
 * @param year Pointer to year.
 * @param month Pointer to month.
 * @param day Pointer to day.
 * @param hour Pointer to hour.
 * @param minute Pointer to minute.
 * @param second Pointer to second.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 9/2006
 */
int verftime(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second) {

    int units, i, j;
    int dtime;
    static int error_count = 0;
    static int warning_count = 0;

    i = code_table_4_0(sec);

    // if statistically processed field, verftime is in header
    j = stat_proc_verf_time(sec, year, month, day, hour, minute, second);
    if (j == 0) return 0;

    get_time(sec[1]+12, year, month, day, hour, minute, second);

    // some products have no forecast time

    if (code_table_4_4(sec) == -1) return 0;

    // check the significance of the refernce time
    i = code_table_1_2(sec);

    if (i == 2) {
        // rt=verifying time of forecast
        // unclear what it means for time averages/accumulations
        if (warning_count == 0) {
            fprintf(stderr,"Warning: rt == vt (CodeTable 1.2)\n");
            warning_count++;
        }
        return 0;
    }
    if (i == 3) {
        // rt = observing time 
        return 0;
    }

    if (i != 0 && i != 1) {
        if (error_count == 0) {
            fprintf(stderr,"verifying time: Table 1.2=%d not supported "
                " using RT=analysis/start of forecast\n", i);
            error_count++;
        }
    }

    units = code_table_4_4(sec);
    dtime = forecast_time_in_units(sec);
    if (dtime >= 0) 
        return add_time(year, month, day, hour, minute, second, (unsigned int) dtime, units);
    return sub_dt(year, month, day, hour, minute, second, (unsigned int) (unsigned int) -dtime, units);
}

/*
 * HEADER:400:start_ft:inv:0:verf time = reference_time + forecast_time (YYYYMMDDHH) : no stat. proc time
 */

/**
 * Prints the verification time (without minutes and seconds) at the start of the forecast 
 * period (no statistical processing time).
 * 
 * Suppose you make a forecast starting from initial conditions of 00Z January 1 for the 
 * average conditions from the month of February. In this forecast, there are three time 
 * stamps. 
 * 
 * Name | Date | YYYYMMDDHH option | (HH)Z(DD)(MON)YYYY option | With Minutes and Seconds
 * -----|------|-------------------|--------------------------|-------------------------
 * Reference Time | 00Z Jan 1 | -t | -v2 -t | -T
 * Start of Forecast Period | 00Z Feb 1 | -start_ft | -v2 -start_ft | -start_FT
 * End of Forecast Period | 00Z Mar 1 | -end_ft | -v2 -end_ft | -end_FT
 *
 * For common times,
 * 
 * analysis:                      reference time == start_ft == end_ft
 * 6 hour forecast:               reference time + 6 == start_ft == end_ft
 * 30 year monthly climatology:   reference time = start_ft = start of 1st month used to make climatology
 *                                end_ft = end of the last month used to make the climatology
 * 
 * ## Usage
 * -start_ft
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
 * @author Wesley Ebisuzaki @date 9/2006
 */
int f_start_ft(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (start_ft(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"start_ft=%4.4d%2.2d%2.2d%2.2d", year,month,day,hour);
            }
            else {
               sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"start_ft=?");
       }
    }
    return 0;
}

/*
 * HEADER:400:start_FT:inv:0:verf time = reference_time + forecast_time (YYYYMMDDHHMMSS) - no stat. proc time
 */

/**
 * Prints the verification time (YYYYMMDDHHMMSS) at the start of the forecast period (no 
 * statistical processing time).
 * 
 * Suppose you make a forecast starting from initial conditions of 00Z January 1 for the 
 * average conditions from the month of February. In this forecast, there are three time 
 * stamps. 
 * 
 * Name | Date | YYYYMMDDHH option | (HH)Z(DD)(MON)YYYY option | With Minutes and Seconds
 * -----|------|-------------------|--------------------------|-------------------------
 * Reference Time | 00Z Jan 1 | -t | -v2 -t | -T
 * Start of Forecast Period | 00Z Feb 1 | -start_ft | -v2 -start_ft | -start_FT
 * End of Forecast Period | 00Z Mar 1 | -end_ft | -v2 -end_ft | -end_FT
 *
 * For common times,
 * 
 * analysis:                      reference time == start_ft == end_ft
 * 6 hour forecast:               reference time + 6 == start_ft == end_ft
 * 30 year monthly climatology:   reference time = start_ft = start of 1st month used to make climatology
 *                                end_ft = end of the last month used to make the climatology
 * 
 * ## Usage
 * -start_FT
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
 * @author Wesley Ebisuzaki @date 9/2006
 */
int f_start_FT(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (start_ft(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"start_FT=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year,month,day,hour,minute,second);
            }
            else {
                sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour,minute,day,months[month*3-3],
                        months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"start_FT=?");
        }
    }
    return 0;
}

/*
 * HEADER:400:end_ft:inv:0:verf time = reference_time + forecast_time + stat. proc time (YYYYMMDDHH) (same as -vt)
 */

/**
 * Prints the verification time (without minutes and seconds) at the end of the forecast 
 * period (reference_time + forecast_time + stat. proc time).
 * 
 * Suppose you make a forecast starting from initial conditions of 00Z January 1 for the 
 * average conditions from the month of February. In this forecast, there are three time 
 * stamps. 
 * 
 * Name | Date | YYYYMMDDHH option | (HH)Z(DD)(MON)YYYY option | With Minutes and Seconds
 * -----|------|-------------------|--------------------------|-------------------------
 * Reference Time | 00Z Jan 1 | -t | -v2 -t | -T
 * Start of Forecast Period | 00Z Feb 1 | -start_ft | -v2 -start_ft | -start_FT
 * End of Forecast Period | 00Z Mar 1 | -end_ft | -v2 -end_ft | -end_FT
 *
 * For common times,
 * 
 * analysis:                      reference time == start_ft == end_ft
 * 6 hour forecast:               reference time + 6 == start_ft == end_ft
 * 30 year monthly climatology:   reference time = start_ft = start of 1st month used to make climatology
 *                                end_ft = end of the last month used to make the climatology
 * 
 * ## Usage
 * -end_ft
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
 * @author Wesley Ebisuzaki @date 9/2006
 */
int f_end_ft(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"end_ft=%4.4d%2.2d%2.2d%2.2d", year,month,day,hour);
            }
            else {
                sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour,day,months[month*3-3],
                        months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"end_ft=?");
        }
    }
    return 0;
}

/*
 * HEADER:400:end_FT:inv:0:verf time = reference_time + forecast_time + stat. proc time (YYYYMMDDHHMMSS) (same as -VT)
 */

/**
 * Prints the verification time (YYYYMMDDHHMMSS) at the end of the forecast period 
 * (reference_time + forecast_time + stat. proc time).
 * 
 * Suppose you make a forecast starting from initial conditions of 00Z January 1 for the 
 * average conditions from the month of February. In this forecast, there are three time 
 * stamps. 
 * 
 * Name | Date | YYYYMMDDHH option | (HH)Z(DD)(MON)YYYY option | With Minutes and Seconds
 * -----|------|-------------------|--------------------------|-------------------------
 * Reference Time | 00Z Jan 1 | -t | -v2 -t | -T
 * Start of Forecast Period | 00Z Feb 1 | -start_ft | -v2 -start_ft | -start_FT
 * End of Forecast Period | 00Z Mar 1 | -end_ft | -v2 -end_ft | -end_FT
 *
 * For common times,
 * 
 * analysis:                      reference time == start_ft == end_ft
 * 6 hour forecast:               reference time + 6 == start_ft == end_ft
 * 30 year monthly climatology:   reference time = start_ft = start of 1st month used to make climatology
 *                                end_ft = end of the last month used to make the climatology
 * 
 * ## Usage
 * -end_FT
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
 * @author Wesley Ebisuzaki @date 9/2006
 */
int f_end_FT(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"end_FT=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year,month,day,hour,minute,second);
            }
            else {
                sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour,minute,day,months[month*3-3],
                        months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"end_FT=?");
        }
    }
    return 0;
}

/**
 * Get the verification time at the start of the forecast period.
 * 
 * @param sec Pointer to GRIB sections.
 * @param year Pointer to the year of the verification time.
 * @param month Pointer to the month of the verification time.
 * @param day Pointer to the day of the verification time.
 * @param hour Pointer to the hour of the verification time.
 * @param minute Pointer to the minute of the verification time.
 * @param second Pointer to the second of the verification time.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 9/2006
 */
int start_ft(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second) {

    int units, i;
    int dtime;
    static int error_count = 0;
    static int warning_count = 0;

    i = code_table_4_0(sec);

    // get reference time
    get_time(sec[1]+12, year, month, day, hour, minute, second);
    // some products have no forecast time .. done

    if ( (units = code_table_4_4(sec)) == -1) return 0;

    // check the significance of the refernce time
    i = code_table_1_2(sec);

    if (i == 2) {
        // rt=verifying time of forecast
        // unclear what it means for time averages/accumulations
        if (warning_count == 0) {
            fprintf(stderr,"Warning: rt == vt (CodeTable 1.2)\n");
            warning_count++;
        }
        return 0;
    }

    if (i != 0 && i != 1) {
        if (error_count == 0) {
            fprintf(stderr,"verifyingtime: Table 1.2=%d not supported "
                " using RT=analysis/start of forecast\n", i);
            error_count++;
        }
    }

    dtime = forecast_time_in_units(sec);
    add_time(year, month, day, hour, minute, second, dtime, units);
    return 0;
}

/**
 * Get the reference time.
 * 
 * @param sec Pointer to GRIB sections.
 * @param year Pointer to the year of the reference time.
 * @param month Pointer to the month of the reference time.
 * @param day Pointer to the day of the reference time.
 * @param hour Pointer to the hour of the reference time.
 * @param minute Pointer to the minute of the reference time.
 * @param second Pointer to the second of the reference time.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 9/2006
 */
int reftime(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second) {
    unsigned char *p;

    p = sec[1];
    *year = (p[12] << 8) | p[13];
    *month = p[14];
    *day = p[15];
    *hour = p[16];
    *minute = p[17];
    *second = p[18];

    return 0;
}

/**
 * Get the reference time as a full_date struct.
 * 
 * @param sec Pointer to GRIB sections.
 * @param date Pointer to the full_date struct to populate.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 9/2006
 */
int Ref_time(unsigned char **sec, struct full_date *date) {
    return Get_time(sec[1]+12, date);
}

/**
 * Get the verification time as a full_date struct.
 * 
 * @param sec Pointer to GRIB sections.
 * @param date Pointer to the full_date struct to populate.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 9/2006
 */
int Verf_time(unsigned char **sec, struct full_date *date) {
    return verftime(sec, &date->year, &date->month, &date->day, &date->hour, &date->minute, &date->second);
}

