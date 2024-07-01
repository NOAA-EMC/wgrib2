#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_OPENMP
#include <omp.h>
#else
#define omp_get_num_threads()           1
#define omp_get_thread_num()		0
#endif

// 2009 public domain wesley ebisuzaki
//
// note: assumption that the grib file will use 25 bits or less for storing data
//       (limit of bitstream unpacking routines)
// note: assumption that all data can be stored as integers and have a value < INT_MAX

// #define DEBUG

int unpk_complex(unsigned char **sec, float *data, unsigned int ndata) {

    unsigned int i, ii, j, n_bytes, n_bits;
    int k, nbits, ref_group_length;
    unsigned char *p, *d, *mask_pointer;
    double ref_val, ref_val0, factor_10, factor_2;
    float missing1, missing2;
    int n_sub_missing;
    int pack, offset;
    unsigned int clocation;
    unsigned int ngroups, ref_group_width, nbit_group_width, len_last, npnts;
    int nbits_group_len, group_length_factor;
    int *group_refs, *group_widths, *group_lengths, *group_offset, *udata;
    unsigned int *group_clocation, *group_location, mask;

    int m1, m2, last, penultimate;
    int extra_vals[2];
    int min_val;
    int ctable_5_4, ctable_5_6,  bitmap_flag, extra_octets;
    int nthreads, thread_id;
    unsigned int di;

    extra_vals[0] = extra_vals[1] = 0;
    pack = code_table_5_0(sec);
    if (pack != 2 && pack != 3) return 0;

    p = sec[5];
    ref_val0 = ieee2flt(p+11);
    factor_2 = Int_Power(2.0, int2(p+15));
    factor_10 = Int_Power(10.0, -int2(p+17));
    ref_val = ref_val0 * factor_10;
    nbits = p[19];
    ngroups = uint4(p+31);
    bitmap_flag = code_table_6_0(sec);
    ctable_5_6 = code_table_5_6(sec);

    if (pack == 3 && (ctable_5_6 != 1 && ctable_5_6 != 2)) 
	fatal_error_i("unsupported: code table 5.6=%d", ctable_5_6);

    extra_octets = (pack == 2) ? 0 : sec[5][48];

    if (ngroups == 0) {
	if (bitmap_flag == 255) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
            for (i = 0; i < ndata; i++) data[i] = ref_val;
            return 0;
        }
        else if (bitmap_flag == 0 || bitmap_flag == 254) {
            mask_pointer = sec[6] + 6;
            mask = 0;
            for (i = 0; i < ndata; i++) {
                if ((i & 7) == 0) mask = *mask_pointer++;
                data[i] = (mask & 128) ?  ref_val : UNDEFINED;
                mask <<= 1;
            }
            return 0;
        }
        else fatal_error("unknown bitmap", "");
    }

    ctable_5_4 = code_table_5_4(sec);
    ref_group_width = p[35];
    nbit_group_width = p[36];
    ref_group_length = uint4(p+37);
    group_length_factor = p[41];
    len_last = uint4(p+42);
    nbits_group_len = p[46];

#ifdef DEBUG
    fprintf(stderr,"ctable 5.4 %d ref_group_width %u nbit_group_width %u ref_group_length %u group_length_factor %d\n",
        ctable_5_4, ref_group_width, nbit_group_width, ref_group_length, group_length_factor);
    fprintf(stderr,"len_last %u nbit_group_len %u\n", len_last, nbits_group_len);
#endif

    npnts =  GB2_Sec5_nval(sec); 	// number of defined points
    n_sub_missing = sub_missing_values(sec, &missing1, &missing2);

    // allocate group widths and group lengths
    group_refs = (int *) malloc(sizeof (unsigned int) * (size_t) ngroups);
    group_widths = (int *) malloc(sizeof (unsigned int) * (size_t) ngroups);
    group_lengths = (int *) malloc(sizeof (unsigned int) * (size_t) ngroups);
    group_location = (unsigned int *) malloc(sizeof (unsigned int) * (size_t) ngroups);
    group_clocation = (unsigned int *) malloc(sizeof (unsigned int) * (size_t) ngroups);
    group_offset = (int *) malloc(sizeof (unsigned int) * (size_t) ngroups);
    udata = (int *) malloc(sizeof (unsigned int) * (size_t) npnts);
    if (group_refs == NULL || group_widths == NULL || group_lengths == NULL || 
	group_location == NULL || group_clocation == NULL || group_offset == NULL
	|| udata == NULL) fatal_error("unpk_complex: memory allocation","");

    // read any extra values
    d = sec[7]+5;
    min_val = 0;
    if (extra_octets) {
	extra_vals[0] = uint_n(d,extra_octets);
	d += extra_octets;
	if (ctable_5_6 == 2) {
	    extra_vals[1] = uint_n(d,extra_octets);
	    d += extra_octets;
	}
	min_val = int_n(d,extra_octets);
	d += extra_octets;
    }

    if (ctable_5_4 != 1) fatal_error_i("internal decode does not support code table 5.4=%d",
		ctable_5_4);

    // do a check for number of grid points and size
    clocation = offset = n_bytes = n_bits = j = 0;

#pragma omp parallel private (i, ii, k, di, thread_id, nthreads)
{
    nthreads = omp_get_num_threads();
    thread_id = omp_get_thread_num();

    // want to split work into nthreads, 
    // want di * nthreads >= ngroups
    // want dt % 8 == 0  so that the offset doesn't change
    //    having offset == 0 is fastest

    di = (ngroups + nthreads - 1) / nthreads;
    di = ((di + 7) | 7) ^ 7;

    i = thread_id * di;
    if (i < ngroups) {
	k  = ngroups - i;
	if (k > di) k = di;

        // read the group reference values
   	rd_bitstream(d + (i/8)*nbits, 0, group_refs+i, nbits, k);

	// read the group widths
	rd_bitstream(d+(nbits*ngroups+7)/8+(i/8)*nbit_group_width, 0,
			group_widths+i,nbit_group_width,k);
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	for (ii = 0; ii < k; ii++) group_widths[i+ii] += ref_group_width;
    }

#pragma omp barrier

    if (ctable_5_4 == 1) {

	// for(i = 0; i < ngroups-1; i++) 

        // di * nthreads > (ngroups-1)
        di = (ngroups - 1 + nthreads - 1) / nthreads;
        // di * nthreads is now a multiple of 8
        di = ((di + 7) | 7) ^ 7;
	i = thread_id * di;
        if (i < ngroups - 1) {
            k  = ngroups - 1 - i;
            if (k > di) k = di;
	    rd_bitstream(d+(nbits*ngroups+7)/8+(ngroups*nbit_group_width+7)/8 + 
	       (i/8)*nbits_group_len, 0,group_lengths + i, nbits_group_len, k);
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	    for (ii = 0; ii < k; ii++) group_lengths[i+ii] = 
		    group_lengths[i+ii] * group_length_factor + ref_group_length;
        }

#pragma omp single
	group_lengths[ngroups-1] = len_last;
    }

/* old version
#pragma omp sections
    {
    
#pragma omp section
        {
           // read the group reference values
   	   rd_bitstream(d, 0, group_refs, nbits, ngroups);
	}

#pragma omp section
	{
	    unsigned int i;
	    // read the group widths

	    rd_bitstream(d+(nbits*ngroups+7)/8,0,group_widths,nbit_group_width,ngroups);
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	    for (i = 0; i < ngroups; i++) group_widths[i] += ref_group_width;
	}


#pragma omp section
	{
	    unsigned int i;
	    // read the group lengths

	    if (ctable_5_4 == 1) {
		rd_bitstream(d+(nbits*ngroups+7)/8+(ngroups*nbit_group_width+7)/8,
		0,group_lengths, nbits_group_len, ngroups-1);

#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
		for (i = 0; i < ngroups-1; i++) {
		    group_lengths[i] = group_lengths[i] * group_length_factor + ref_group_length;
		}
		group_lengths[ngroups-1] = len_last;
	    }
	}

    }
*/

#pragma omp single
    {
        d += (nbits*ngroups + 7)/8 +
             (ngroups * nbit_group_width + 7) / 8 +
             (ngroups * nbits_group_len + 7) / 8;
    }

#pragma omp sections
    {

#pragma omp section
        {
	    unsigned int i;
            for (i = 0; i < ngroups; i++) {
	        group_location[i] = j;
	        j += group_lengths[i];
	    }
	}

#pragma omp section
        {
	    unsigned int i;
            for (i = 0; i < ngroups; i++) {
		/* eliminate potential overflow here */
	        // n_bytes += (group_lengths[i]*group_widths[i]) / 8;
	        // n_bits += (group_lengths[i]*group_widths[i]) % 8;
	        n_bytes += (group_lengths[i] / 8) * (group_widths[i]);
		n_bits += (group_lengths[i] % 8) * (group_widths[i]);
		n_bytes += n_bits / 8;
		n_bits = n_bits % 8;
            }
        }

#pragma omp section
	{
	    unsigned int i;
            for (i = 0; i < ngroups; i++) {
	        group_clocation[i] = clocation;
	        clocation += group_lengths[i]*(group_widths[i]/8) +
	              (group_lengths[i]/8)*(group_widths[i] % 8);
            }
        }

#pragma omp section
        {
	    unsigned int i;
            for (i = 0; i < ngroups; i++) {
	        group_offset[i] = offset;
	        offset += (group_lengths[i] % 8)*(group_widths[i] % 8);
	    }
        }
    }
}

    if (j != npnts) fatal_error_u("bad complex packing: n points %u",j);
    n_bytes += (n_bits+7)/8;

    if (d + n_bytes - sec[7] != GB2_Sec7_size(sec))
        fatal_error("complex unpacking size mismatch old test","");


    if (d + clocation + (offset + 7)/8 - sec[7] != GB2_Sec7_size(sec)) fatal_error("complex unpacking size mismatch","");

#pragma omp parallel for private(i) schedule(static)
    for (i = 0; i < ngroups; i++) {
	group_clocation[i] += (group_offset[i] / 8);
	group_offset[i] = (group_offset[i] % 8);

	rd_bitstream(d + group_clocation[i], group_offset[i], udata+group_location[i], 
		group_widths[i], group_lengths[i]);
    }

    // handle substitute, missing values and reference value
    if (n_sub_missing == 0) {
#pragma omp parallel for private(i,k,j)
	for (i = 0; i < ngroups; i++) {
	    j = group_location[i];
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	    for (k = 0; k < group_lengths[i]; k++) {
		udata[j+k] += group_refs[i];
	    }
	}
    }
    else if (n_sub_missing == 1) {

#pragma omp parallel for private(i,m1,k,j)
	for (i = 0; i < ngroups; i++) {
	    j = group_location[i];
	    if (group_widths[i] == 0) {
	        m1 = (1 << nbits) - 1;
		if (m1 == group_refs[i]) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
		    for (k = 0; k < group_lengths[i]; k++) udata[j+k] = INT_MAX;
		}
		else {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
		    for (k = 0; k < group_lengths[i]; k++) udata[j+k] += group_refs[i];
		}

	    }
	    else {
	        m1 = (1 << group_widths[i]) - 1;
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	        for (k = 0; k < group_lengths[i]; k++) {
		    if (udata[j+k] == m1) udata[j+k] = INT_MAX;
		    else udata[j+k] += group_refs[i];
		}
	    }
	}
    }
    else if (n_sub_missing == 2) {
#pragma omp parallel for private(i,j,k,m1,m2)
	for (i = 0; i < ngroups; i++) {
	    j = group_location[i];
	    if (group_widths[i] == 0) {
	        m1 = (1 << nbits) - 1;
	        m2 = m1 - 1;
		if (m1 == group_refs[i] || m2 == group_refs[i]) {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
		    for (k = 0; k < group_lengths[i]; k++) udata[j+k] = INT_MAX;
		}
		else {
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
		    for (k = 0; k < group_lengths[i]; k++) udata[j+k] += group_refs[i];
		}
	    }
	    else {
	        m1 = (1 << group_widths[i]) - 1;
	        m2 = m1 - 1;
#ifdef IS_OPENMP_4_0
#pragma omp simd
#endif
	        for (k = 0; k < group_lengths[i]; k++) {
		    if (udata[j+k] == m1 || udata[j+k] == m2) udata[j+k] = INT_MAX;
		    else udata[j+k] += group_refs[i];
		}
	    }
	}
    }

    // post processing

	if (pack == 3) {
	    if (ctable_5_6 == 1) {
		last = extra_vals[0];
		i = 0;
		while (i < npnts) {
		    if (udata[i] == INT_MAX) i++;
		    else {
			udata[i++] = extra_vals[0];
			break;
		    }
		}
		for (; i < npnts; i++) {
		    if (udata[i] != INT_MAX) {
			udata[i] += last + min_val;
			last = udata[i];
		    }
		}
	    }
	    else if (ctable_5_6 == 2) {
		penultimate = extra_vals[0];
		last = extra_vals[1];

		i = 0;
		while (i < npnts) {
		    if (udata[i] == INT_MAX) i++;
		    else {
			udata[i++] = extra_vals[0];
			break;
		    }
		}
		while (i < npnts) {
		    if (udata[i] == INT_MAX) i++;
		    else {
			udata[i++] = extra_vals[1];
			break;
		    }
		}
	        for (; i < npnts; i++) {
		    if (udata[i] != INT_MAX) {
			udata[i] =  udata[i] + min_val + last + last - penultimate;
			penultimate = last;
			last = udata[i];
		    }
		}
	    }
	    else fatal_error_i("Unsupported: code table 5.6=%d", ctable_5_6);
	}

	// convert to float

	if (bitmap_flag == 255) {
	    // no bitmap
#pragma omp parallel for schedule(static) private(i)
	    for (i = 0; i < ndata; i++) {
		data[i] = (udata[i] == INT_MAX) ? UNDEFINED : 
			(ref_val0 + udata[i] * factor_2) * factor_10;
	    }
	}
        else if (bitmap_flag == 0 || bitmap_flag == 254) {
	    // bitmap, old code was not optimized because using bitmap increases the file
	    // size, but NCEP did it anyway to be compatible with NCEP codes
	    j = mask = 0;
            mask_pointer = sec[6] + 6;
	    i = 0;
	    while (i < ndata) {
                if ((i & 7) == 0) mask = *mask_pointer++;
	        data[i++] = (mask & 128) ? (ref_val0 + udata[j++] * factor_2) * factor_10 : UNDEFINED;
		mask <<= 1;
            }
        }
        else fatal_error_i("unknown bitmap: %d", bitmap_flag);

	free(group_refs);
	free(group_widths);
	free(group_lengths);
	free(group_location);
	free(group_clocation);
	free(group_offset);
	free(udata);

    return 0;
}
