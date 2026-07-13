/** @file
 * @brief Enable TIGGE grib table.
 * @author Public Domain: Wesley Ebisuzaki @date 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Use TIGGE flag. */
int tigge;

/*
 * HEADER:100:tigge:setup:0:use modified-TIGGE grib table
 */

/**
 * Changes the names of the variables to a table inspired by TIGGE. 
 * 
 * ## Usage 
 * -tigge
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, 1 on failure.
 */
int f_tigge(ARG0) {
    if (mode == -1) tigge = 1;
    return 0;
}

