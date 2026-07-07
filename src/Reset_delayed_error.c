/** @file
 * @brief wgrib2 supports delayed fatal errors. This allows the user to examine the 
 * grib message that caused the fatal error.This is useful for finding the problem 
 * in the grib message.
 * 
 * The -reset_delayed_error option allows you to reset the delayed error flag and 
 * continue processing. Note that this may cause the processing of bad grib files 
 * to continue, which could lead to seg faults. For example, if Section 3 is the wrong 
 * size, and you process the file as if section 3 is the right size.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */
#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Last message to process. */
extern unsigned int last_message;

/*
 * HEADER:100:reset_delayed_error:inv:0:clear reset_delayed_error flag
 */

/**
 * Clear the delayed error flag.
 * 
 * Wgrib2 has two behaviors for fatal errors. The first behavior is to quit immediately. The 
 * second behavior (delayed error) is for wgrib2 to quit after processing the error-causing 
 * grib message. This second behavior is useful because it allows you to examine the bad grib 
 * message, and to fix the grib message. 
 * 
 * The -reset_delayed_error option is an inventory option that clears the delayed-error flags. 
 * Note that options after the -reset_delayed_error option may also raise delayed errors. 
 * 
 * Processing grib files with delayed errors can cause seg faults and other errors. For example, 
 * a delayed error can be raised when Section 3 is not of the correct size. If wgrib2 is processing 
 * Section 3 assuming it has the right size, and the actual size is smaller than the right size, 
 * you could have seg faults. 
 * 
 * ## Usage
 * -reset_delayed_error
 * 
 * If delayed error flag = 0, no error
 *
 * The delayed error flag is the sum of the following errors:
 * 
 * - 1 :: DELAYED_NONERROR_END
 * - 2 :: DELAYED_PDT_SIZE_ERR
 * - 4 :: DELAYED_LOCAL_GRIBTABLE_ERR
 * - 8 :: DELAYED_GRID_SIZE_ERR
 * - 16 :: DELAYED_FTIME_ERR
 * - 32 :: DELAYED_MISC
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * @author Wesley Ebisuzaki @date 12/2020
 */
int f_reset_delayed_error(ARG0) {
    if (mode >= 0) {
        // sprintf(inv_out,"delayed_error=%u", (last_message >> 1) << 1);
        last_message &= DELAYED_NONERROR_END;
    }
    return 0;
}
