/** @file
 * @brief Updates Section 3 (Grid Definition Section).
 * @author Public Domain: Wesley Ebisuzaki @date 4/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

/** Pointer to the new Section 3. */
static unsigned char *new_sec3;

/** Size of the new Section 3. */
static int new_sec3_size = 0;

/**
 * Routine to update Section 3 (Grid Definition Section).
 * 
 * This routine assumes that nobody will use the old Section 3.
 *
 * NOT thread safe! Can only be used for on **sec.
 * 
 * @param sec Pointer to GRIB sections containing the new Section 3.
 * @param sec3 Pointer to the new Section 3.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 *
 * @author Wesley Ebisuzaki @date 4/2019
 */
int update_sec3(unsigned char **sec, unsigned char *sec3) {

    unsigned int sec3_size, i;

    sec3_size = uint4(sec3);
    if (sec3_size <= 5) fatal_error_i("update_sec3 problem: sec3 size %d", sec3_size);
    if (sec3_size > new_sec3_size) {
        if (new_sec3_size) free(new_sec3);
        new_sec3 = (unsigned char *) malloc(sizeof(unsigned char) * (size_t) sec3_size);
        if (new_sec3 == NULL) fatal_error("update_sec3: memory allocation","");
        new_sec3_size = sec3_size;
    }
    for (i = 0; i < sec3_size; i++) new_sec3[i] = sec3[i];
    sec[3] = &(new_sec3[0]);
    return 0;
}
