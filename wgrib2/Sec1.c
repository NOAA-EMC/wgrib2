/** @file
 * @brief Functions dealing with GRIB section 1 (Identification Section).
 * @author Public Domain: Wesley Ebisuzaki @date 12/2006
 */

#include <stdio.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Abbreviated month names. */
static const char *months = "janfebmaraprmayjunjulaugsepoctnovdec";

/** Warning flag for non-zero minutes/seconds in reference time. */
extern int warn_nonzero_min_sec;

/*
 * HEADER:100:t:inv:0:reference time YYYYMMDDHH, -v2 for alt format
 */

/**
 * Prints the reference time in the format YYYYMMDDHH.
 * 
 * In conjuntion with -v2 verbose mode, the format of the time will change to be GrADS compatible.
 * 
 * ## Usage
 * -t
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_t(ARG0) {

    int year, month, day, hour, minute, second;
    const char *m;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        if (warn_nonzero_min_sec && (minute != 0 || second != 0)) {
            fprintf(stderr,"** Warning: reference time includes non-zero minutes/seconds **\n");
            warn_nonzero_min_sec = 0;
        }
        if (mode != 2) {
            sprintf(inv_out,"d=%4.4d%2.2d%2.2d%2.2d", year, month, day, hour);
        }
        else {
            m = months + (month-1)*3;
            sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour, day, m[0],m[1],m[2],year);
        }
    }
    return 0;
}

/*
 * HEADER:101:T:inv:0:reference time YYYYMMDDHHMMSS
 */

/**
 * Prints the reference time in the format YYYYMMDDHHMMSS.
 * 
 * In conjuntion with -v2 verbose mode, the format of the time will change to be GrADS compatible.
 * 
 * ## Usage
 * -T
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_T(ARG0) {
    int year, month, day, hour, minute, second;
    const char *m;
    if (mode == -1) warn_nonzero_min_sec = 0;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        if (mode != 2) {
            sprintf(inv_out,"D=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year, month,day,hour,minute,second);
        }
        else {
            m = months + (month-1)*3;
            sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour, minute, day, m[0],m[1],m[2],year);
        }
    }
    return 0;
}

/*
 * HEADER:103:YY:inv:0:reference time YYYY
 */

/**
 * Prints the year of the reference time.
 * 
 * ## Usage
 * -YY
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_YY(ARG0) {
    int year, month, day, hour, minute, second;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        sprintf(inv_out, mode == 0 ? "YY=%4.4d" : "%4.4d", year);
    }
    return 0;
}

/*
 * HEADER:105:MM:inv:0:reference time MM
 */

/**
 * Prints the month of the reference time.
 * 
 * ## Usage
 * -MM
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_MM(ARG0) {
    int year, month, day, hour, minute, second;
    if (mode >= 0) {
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        sprintf(inv_out,"MM=%2.2d",month);
    }
    return 0;
} 

/*
 * HEADER:110:RT:inv:0:type of reference Time
 */

/**
 * Prints the type (signficance) of the reference time (Grib Table 1.2)
 * 
 * The reference time is usually the analysis time or the time of the start of the forecast 
 * (forecast time=0). However, the grib standard also allows the reference time to be verifying 
 * time of the forecast or the observation time as indicated by Grib Table 1.2. While legal 
 * grib, you should think twice before setting the reference time to the verifying time of the 
 * forecast. The -RT option prints the type (signficance) of the reference time (Grib Table 1.2). 
 * 
 * ## Usage
 * -RT
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_RT(ARG0) {
    if (mode >= 0) {
        switch (sec[1][11]) {
            case 0: sprintf(inv_out,"RT=analysis");
                break;
            case 1: sprintf(inv_out,"RT=Start of fcst");
                break;
            case 2: sprintf(inv_out,"RT=Verf time of fcst");
                break;
            case 3: sprintf(inv_out,"RT=Obs time");
                break;
            default:
                break;
        }
    }
    return 0;
}

/*
 * HEADER:110:center:inv:0:center 
 */

/**
 * Prints out the center (ex. NCEP, ECMWF) that created the grib file. If your center is printed 
 * as a number rather than a name, feel welcome to send an email requesting an update to the 
 * tables. 
 * 
 * ## Usage
 * -center
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_center(ARG0) {
    int ctr;
    char tmp[20];
    const char *string;
 
    if (mode >= 0) {
        ctr = GB2_Center(sec);
        switch (ctr) {
#include "code_table0.dat"
        }
        if (mode == 0) {
            sprintf(inv_out,"center=%d", ctr);
        }
        else {
            sprintf(inv_out,"center=%s", string);
        }
    }
    return 0;
}

/*
 * HEADER:110:subcenter:inv:0:subcenter 
 */

/**
 * Prints out the subcenter. For example, within NCEP, the subcenters include NCO, EMC, HPC, 
 * MPC, CPC, etc. 
 * 
 * ## Usage
 * -subcenter
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_subcenter(ARG0) {
    int ctr, subctr, ctrsubctr;
    const char *string;
    
    if (mode >= 0) {
        ctr = GB2_Center(sec);
        subctr = GB2_Subcenter(sec);
	      string = NULL;
	      if (ctr == 7) {
          switch (subctr) {
#include "CommonCodeTable_12.dat"
           }
        }
        if (ctr == (USAF)) {
            switch (subctr) {
#include "gribtables/usaf/usaf_tableC2.dat"
            }
        }
        if (subctr > 0) {  /* a lot of messages have no sub-centre declared */
            ctrsubctr = (ctr<<16)+subctr;
            switch (ctrsubctr) {
#include "CommonCodeTable_12.dat"
            }
        }
        if (mode == 0 || string == NULL) {
            sprintf(inv_out,"subcenter=%d", subctr);
        }
        else {
            sprintf(inv_out,"subcenter=%s", string);
        }
    }
    return 0;
}
