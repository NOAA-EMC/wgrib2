/** @file
 * @brief Implements 4th NCEP bug fix. This fixes NCEP grib2 files where DX and DY are 
 * undefined.
 * @author Public Domain: Wesley Ebisuzaki @date 08/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Fix_ncep_4.c
 *
 * sets flag controlling whether DX and DY are defined
 *
 * 8/2008: Public Domain: Wesley Ebisuzaki
 *
 */

/** Flag to indicate if NCEP fix 4 is active.  */
extern int fix_ncep_4_flag;

/*
 * HEADER:100:fix_ncep_4:setup:0:fixes NCEP grib2 files where DX and DY are undefined
 */

/**
 * "Set" option controlling whether DX and DY are defined in NCEP files.
 *
 * This option is used to fix NCEP grib2 files where DX and DY are undefined.
 *
 * ## Usage
 * -fix_ncep_4
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * @author Wesley Ebisuzaki @date 08/2008
 */
int f_fix_ncep_4(ARG0) {
    fix_ncep_4_flag = 1;
    return 0;
}

/**
 * Implements bug fix for NCEP files where DX and DY are undefined. 
 *
 * @param sec Pointer to the GRIB section.
 *
 * @return 0 for success.
 *
 * @author Wesley Ebisuzaki @date 08/2008
 */
int fix_ncep_4(unsigned char **sec) {
    int i;
    i = flag_table_3_3(sec);
    if (i >= 0) {
        i = i | 48;
        set_flag_table_3_3(sec, i);
    }
    return 0;
}
