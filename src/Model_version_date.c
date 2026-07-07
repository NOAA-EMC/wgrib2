/** @file
 * @brief Print the model version date code.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */
#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Last message to process. */
extern unsigned int last_message;

/*
 * HEADER:100:model_version_date:inv:0:prints model date code
 */

/**
 * Prints the model date code.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */
int f_model_version_date(ARG0) {
    unsigned char *p;
    if (mode >= 0) {
        p = year_of_model_version_date_location(sec);
        if (p != NULL) {
            sprintf(inv_out,"model_version_date=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d:", uint2(p),(int) p[2],
                    (int) p[3], (int) p[4], (int) p[5], (int) p[6]);
        }
    }
    return 0;
}
