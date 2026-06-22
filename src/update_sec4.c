/** @file
 * @brief Updates Section 4 (Product Definition Section).
 * @author Public Domain: Wesley Ebisuzaki @date 10/2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

/** Pointer to the new Section 4. */
static unsigned char new_sec4[SET_PDT_SIZE];

/**
 * Routine to update Section 4 (Product Definition Section).
 *
 * This routine assumes that nobody will use the old Section 4.
 *
 * NOT thread safe! Can only be used for on **sec.
 *
 * @param sec Pointer to GRIB sections containing the new Section 4.
 * @param sec4 Pointer to the new Section 4.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 *
 * @author Wesley Ebisuzaki @date 10/2013
 */
int update_sec4(unsigned char **sec, unsigned char *sec4) {
    unsigned int sec4_size, i;

    sec4_size = uint4(sec4);
    if (sec4_size > SET_PDT_SIZE || sec4_size < 9) fatal_error_i("set_sec4 problem: sec4 size %d", (int) sec4_size);

    for (i = 0; i < sec4_size; i++) new_sec4[i] = sec4[i];
    sec[4] = &(new_sec4[0]);
    return 0;
}

