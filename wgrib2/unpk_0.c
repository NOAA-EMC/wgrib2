#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include "wgrib2.h"

#ifdef USE_OPENMP
#include <omp.h>
#else
#define omp_get_num_threads()		1
#endif

/* 1996				wesley ebisuzaki
 *
 * Unpack BDS section
 *
 * input: *bits, pointer to packed integer data
 *        *bitmap, pointer to bitmap (undefined data), NULL if none
 *        n_bits, number of bits per packed integer
 *        n, number of data points (includes undefined data)
 *        ref, scale: flt[] = ref + scale*packed_int
 * output: *flt, pointer to output array
 *        undefined values filled with UNDEFINED
 *
 * note: code assumes an integer >= 32 bits
 *
 * 7/98 v1.2.1 fix bug for bitmaps and nbit >= 25 found by Larry Brasfield
 * 2/01 v1.2.2 changed jj from long int to double
 * 3/02 v1.2.3 added unpacking extensions for spectral data 
 *             Luis Kornblueh, MPIfM 
 * 7/06 v.1.2.4 fixed some bug complex packed data was not set to undefined
 * 10/15 v.1.2.5 changed n and i to unsigned
 * 3/16 v.1.2.6 OpenMP
 * 6/16 v.1.2.7 faster OpenMP and optimization
 */

static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};
static double shift[9] = {1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0};

void unpk_0(float *flt, unsigned char *bits0, unsigned char *bitmap0,
	int n_bits, unsigned int n, double ref, double scale, double dec_scale) {

    unsigned char *bits, *bitmap;

    int c_bits, j_bits, nthreads;
    unsigned int map_mask, bbits, i, j, k, n_missing, ndef, di;
    double jj;

    ref = ref * dec_scale;
    scale = scale * dec_scale;
    bits = bits0;
    bitmap = bitmap0;

    bbits = 0;

    /* assume integer has 32+ bits */
    /* optimized code for n_bits <= 25bits */
    if (n_bits <= 25) {
        n_missing = bitmap ? missing_points(bitmap0, n) : 0;
	ndef = n - n_missing;

	// 1-cpu: rd_bitstream_flt(bits0, 0, flt+n_missing, n_bits, ndef);
	// 1-cpu: for (j = 0; j < ndef; j++) flt[j+n_missing] = ref + scale*flt[j+n_missing];

#pragma omp parallel private(i,j,k)
	{
#pragma omp single
	    {
	        nthreads = omp_get_num_threads();
	        di = (ndef + nthreads - 1) / nthreads;
                di = ((di + 7) | 7) ^ 7;
	    }
#pragma omp for
	    for (i = 0; i < ndef; i += di) {
	        k  = ndef - i;
	        if (k > di) k = di;
	        rd_bitstream_flt(bits0 + (i/8)*n_bits, 0, flt+n_missing+i, n_bits, k);
	        for (j = i+n_missing; j < i+k+n_missing; j++) {
		    flt[j] = ref + scale*flt[j];
	        }
	    }
	}
/*
#pragma omp parallel for private(i,j,k)
	for (i = 0; i < ndef; i += CACHE_LINE_BITS) {
	    k  = ndef - i;
	    if (k > CACHE_LINE_BITS) k = CACHE_LINE_BITS;
	    rd_bitstream_flt(bits0 + (i/8)*n_bits, 0, flt+n_missing+i, n_bits, k);
	    for (j = i+n_missing; j < i+k+n_missing; j++) {
		flt[j] = ref + scale*flt[j];
	    }
	}
*/

	if (n_missing != 0) {
	    j = n_missing;
	    for (i = 0; i < n; i++) {
		/* check bitmap */
		if ((i & 7) == 0) bbits = *bitmap++;
		if (bbits & 128) {
		    flt[i] = flt[j++];
		}
		else {
		    flt[i] = UNDEFINED;
		}
		bbits = bbits << 1;
	    }
        }
    }
    else {
	/* older unoptimized code, not often used */
        c_bits = 8;
        map_mask = 128;
        while (n-- > 0) {
	    if (bitmap) {
	        j = (*bitmap & map_mask);
	        if ((map_mask >>= 1) == 0) {
		    map_mask = 128;
		    bitmap++;
	        }
	        if (j == 0) {
		    *flt++ = UNDEFINED;
		    continue;
	        }
	    }

	    jj = 0.0;
	    j_bits = n_bits;
	    while (c_bits <= j_bits) {
	        if (c_bits == 8) {
		    jj = jj * 256.0  + (double) (*bits++);
		    j_bits -= 8;
	        }
	        else {
		    jj = (jj * shift[c_bits]) + (double) (*bits & mask[c_bits]);
		    bits++;
		    j_bits -= c_bits;
		    c_bits = 8;
	        }
	    }
	    if (j_bits) {
	        c_bits -= j_bits;
	        jj = (jj * shift[j_bits]) + (double) ((*bits >> c_bits) & mask[j_bits]);
	    }
	    *flt++ = ref + scale*jj;
        }
    }
    return;
}
