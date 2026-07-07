/** @file
 * @brief For loop processing for wgrib2 options.
 *
 * The -for option allows you to set up a for-loop for records to process.
 *
 * For example, to only process the first 100 records:
 *
 * -for 1:100
 * @author Public Domain: Wesley Ebisuzaki @date 10/2008
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:for:setup:1:process record numbers in range,  X=(start:end:step), only one -for allowed
 */

/** Flag indicating if for loop is active. */
extern int for_mode;

/** Starting record number in for loop. */
extern int for_start;

/** Ending record number in for loop. */
extern int for_end;

/** Step size for for loop. */
extern int for_step;

/**
 * Selects the range of record  numbers upon which to operate using the "for" syntax. 
 * If you want to operate on records 10 to 20, you would use the parameter 10:20. If 
 * you want to operate on all the even records from 10 to 20, you would use 10:20:2. 
 * The restrictions are the start value must be less than the ending value and the 
 * step has to be greater than zero. 
 * 
 * The -for option ignores the submessage number when selecting the fields. 
 * 
 * ## Usage
 * -for I:J:K  
 *      same as for n = I to J by K
 * -for I:J
 *      same as for n = I to J by 1
 * -for I::K
 *      same as for n = I to MAX_INTEGER by K
 * -for I
 *      same as for n = I to MAX_INTEGER by 1
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
 */
int f_for(ARG1)  {
    if (mode == -1) {
        if (for_mode == 1) fatal_error("for: only one for allow","");
        for_mode = 1;
        parse_loop(arg1, &for_start, &for_end, &for_step);
    }
    return 0;
}
