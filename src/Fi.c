/** @file
 * @brief Option used in the old IF structure (deprecated).
 * @author Public Domain: Wesley Ebisuzaki @date 2/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:fi:output:0:deprecated, used in old IF structure
 */

/**
 * Used by the version 1 IF block. It is recommened that version 2 of the IF block be used and 
 * -fi no longer be used.
 *
 * The -endif option is the new way to close an IF block (wgrib2 v3.0.0+). Wgrib2 now handles 
 * both the old and new IF blocks. While there is no plans to remove the old IF blocks, it is 
 * recommended that the new IF blocks be used because the scripts will be easier to read by 
 * future users. Note that the old and new IF blocks cannot be mixed.
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 *
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_fi(ARG0) {
    return 0;
}

