/** @file
 * @brief Else statement for conditional processing in wgrib2.
 * @author Public Domain: Wesley Ebisuzaki @date 02/2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:else:Else:0:else, -if ... -else ... -endif
 */

/**
 * Handles the else statement in wgrib2. The -else option is part of the -if, -else, 
 * -endif structure for conditional execution of wgrib2 options. More details are in
 * the -if documentation.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 02/2019
 */
int f_else(ARG0) {
    return 0;
}

