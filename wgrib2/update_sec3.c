#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * use this routine to update sec3/pdt
 * NOTE: this routine assumes that nobody
 * will use the old sec4
 * NOT thread safe, can only be used for one **sec
 *
 * 4/2019 Public Domain by Wesley Ebisuzaki
 */


static unsigned char *new_sec3;
static int new_sec3_size = 0;

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
