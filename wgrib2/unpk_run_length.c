#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"

// #define DEBUG

// unpk_run_length (8/2009) is in the public domain,  Wesley Ebisuzaki
//
// this routine unpacks a grib2 data section that is in the run length
//    packing method
//
// 8/2009 preliminary version, based on readrdr
// 9/2009 fixed typo in bitmap section
// 12/2014: David Binderman fixed line: while (vals[i] > mv && i < nvals) {
// 4/2016:  Takayuki Usui found offset by 1 problem and suggested a fix
//          added case where nvals == 0 or mv == 0
//          more error checking

int unpk_run_length(unsigned char **sec, float *data, unsigned int ndata) {

    int i, k, decimal_scale, n_bits;
    int mv, mvl;
    unsigned int j, ncheck;

    double *levels, dec_factor;
    int size_compressed_data, nvals, *vals;
    int v, n, factor, range;
    int bitmap_flag;
    unsigned char *mask_pointer;
    const unsigned int mask[] = {128,64,32,16,8,4,2,1};

    if (code_table_5_0(sec) != 200) return 0;		// only for DRT 5.200

    if (GB2_Sec3_npts(sec) != ndata) fatal_error("Run-length decoding: programming error","");
    n_bits = (int) sec[5][11];
    mv = (int) uint2(sec[5]+12);
    mvl = (int) uint2(sec[5]+14);
    decimal_scale = (int) sec[5][16];
    if (decimal_scale > 127) {		// convert to signed negative values
	decimal_scale = - (decimal_scale - 128);
    }
    dec_factor = Int_Power(10.0, -decimal_scale);
    if (mv > mvl) fatal_error_ii("Run-length decoding: mv %d > mvl %d", mv, mvl);

#ifdef DEBUG
    printf(" n_bits=%d mv=%d mvl=%d decimal_scale=%d\n", n_bits, mv, 
		mvl, decimal_scale);
#endif

    size_compressed_data = GB2_Sec7_size(sec)-5;
    nvals = ( size_compressed_data * 8) / n_bits;

#ifdef DEBUG
    printf(" size_compressed data %d ndata %u\n", size_compressed_data, ndata);
#endif

    if (nvals == 0 || mv == 0) {
	for (j = 0; j < ndata; j++) {
            data[j] = UNDEFINED;
	}
	return 0;
    }

    levels = (double *) malloc((mvl + 1) * sizeof(double));
    vals = (int *) malloc(nvals * sizeof(int));
    if (levels == NULL || vals == NULL) fatal_error("Run-length decoding: memory allocation","");

    /* levels[0..mvl]: levels[0] = UNDEFINED; levels[1..mvl] from table */
    levels[0] = UNDEFINED;
    for (i = 1; i <= mvl; i++) {
	levels[i] = int2(sec[5] + 15 + i*2)*dec_factor;
    }

#ifdef DEBUG
    for (i = 0; i <= mvl; i++) {
	printf(" lvls[%d] = %lf ", i, levels[i]);
	if (i % 4 == 0) printf("\n");
    }
    printf("\n");
#endif

    rd_bitstream(sec[7]+5, 0, vals, n_bits, nvals);

    ncheck = i = 0;
    range = (1 << n_bits) - 1 - mv;
    if (range <= 0) fatal_error("unpk_running_length: range error","");

    j = 0;
    mask_pointer = sec[6] + 6;
    bitmap_flag = code_table_6_0(sec);
    if (bitmap_flag == 254) bitmap_flag = 0;
    if (bitmap_flag != 0 && bitmap_flag != 255) 
	fatal_error("unpk_running_length: unsupported bitmap","");

    while (i < nvals) {
	if (vals[i] > mv) fatal_error_i("Run-length decoding: error val=%d",(int) i);
	v = vals[i++];

	/* figure out any repeat factor */
	n = 1;
	factor = 1;
	// 12/2014 while (vals[i] > mv && i < nvals) {
	while (i < nvals && vals[i] > mv) {
	    n += factor * (vals[i]-mv-1);
	    factor = factor * range;
	    i++;
	}

	ncheck += n;
        if (ncheck > ndata) fatal_error_uu("unpk_run_length: ncheck (%u) > data (%u),",ncheck, ndata);

	if (bitmap_flag != 0) {
	    for (k = 0; k < n; k++) {
		    data[j++] = levels[v];
	    }
	}
	else {
	    for (k = 0; k < n; k++) {
		while (mask_pointer[j >> 3] & mask[j & 7]) {
	            data[j++] = UNDEFINED;
		}
		data[j++] = levels[v];
	    }
	}
    }
    if (j != ndata) fatal_error("Run-length decoding: bitmap problem","");
    free(levels);
    free(vals);
    return 0;
}
