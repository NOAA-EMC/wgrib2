#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * use this routine to update sec4/pdt
 * NOTE: this routine assumes that nobody
 * will use the old sec4
 * NOT thread safe, can only be used for one **sec
 *
 * 10/2013 Public Domain by Wesley Ebisuzaki
 */


static unsigned char new_sec4[SET_PDT_SIZE];

int update_sec4(unsigned char **sec, unsigned char *sec4) {
    unsigned int sec4_size, i;

    sec4_size = uint4(sec4);
    if (sec4_size > SET_PDT_SIZE || sec4_size < 9) fatal_error_i("set_sec4 problem: sec4 size %d", (int) sec4_size);

    for (i = 0; i < sec4_size; i++) new_sec4[i] = sec4[i];
    sec[4] = &(new_sec4[0]);
    return 0;
}

