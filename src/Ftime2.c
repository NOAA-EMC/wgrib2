/** @file
 * @brief Implementation for -ftime2 and helper functions. The -ftime2 option is the
 * replacement for the -ftime1 option.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

// #define OLD_MODE

int ftime2(unsigned char **sec, char *inv_out, int mode);
int ftime2_tr(unsigned char **sec, char *inv_out, unsigned char *verf_time, int fcst_time, int fcst_unit, int n, 
              int mode, int *prt_missing);
static void print_ftime2 (int unit1, int value1, int unit2, int value2, int format, char *inv_out);

/**  Last message to process. */
extern unsigned int last_message;

/** Current mode for ftime. If zero, use default mode. If 1, do not simplify time codes. */
extern int ftime_mode;

/*
 * HEADER:440:ftime2:inv:0:timestamp -- will replace -ftime in the future TESTING
 */

/**
 * Prints the forecast time stamp such as the forecast hour, accumulation period, etc.
 * This option is used by many other inventory options. The -set_ftime2 option does the
 * inverse of the -ftime2 option. The -set_ftime2 option takes the forecast time stamp in
 * text format, and alters the grib metadata. 
 * 
 * The -ftime2 option is new and replaces the -ftime1 option. It fixes some problems in
 * describing climatologies of monthly means (averages of averages) or any nested statistical
 * processing operators. This option is fully recursive while the older -ftime1 attempted and 
 * failed to bypass the recursion. 
 * 
 * Along with the changes to -ftime2, the -set_time, -set_ave options are being replaced by
 * -set_ftime2.
 * 
 * ## Types of time stamps
 * The simplest forecast time stamps are for an analysis and a forecast at a point in time. 
 * Note that wgrib2 considers a "0 hour forecast" to be an "analysis". In theory, code table 
 * 4.3 could be used to determine whether the field is a "0 hour forecast" or "analysis". In 
 * practice, models like the GFS will use an analysis for the initial conditions, and the 
 * "0 hour forecast" will be the same as the analysis. So code table 4.3 is ignored. 
 *      Single analysis or single forecast
 *      - anl analysis
 *      - 12 hr fcst 12 hour forecast 
 * 
 * The next simplest time stamp involves an statistical operator and single time period from 
 * a forecast. Processing is done continously or at a high frequency. 
 *      Single forecast, examples of common statiistical operators (Code Table 4.10)
 *      - 174-180 hour acc fcst: accumulated forecast between 174-180 hours
 *      - 174-180 hour ave fcst: average forecast between 174-180 hours
 *      - 174-180 hour max fcst: maximum forecast between 174-180 hours
 *      - 174-180 hour min fcst: minimum forecast between 174-180 hours
 *      - 174-180 hour OPR fcst: operator of forecast between 174-180 hours, OPR from Code Table 4.10 
 * 
 * Grib allows for processed sets of data. For example, you have have analyses from May 1,
 *  .., May 31. You can create a grib message which is the May mean. You can then combine the May
 *  means from 1991..2020, and produce a 1991-2020 climatology. Another example is the take the forecasts
 *  from a single forecast run. You can take days 7..14, and create the 2nd week forecast. 
 *      Multiple analyses(forecasts), constant time between analyses (forecast start times)
 *      - 240@3 hour ave(anl),missing=0 average of 240 analyses with 3 hour interval between analyses, no missing analyses
 *      - 248@3 hour ave(6 hour fcst),missing=0 average of 240 6-hour fcsts with 3 hour interval of forecast start times 
 *      - 248@3 hour ave(0-3 hour ave fcst),missing=0, average of 240 "0-3 hour ave forecasts", with 3 hour interval between forecast start times 
 * 
 * The next type of forecast time stamp is used for processing forecast from a single forecast run. 
 * The "(xxx)++" notation means that xxx is incremented to the next forecast time. 
 *      Multiple forecasts from a single forecast run
 *          - 9@1 hour max(0-1 hour max fcst)++,missing=0: max(0-1 hour max fcst, 1-2 hour max fcst, .., 8-9 hour max fcst) 
 *          Note: this can be encoded as "0-9 hour max fcst"
 * 
 * LAF (Lagged Average Forecasts) which are forecasts with different forecast start times and the
 * same verification time. This is the easiest to produce ensemble forecast.
 *      Lagged Average Forecasts, old to new forecasts
 *      - LAF[..]++ reference time++, fcst_time-- (verification time does not change)
 *      Lagged Average Forecasts, new to old forecasts
 *      - LAF[..]-- reference time--, fcst_time++ (verification time does not change)
 * 
 * Grib allows fields that are produced by multiple statistical operators. This is how you 
 * can generate climatologies. 
 *      Climatology
 *      - 30@1 year ave(124@6 hour ave(anl)),missing=0: reference time is 1981050100
 *      "124@6 hour ave(anl)" is the average of (00Z May 1, 06Z May2, .., 18Z May 31), month average
 *      "30@1 year ave(X)" is the 30 year average of the X
 *      So a 1981-2010 May climatology has been precisely defined
 *      - 30@1 year ave(31@24 hour ave(anl)),missing=0: reference time is 1981050100
 *      This is the 1981-2020 climatology of the May 00Z analyses
 * 
 * ## Usage
 * -ftime2
 * 
 * Note: -ftime2 is called by many other options like -s and -S.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ wgrib2 grib2.polar -ftime2
 * 1.1:0:24 hour fcst
 * 1.2:0:24 hour fcst
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 12/2020
 */
int f_ftime2(ARG0) {
    if (mode < 0) return 0;
    ftime2(sec, inv_out, mode);
    return 0;
}

/*
Code Table 4.4: Indicator of unit of time range
Code figure   Meaning
  0	Minute
  1	Hour
  2	Day
  3	Month
  4	Year
  5	Decade (10 years)
  6	Normal (30 years)
  7	Century (100 years)
  8-9	Reserved
 10	3 hours
 11	6 hours
 12	12 hours
 13	Second
 14-191	Reserved
192-254	Reserved for local use
255	Missing
*/

/**
 * Prints the forecast time stamp.
 * 
 * @param sec Pointer to the GRIB section.
 * @param inv_out Pointer to the inventory output buffer.
 * @param mode Mode of operation.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2020
 */
int ftime2(unsigned char **sec, char *inv_out, int mode) {
    int fcst_time, fcst_unit;
    // int unit, value, n,i,code_4_11, code_4_10, fcst_time, fcst_unit;
    // int loc_n;
    unsigned char *verf_time;
    int prt_missing;
    if (mode == 99) fprintf(stdout,"ProdDefTemplateNo=%d\n", GB2_ProdDefTemplateNo(sec));

    /* if not a forecast .. no code 4.4 */
    if (code_table_4_4_location(sec) == NULL) {
        sprintf(inv_out,"anl");
        return 0;
    }

    /* forecast time */
    if ((fcst_unit = code_table_4_4(sec)) < 0) return -1;
    fcst_time = forecast_time_in_units(sec);

    verf_time = stat_proc_verf_time_location(sec);
    if (mode == 99) fprintf(stderr,"ftime2: is stat_proc_verf_time %d\n", verf_time != NULL);
    if (verf_time == NULL) {			/* point in time */
        if (fcst_time == 0) {
            sprintf(inv_out,"anl");
        }
        else {
    	    print_ftime2(fcst_unit, fcst_time, 0, 0, 1, inv_out);
            inv_out += strlen(inv_out);
            sprintf(inv_out," fcst");
            inv_out += strlen(inv_out);
        }
        return 0;
    }

    /* must be a time range */

    prt_missing = 0;
    ftime2_tr(sec, inv_out, verf_time, fcst_time, fcst_unit, 0, mode, &prt_missing);
    if (prt_missing) {
        inv_out += strlen(inv_out);
        if (is_missing(verf_time + 8, 4)) sprintf(inv_out,",missing=-1");
        else {
            sprintf(inv_out,",missing=%u", uint4(verf_time + 8));
        }
    }
    return 0;
}

/*
 * ftime2_tr: prints out the time range
 *   recursive
 */

/**
 * Recursively prints out the time range.
 * 
 * @param sec Pointer to the GRIB section.
 * @param inv_out Pointer to the inventory output buffer.
 * @param verf_time Pointer to the verification time.
 * @param fcst_time Forecast time.
 * @param fcst_unit Forecast time unit.
 * @param n Current level.
 * @param mode Mode of operation.
 * @param prt_missing Pointer to the missing flag.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2020
 */
int ftime2_tr(unsigned char **sec, char *inv_out, unsigned char *verf_time, int fcst_time, int fcst_unit, int n, 
	    int mode, int *prt_missing) {
    int n_max, i, code_4_10, code_4_11, code_4_4a, code_4_4b, timea, timeb;
    int tmp_value, tmp_unit, center, pdt;
    const char *left, *right;

    center = GB2_Center(sec);
    center = center == JMA2 ? JMA1 : center;	// treat JMA2 the same as JMA1

    pdt = GB2_ProdDefTemplateNo(sec);

    n_max = verf_time[7];
    if ((center == JMA1 || center == JMA2) && (pdt == 50008 || pdt == 50009 || pdt == 50011 || pdt == 50012)) {
        if (n_max != 1) fatal_error("JMA pdt %d n_max != 1 (%d)", pdt, n_max);
    }
    
    n_max = verf_time[7];
    if (mode == 99) fprintf(stderr," n=%d nmax=%d ", n, n_max);
    // if (n_max == 0) fatal_error("Statistical processing bad n=0","");
    if (n_max <= 0) {
        fprintf(stderr,"** ERROR bad grib message: Statistical Processing bad n=%d **\n", n_max);
        last_message |= DELAYED_FTIME_ERR;
        return 0;
    }
    if (n_max == 255) {
        fprintf(stderr,"** ERROR bad grib message: Statistical Processing n is undefined **\n");
        last_message |= DELAYED_FTIME_ERR;
        return 0;
    }

    code_4_10  = verf_time[12 + n*12];
    code_4_11  = verf_time[13 + n*12];
    code_4_4a  = verf_time[14 + n*12];
    timea = int4(verf_time + 15 + n*12);
    code_4_4b  = verf_time[19 + n*12];
    timeb = int4(verf_time + 20 + n*12);

    if (mode == 99) fprintf(stderr,">> 4.10=%d 4.11=%d 4.4a=%d timea=%d 4.4b=%d timeb=%d\n", code_4_10, code_4_11, code_4_4a,
        timea, code_4_4b, timeb);
     
        // code table 4.11
        //
        // 1:        (..)                       ref_time++
        // 2:        (..)++                     fcst_time++
        // 3:        LAF[..]++                  fcst_time--, ref_time++
        // 4:        LAF[..]--                  fcst_time++, ref_time--


    if (n == n_max - 1) {			// final level or inner loop (old code)
        if (mode == 99) fprintf(stderr,"code 4.11 %d timea %d timeb %d fcst_time %d\n", code_4_11, timea, timeb, fcst_time);

        if (code_4_11 == 0) {
            sprintf(inv_out,"Code Table 4.11=reserved");
            return 0;
        }

        /* 124@6 hour ave(anl) 120@6 hour ave(12 hour fcst) */
        /* start of forecast is increased, forecast length is the same */
        if (code_4_11 == 1 && timeb == 0) {
            inv_out += strlen(inv_out);
            print_ftime2(code_4_4a,0,code_4_4a, timea, 2, inv_out);
            inv_out += strlen(inv_out);
            if (fcst_time == 0) {
                sprintf(inv_out," %s anl", code_4_10_name(code_4_10, center));
            }
            else {
                sprintf(inv_out," %s(%d %s fcst)", code_4_10_name(code_4_10, center), fcst_time, time_range2a(fcst_unit));
            }
            inv_out += strlen(inv_out);
            return 0;
        }       
        if (code_4_11 == 1) {
            inv_out += strlen(inv_out);
            if (code_4_4a == code_4_4b) {
                sprintf(inv_out,"%d@%d %s %s", timea/timeb+1,timeb,time_range2a(code_4_4b), code_4_10_name(code_4_10, center));
            }
            else {
                sprintf(inv_out,"%d %ss@%d %s %s", timea, time_range2a(code_4_4a), timeb, time_range2a(code_4_4b), 
                        code_4_10_name(code_4_10, center));
            }
            inv_out += strlen(inv_out);
            if (fcst_time == 0) {
                sprintf(inv_out,"(anl)");
            }
            else {
                sprintf(inv_out,"(");
                inv_out += strlen(inv_out);
                print_ftime2(fcst_unit,fcst_time,code_4_4a, timea, 1, inv_out);
                inv_out += strlen(inv_out);
                sprintf(inv_out," fcst)");
            }
            *prt_missing = 1;
            return 0;
        }

        /* 1-5 hour ave fcst */
        if (code_4_11 == 2 && timeb == 0) {
            print_ftime2(fcst_unit,fcst_time,code_4_4a, timea, 2, inv_out);
            inv_out += strlen(inv_out);

            // NCEP TMIN and TMAX are depreated but still used
            // missing statistical operator .. don't print it .. remain compatible to ftime v1

            if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 0
                   && (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 4 || GB2_ParmNum(sec) == 5)
                   && code_4_10 == 255) {
                sprintf(inv_out," fcst");
            }
            else {
                sprintf(inv_out," %s fcst", code_4_10_name(code_4_10, center));
            }
            return 0;
        }

        /* 74-180 hour ave(fcst,dt=10 hour) */
        if (code_4_11 == 2) {
            *prt_missing = 1;
            print_ftime2(fcst_unit,fcst_time, code_4_4a, timea, 2, inv_out);
            inv_out += strlen(inv_out);
            tmp_unit = code_4_4b;
            tmp_value = timeb;
            normalize_time_range(&tmp_unit, &tmp_value);
            sprintf(inv_out," %s@(fcst,dt=%d %s)", code_4_10_name(code_4_10, center),tmp_value,time_range2a(tmp_unit));
            // sprintf(inv_out," %s@(fcst,dt=%d %s)", code_4_10_name(code_4_10, center),timeb,time_range2a(code_4_4b));
            return 0;
        }
        /* 0-12 hour ave(24-12 hour fcst) */
        if (code_4_11 == 3 && timeb == 0) {
            inv_out += strlen(inv_out);
            sprintf(inv_out,"0-%d %s %s(", timea, time_range2a(code_4_4a),code_4_10_name(code_4_10, center));
            inv_out += strlen(inv_out);
            print_ftime2(fcst_unit,fcst_time, code_4_4a, -timea, 3, inv_out);
            inv_out += strlen(inv_out);
            sprintf(inv_out," fcst)");
            inv_out += strlen(inv_out);

            return 0;
        }

        //  ensemble acc-3 valid 174 hour
        if (code_4_11 == 3) {
            sprintf(inv_out,"ensemble %s-3 valid %d %s", code_4_10_name(code_4_10, center), fcst_time, time_range2a(fcst_unit));
            return 0;
        }
	    
        //  ensemble acc-4 valid 174 hour
        if (code_4_11 == 4) {
            sprintf(inv_out,"ensemble %s-4 valid %d %s", code_4_10_name(code_4_10, center), fcst_time, time_range2a(fcst_unit));
            return 0;
        }
        if (code_4_11 == 5) {
#ifdef OLD_MODE
            sprintf(inv_out,"%d %s dt %d %s%d %s %s", fcst_time, time_range2a(fcst_unit),
            timea, time_range2a(fcst_unit), timea, time_range2a(fcst_unit), code_4_10_name(code_4_10, center));
#else
            print_ftime2(fcst_unit,fcst_time, code_4_4a, timea, 2, inv_out);
            inv_out += strlen(inv_out);
            sprintf(inv_out, " (subinterval) %s", code_4_10_name(code_4_10, center));

#endif
        }
        if (code_4_11 > 5) {
#ifdef OLD_MODE
            sprintf(inv_out,"Code Table 4.11=reserved");
#else
            sprintf(inv_out,"Code Table 4.11=%d", code_4_11);
#endif
        }
        return 0;
    }

    // multi-level definition

    if (timeb == 0) {
        fprintf(stderr,"\n*** FATAL ERROR ftime2: time increment is zero ***\n");
        last_message |= DELAYED_FTIME_ERR;  
        return 0;
    }
    if (code_4_4a == 255 || code_4_4b == 255) {
        fprintf(stderr,"\n*** FATAL ERROR ftime2: code table 4.4 is undefined ***\n");
        last_message |= DELAYED_FTIME_ERR;  
        return 0;
    }
    if (code_4_11 >= 1 && code_4_11 <= 5) {
        if (code_4_11 == 1) {
            left  = "(";
            right = ")";
        }
        else if (code_4_11 == 2) {
            left  = "(";
            right = ")++";
        }
        else if (code_4_11 == 3) {
            left  = " (";
            right = ")--";
        }
        else if (code_4_11 == 4) {
            left  = "(";
            right = ")++";
        }
        else if (code_4_11 == 5) {
            left  = " ?[";
            right = "]";
        }
        else {				// what else to do
            left  = "{?";	
            right = "}?";
        }
        if (code_4_4a == code_4_4b) {
            i = timea/timeb+1;
            simple_time_range(&code_4_4b, &timeb);
            if (code_4_11 == 4) sprintf(inv_out,"%d@-%d %s %s%s",i,timeb,time_range2a(code_4_4b), code_4_10_name(code_4_10, center),left);
            else sprintf(inv_out,"%d@%d %s %s%s",i,timeb,time_range2a(code_4_4b), code_4_10_name(code_4_10, center),left);
        }
        else {
            sprintf(inv_out,"%d %ss@%d %s %s%s", timea, time_range2a(code_4_4a), timeb, time_range2a(code_4_4b), 
                    code_4_10_name(code_4_10, center),left);
        }
        inv_out += strlen(inv_out);
        ftime2_tr(sec, inv_out, verf_time, fcst_time, fcst_unit, n+1, mode, prt_missing);
        inv_out += strlen(inv_out);
        sprintf(inv_out,"%s",right);
        *prt_missing = 1;
        return 0;
    }
    if (code_4_11 == 0) {
        sprintf(inv_out,"Code Table 4.11=reserved ?[");
        inv_out += strlen(inv_out);
        ftime2_tr(sec, inv_out, verf_time, fcst_time, fcst_unit, n+1, mode, prt_missing);
        inv_out += strlen(inv_out);
        sprintf(inv_out,"]");
        *prt_missing = 1;
    }
    else {
        sprintf(inv_out, "CodeTable 4.11=%d ?[", code_4_11);
        inv_out += strlen(inv_out);
        ftime2_tr(sec, inv_out, verf_time, fcst_time, fcst_unit, n+1, mode, prt_missing);
        inv_out += strlen(inv_out);
        sprintf(inv_out,"]");
        *prt_missing = 1;
    }
    return 0;
}

/**
 * Print formatted time range.
 * 
 * @param unit1 Time unit for the first time value.
 * @param value1 Value of the first time.
 * @param unit2 Time unit for the second time value.
 * @param value2 Value of the second time.
 * @param format Format of the time range.
 * If format == 1, print "N time_range" (ex. 12 hour, -1 day).
 * If format == 2, print "N-M time_range" or "N time_range1-(N time_range1+M time_range2)" 
 * (ex. 0-3 hour, 2 hour-(4 day+2 hour)).
 * If format == 3, print "N:M time_range" or "N time_range1:(N time_range1+M time_range2)"
 * (ex. 0:3 hour, 2 hour:(4 day+2 hour)).
 * @param inv_out Output buffer for the formatted time range.
 * 
 * @author Wesley Ebisuzaki @date 12/2020
 */
static void print_ftime2(int unit1, int value1, int unit2, int value2, int format, char *inv_out) {

    int dash_colon;

    normalize_time_range(&unit1, &value1);

    if (format == 1) {
        sprintf(inv_out,"%d %s",value1,time_range2a(unit1));
        return;
    }

    dash_colon = format == 2 ? '-' : ':';

    normalize_time_range(&unit2, &value2);

    if (value1 == 0) {
        unit1 = unit2;
    }
    else if (value2 == 0) {
        unit2 = unit1;
    }
    if (unit1 == unit2) {
        if ((ftime_mode & 1) == 0) {
            if (unit1 ==  13 && (value1 % 60 == 0) && (value2 % 60 == 0)) {		// seconds -> minutes
                value1 /= 60;
                value2 /= 60;
                unit1=0;								// hours
            }
            if (unit1 ==  0 && (value1 % 60 == 0) && (value2 % 60 == 0)) {		// minutes -> hours
                value1 /= 60;
                value2 /= 60;
                unit1=1;								// hours
            }
            if (unit1 ==  1 && (value1 % 24 == 0) && (value2 % 24 == 0)) {		// hours -> days
                value1 /= 24;
                value2 /= 24;
                unit1=2;								// days
            }
            if (unit1 ==  3 && (value1 % 12 == 0) && (value2 % 12 == 0)) {		// months -> years
                value1 /= 12;
                value2 /= 12;
                unit1=4;								// years
            }
        }
        sprintf(inv_out,"%d%c%d %s",value1,dash_colon,value1+value2,time_range2a(unit1));
    }
    else {
        sprintf(inv_out,"%d %s%c(%d %s+%d %s)",value1,time_range2a(unit1),dash_colon,
                value1,time_range2a(unit1),value2,time_range2a(unit2));
        if (unit2 == 255 || unit1 == 255) fprintf(stderr,"WARNING: missing CODE 4.11\n");
    }
    return;
}
