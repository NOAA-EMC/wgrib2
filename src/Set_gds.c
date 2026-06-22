/** @file
 * @brief Routine to make a blank GDS (Grid Definition Section).
 * @author Public Domain: Wesley Ebisuzaki @date 4/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:set_gds:misc:1:makes new gds (section 3), X=size in bytes
 */

/**
 * Makes a new Section 3 (Grid Definition Section).
 * 
 * ## Usage
 * -set_gds X
 *
 * X is the size of the new GDS in bytes.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 4/2019
 */
int f_set_gds(ARG1) {

    unsigned char *new_sec3;
    int size_new_sec3, i;

    if (mode < 0) return 0;
    size_new_sec3 = atoi(arg1);
    if (size_new_sec3 <  5) {
        fatal_error("set_gdt: X=length of GDT (Sec 3)","");
        /* is needed to remove compiler warning */
        exit(1);
    }
    new_sec3 = (unsigned char *) malloc((size_t) (sizeof(unsigned char) * size_new_sec3));
    if (new_sec3 == NULL) fatal_error("new_gds: memory allocation","");

    /* size of sec3 */
    uint_char(size_new_sec3, new_sec3);
    /* id for sec3 */
    new_sec3[4] = 3;
    for (i = 5; i < size_new_sec3; i++) new_sec3[i] = 255;

    update_sec3(sec, new_sec3);
    free(new_sec3);
    return 0;
}
