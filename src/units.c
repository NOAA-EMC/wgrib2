/** @file
 * @brief Time range conversion functions.
 * @author Public Domain: Wesley Ebisuzaki @date 2005
 */
#include <stdio.h>
#include <string.h>
#include "grb2.h"  /* for center codes */
#include "wgrib2.h"

/** Ftime control. */
extern int ftime_mode;

/** Time range table. */
static struct tr_table_struct {
    const char *name;   /**< Name of the time range. */
    const int val;      /**< Value of the time range. */
} tr_table [] = {
	{ "min", 0 },
	{ "hour", 1 }, 
	{ "day", 2 },
	{ "month", 3 },
	{ "year", 4 },
	{ "decade", 5 },
	{ "normal", 6 },
	{ "century", 7 },
	{ "3-hours", 10 },
	{ "6-hours", 11 },
	{ "12-hours", 12 },
	{ "sec", 13 },
	{ "missing", 255 },
};

/**
 * Converts an ASCII string to a time range integer.
 * 
 * @param string Pointer to the input string.
 * 
 * @return Time range integer. Returns -1 on failure.
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
int a2time_range(const char *string) {
    int i;
    for (i = 0; i < sizeof (tr_table) / sizeof(struct tr_table_struct); i++) {
        if (strcmp(string,tr_table[i].name) == 0) return tr_table[i].val;
    }
    return -1;
}

/**
 * Converts a time range integer to an ASCII string.
 * 
 * @param tr Time range integer.
 * 
 * @return Pointer to the corresponding string. Returns "?" on failure.
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
const char *time_range2a(int tr) {
    int i;
    for (i = 0; i < sizeof (tr_table) / sizeof(struct tr_table_struct); i++) {
        if (tr_table[i].val == tr) return tr_table[i].name;
    }
    return "?";
}

/**
 * Converts units to single digit units (i.e. 3 hours -> hours).
 * 
 * @param tr Pointer to the time range integer.
 * @param val Pointer to the value integer.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
int normalize_time_range(int *tr, int *val) {
    switch(*tr) {
        case 10:		// 3 hours
            *tr = 1;
            *val *= 3;
            break;
        case 11:		// 6 hours
            *tr = 1;
            *val *= 6;
            break;
        case 12:		// 12 hours
            *tr = 1;
            *val *= 12;
            break;
    }
    return 0;
}

/**
 * Converts time range values to a simplified representation.
 * 
 * @param tr Pointer to the time range integer.
 * @param val Pointer to the value integer.
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
void simple_time_range(int *tr, int *val) {

    if (*tr == 10) {	// 3 hours
        *tr = 1;
        *val *= 3;
    }
    else if (*tr == 11) {	// 6 hours
        *tr = 1;
        *val *= 6;
    }
    else if (*tr == 12) {	// 12 hours
        *tr = 1;
        *val *= 12;
    }

    if ((ftime_mode & 1) == 0) {
        if (*tr == 13 && (*val % 60 == 0) && (*val != 0)) {		// seconds
            *val /= 60;
            *tr = 0;						// minutes
        }
        if (*tr == 0 && (*val % 60 == 0) && (*val != 0)) {		// minutes
            *val /= 60;
            *tr = 1;						// hours
        }
        if (*tr == 1 && (*val % 24 == 0) && (*val != 0)) {		// hours
            *val /= 24;
            *tr = 2;						// days
        }
    }
}

/*
 * a2code_4_10(const char *string)
 *  converts a string to code 4.10
 */

/**
 * Converts an ASCII string to corresponding value in Code Table 4.10 - Type of Statistical 
 * Processing.
 * 
 * @param string Pointer to the input string.
 * 
 * @return Corresponding Code Table 4.10 value. Returns -1 on failure.
 * 
 * @author Wesley Ebisuzaki @date 2005
 */
int a2code_4_10(const char *string) {
    int i;

    i = -1;
    if (strcmp(string,"ave") == 0) i = 0;
    else if (strcmp(string,"acc") == 0) i = 1;
    else if (strcmp(string,"max") == 0) i = 2;
    else if (strcmp(string,"min") == 0) i = 3;
    else if (strcmp(string,"last-first") == 0) i = 4;
    else if (strcmp(string,"RMS") == 0) i = 5;
    else if (strcmp(string,"StdDev") == 0) i = 6;
    else if (strcmp(string,"covar") == 0) i = 7;
    else if (strcmp(string,"first-last") == 0) i = 8;
    return i;
}

/**
 * Converts a Code Table 4.10 (Type of Statistical Processing) value to its
 * corresponding ASCII string representation.
 * 
 * @param code_4_10 Code Table 4.10 value.
 * @param center Center value (not currently used).
 * 
 * @return Corresponding string representation of the Code Table 4.10 value. Returns "???" on failure.
 * 
 * @author Wesley Ebisuzaki @date 2005
 * 
  * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2005 | W. Ebisuzaki | Initial
 * 11/2025 | M. Schwarb | Added center input parameter for future use (not currently used)
 */
const char *code_4_10_name(int code_4_10, int center) {
    const char *string;
    
    string = "???";
    switch(code_4_10) {
#include           "CodeTable_4.10.dat"
    }
    return string;
}


/*
 * a2anl_fcst(const char *string)
 *
 * returns 0 == anl
 *         1 == fcst
 *        -1    not above
 */

/**
 * Converts an ASCII string to a corresponding value in Code Table 1.4 - Type of Data.
 * 
 * Only supports "Analysis" (0) and "Forecast" (1).
 * 
 * @param string Pointer to the input string (must be "anl" or "fcst").
 *
 * @return
 * - 0 :: Analysis (anl)
 * - 1 :: Forecast (fcst)
 * - -1 :: Not valid
 * @author Wesley Ebisuzaki @date 2005
 */
int a2anl_fcst(const char *string) {
    int i;
    i = -1;
    if (strcmp(string,"anl") == 0) i = 0;
    else if (strcmp(string,"fcst") == 0) i = 1;
    return i;
}
