/** @file
 * @brief Type of reference time.
 * @author Public Domain: Wesley Ebisuzaki @date 2023
 */
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:type_reftime:inv:0:type of reference time
 */

/**
 * Prints the type of reference time.
 * 
 * ## Usage
 * -type_reftime
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 2023
 */
int f_type_reftime(ARG0) {
    const char *string;

    if (mode < 0) return 0;
    switch(code_table_1_2(sec)) {
        case 0: string = "type_reftime=analysis"; break;
        case 1: string = "type_reftime=start of forecast"; break;
        case 2: string = "type_reftime=verifying time of forecast"; break;
        case 3: string = "type_reftime=observation time"; break;
        case 4: string = "type_reftime=local time"; break;
        default: string="type_reftime=unknown"; break;
    }
    strcat(inv_out, string);
    return 0;
}
