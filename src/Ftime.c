/** @file
 * @brief Forecast time stamp processing functions.
 * @author Public Domain: Wesley Ebisuzaki @date 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"
/*
 * Public Domain 2017: Wesley Ebisuzaki
 *
 * allow multiple versions of ftimeX, set_ftimeX to be available
 *
 * note: ftimeX, set_ftimeX should not (1) require initialization, (2) require cleanup
 * (3) require static variables.
 */

/** Default version of ftime. */
#define DEFAULT_FTIME	2

#ifndef DEFAULT_FTIME
int version_ftime = 1; /**< Current version of ftime. */
#else
int version_ftime = DEFAULT_FTIME;
#endif

/*
 * HEADER:440:ftime:inv:0:same as ftime2
 */

/**
 * Prints the forecast time stamp such as the forecast hour, accumulation period, etc.
 * This option is used by many other inventory options. The -set_ftime option does the
 * inverse of the -ftime option. The -set_ftime option takes the forecast time stamp in
 * text format, and alters the grib metadata. 
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
 * It was noted that the old ftime had problems with the more complicated time stamps and 
 * ftime2 was developed as the replacement. Currently (5/2021), you can use the old ftime 
 * by -ftime1 and the new ftime by -ftime2 . The default -ftime will use -ftime2. The -config 
 * option will tell which version is the default. 
 * 
 * ## Usage
 * -ftime
 * 
 * Note: -ftime is called by many other options like -s and -S.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2017
 */
int f_ftime(ARG0) {
    if (mode < 0) return 0;
    return f_ftime2(call_ARG0(inv_out,NULL));
    // if (version_ftime == 1) return f_ftime1(call_ARG0(inv_out,NULL));
    // if (version_ftime == 2) return f_ftime2(call_ARG0(inv_out,NULL));
    // return 1;
}

/*
 * HEADER:100:set_ftime:misc:1:same as set_ftime2
 */

/**
 * Same as set_ftime2. See documentation for the -set_ftime2 option.
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2017
 */
int f_set_ftime(ARG1) {
    if (mode < 0) return 0;
    return f_set_ftime2(call_ARG1(inv_out,NULL,arg1));
    // if (version_ftime == 1) return f_set_ftime1(call_ARG1(inv_out,NULL,arg1));
    // if (version_ftime == 2) return f_set_ftime2(call_ARG1(inv_out,NULL,arg1));
    // return 1;
}


// /*
//  * HEADER:440:set_version_ftime:setup:1:set version of ftime X=1 (deprecated), 2
//  */
// int f_set_version_ftime(ARG1) {
//    if (mode != -1) return 0;
//   version_ftime = atoi(arg1);
//    if (version_ftime < 1 || version_ftime > 2) fatal_error_i("set_version_ftime: %d not valid version", version_ftime);
//    return 0;
// }
