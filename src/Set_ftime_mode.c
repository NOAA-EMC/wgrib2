/** @file
 * @brief Control ftime output.
 * @author Public Domain: Wesley Ebisuzaki @date 1/2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"


/*
 * HEADER:100:set_ftime_mode:setup:1:set ftime mode X,  0 - default,  1 - do not simplify time codes
 */

/** Ftime control. 0 - default, 1 - stop changing 60 sec -> 1 min, 60 min -> 1 hour,  24 hour -> 1 day. */
int ftime_mode = 0;

/**
 * Set ftime mode.
 * 
 * ## Usage
 * -set_ftime_mode X
 *
 * X = 0 or 1
 *
 * 0 - default
 * 
 * 1 - stop changing 60 sec -> 1 min, 60 min -> 1 hour,  24 hour -> 1 day
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise  
 * 
 * @author Wesley Ebisuzaki @date 1/2023
 */
int f_set_ftime_mode(ARG1) {

    if (strcmp(arg1,"0") == 0) {
        ftime_mode = 0;
    }
    else if (strcmp(arg1,"1") == 0) {
        ftime_mode = 1;
    }
    else fatal_error("set_ftime_mode: input must  be 0 or 1");
    return  0;
}
