/** @file
 * @brief Provide some status of the internal state of wgrib2.
 * @author Public Domain: Wesley Ebisuzaki @date 4/2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/*
 * HEADER:100:status:misc:1:X  X=file
 */

/**
 * This option is for developers. It allows the developer to query the status at various 
 * points in the processing. 
 * 
 * ## Usage
 * -status FILE
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 4/2015
 */
int f_status(ARG1) {

    if (strncmp(arg1,"file",4) == 0) {
        fprintf(stderr,"mode=%d\n", mode);
        fprintf(stderr,"flush mode=%d\n", flush_mode);
        // inv_out += strlen(inv_out);
        fprintf(stderr,"file_append=%d\n", file_append);
        // inv_out += strlen(inv_out);
        status_ffopen();
    }
    return 0;

}
