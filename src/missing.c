#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"

/*
 *  Public domain:  w. ebisuzaki 
 *  number of missing data points  as determined by bitmap
 *
 *  v1.1: just faster my dear
 *  v1.2: just faster my dear
 *  v1.3: just faster my dear
 *
 */

static unsigned int bitsum[256] = {
    8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0};


unsigned int missing_points(unsigned char *bitmap, unsigned int n) {

    unsigned int count, i, j, rem;
    if (bitmap == NULL) return 0;
/*
    count = 0;
    while (n >= 8) {
	tmp = *bitmap++;
	n -= 8;
        count += bitsum[tmp];
    }
    tmp = *bitmap | ((1 << (8 - n)) - 1);
    count += bitsum[tmp];
*/

    j = n >> 3;
    rem = n & 7;
    count = 0;
#pragma omp parallel for private(i) reduction(+:count)
    for (i = 0; i < j; i++) {
	  count += bitsum[bitmap[i]];
    }
    count += rem ? bitsum[bitmap[j] | ((1 << (8 - rem)) - 1)] : 0;

    return count;
}
