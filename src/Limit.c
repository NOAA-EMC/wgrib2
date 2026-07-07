/** @file
 * @brief Limit processing to a specified number of fields.
 * 
 * This option is for NOMADS, so processing won't take forever.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Last message number processed. */
extern unsigned int last_message;

/*
 * HEADER:100:limit:misc:1:stops after X fields decoded
 */

/**
 * Stops the processing of the grib file after N messages/submessages have been decoded. 
 * The purpose of this function to limit amount of CPU time that a web server may devote 
 * to an individual request. 
 * 
 * The newer -alarm option is better suited for stopping jobs web jobs. 
 * 
 * ## Usage
 * -limit N
 * 
 * N is an integer.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * @author Wesley Ebisuzaki @date 12/2020
 */
int f_limit(ARG1) {
    int *save;
    if (mode == -1) {
        *local = save = (int *) malloc(sizeof(int));
        *save = atoi(arg1);
    }
    else if (mode == -2) {
        save = (int *) *local;
        free(save);
    }
    else if (mode >= 0 && decode == 1) {
        save = (int *) *local;
        if (--*save == 0) last_message = 1;
    }
    return 0;
}
