#include <stdio.h>
#include "wgrib2.h"

/*
 * does a byte swap of 4-byte integers/ieee
 *
 * n should be a multiple of 4
 *
 * 3/2008 Public Domain Wesley Ebisuzaki
 * 7/2015 OpenMP version Wesley Ebisuzaki
 */

int swap_buffer(unsigned char *buffer, unsigned int n) {
    unsigned int ii;
    unsigned char i, j;

#pragma omp parallel for private(ii, i, j)
    for (ii = 0; ii < n; ii += 4) {
	i = buffer[ii];
	j = buffer[ii+1];
        buffer[ii] = buffer[ii+3];
        buffer[ii+1] = buffer[ii+2];
        buffer[ii+2] = j;
        buffer[ii+3] = i;
    }
    return 0;
}


