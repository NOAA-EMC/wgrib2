/** @file
 * @brief Some routines for probability forecasts.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 11/2008 | W. Ebisuzaki | Initial
 * 12/2008 | W. Ebisuzaki | Fixed return code
 * 1/2009 | W. Ebisuzaki | NCEP bug fix
 * 3/2018 | W. Ebisuzaki | Move NCEP bug fix to Fix_ncep_2.c, more info for -v
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 11/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Lower limit of the probability range. */
#define LOWER_LIMIT scaled2flt(INT1(p[1]), int4(p+2) )

/** Upper limit of the probability range. */
#define UPPER_LIMIT scaled2flt(INT1(p[6]), int4(p+7) )

/*
 * HEADER:100:prob:inv:0:probability information
 */

/**
 * Prints the probability information.
 * 
 * The TMP (temperature) field usually contains the temperature in Kelvin. However, with the 
 * right options, the TMP field could contain the probability of the temperature being 
 * - below a specified value
 * - above a specified value
 * - between two limits (lower limit <= V < upper limit).
 * 
 * The -prob option prints out the probability field, if any. Normally, -prob is invoked by 
 * other inventory functions like -misc and -s. 
 * 
 * ## Usage
 * -prob
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 sref.t03z.pgrb243.prob.grib2 -prob
 * 1:0:prob <273
 * 2:1378:prob <273
 * 3:2756:prob >500
 * 4:6309:prob >1000
 * 5:8800:prob >2000
 * 6:9801:prob >3000
 * 7:10262:prob >4000
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 11/2008
 */
int f_prob(ARG0) {
    unsigned char *p;

    if (mode == -1) {
        return 0;
    }
    else if (mode >= 0) {
        p = code_table_4_9_location(sec);
        if (p == NULL) return 0;
        // sprintf(inv_out,"probtype=%d ",*p);
        switch (*p) {

            case 0: sprintf(inv_out,"prob <%g", LOWER_LIMIT); break;
            case 1: sprintf(inv_out,"prob >%g", UPPER_LIMIT); break;
	
            case 2: if (LOWER_LIMIT == UPPER_LIMIT) {
                sprintf(inv_out,"prob =%g", LOWER_LIMIT); break;
                }
                sprintf(inv_out,"prob >=%g <%g", LOWER_LIMIT, UPPER_LIMIT); break;
            case 3: sprintf(inv_out,"prob >%g", LOWER_LIMIT);
                break;
            case 4: sprintf(inv_out,"prob <%g", UPPER_LIMIT);
                break;
            case 5: sprintf(inv_out,"prob =%g", LOWER_LIMIT);
                break;
            case 6: sprintf(inv_out, "prob above normal");
                break;
            case 7: sprintf(inv_out, "prob near normal");
                break;
            case 8: sprintf(inv_out, "prob below normal");
                break;
            default: sprintf(inv_out, "prob ?? (code table 4.9=%d)", *p);
                break;
        }
        if (mode == 99) {
            inv_out += strlen(inv_out);
            sprintf(inv_out, " LOWER LIMIT scale=%d, val= 0x%.2x 0x%.2x 0x%.2x 0x%.2x",
            INT1(p[1]), p[2], p[3], p[4], p[5]);
        }
        inv_out += strlen(inv_out);
        sprintf(inv_out,":prob fcst %u/%u", p[-2], p[-1]);
    }
    return 0;
}

