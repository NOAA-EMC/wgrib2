/** @file
 * @brief Implements 2nd NCEP bug fix. NCEP operations has probability for less than 
 * negative value. Uses 2's complement to store integer rather than signed number.
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 11/2008 | W. Ebisuzaki | Initial
 * 12/2008 | W. Ebisuzaki | fixed return code
 * 1/2008 | W. Ebisuzaki | ncep bug fix
 * 3/2018 | W. Ebisuzaki | move ncep bug fix to Fix_ncep_2.c
 * 08/2011 | A. Stahl | Removed UPPER_LIMIT, LOWER_LIMIT definitions (not used)
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 11/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Flag to indicate if NCEP fix 2 is active. */
extern int fix_ncep_2_flag;

/*
 * HEADER:100:fix_ncep_2:setup:0:ncep bug fix 2, probability observation < -ve number
 */

/**
 * "Set" option that fixes negative probability.
 *
 * NCEP operations has probability for less than negative value. NCEP operations knows 
 * of the problem but can't fix it because of downstream software.
 * 
 * ## Usage
 * -fix_ncep_2
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 *
 * @author Wesley Ebisuzaki @date 11/2008
 */
int f_fix_ncep_2(ARG0) {
    fix_ncep_2_flag = 1;
    return 0;
}

/**
 * Implements NCEP negative probability bug fix. Uses 2's complement to store integer 
 * rather than signed number.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return 0 for success.
 * 
 * @author Wesley Ebisuzaki @date 11/2008
 */
int fix_ncep_2(unsigned char **sec) {
    unsigned char *p;
    int i;

    p = code_table_4_9_location(sec);
    if (p == NULL) return 0;

    if ((p[2] & 0x0c) == 0x0c) {	// fix bug
        i = int4_comp(p+2);
        int_char(i, p+2);
    }
    if ((p[7] & 0x0c) == 0x0c) {	// fix bug
        i = int4_comp(p+7);
        int_char(i, p+7);
    }
    return 0;
}
