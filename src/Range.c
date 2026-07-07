/** @file
 * @brief For partial http transfers, you need to know start and finish of the grib message.
 * This option writes out the range of the record.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */
#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Position of the record in bytes. */
extern long int pos;

/** Length of the record in bytes. */
extern unsigned long int len;

/*
 * HEADER:100:range:inv:0:print out location of record in bytes, 0 = first byte
 */

/**
 * Prints the starting and ending byte of the grib message. If the range field is used by 
 * partial http downloading of grib files. 
 * 
 * ## Usage
 * -range
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_range(ARG0) {
    if (mode >= 0) sprintf(inv_out, "range=%ld-%ld",pos,pos+len-1);
    return 0;
}
