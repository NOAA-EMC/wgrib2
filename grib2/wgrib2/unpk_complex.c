#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

// 2009 public domain wesley ebisuzaki
//
// note: assumption that the grib file will use 25 bits or less for storing data
//       (limit of bitstream unpacking routines)
// note: assumption that all data can be stored as integers and have a value < INT_MAX

// #define DEBUG

int unpk_complex(unsigned char **sec, float *data, unsigned int ndata) {

    unsigned int i, j, n;
    int k, nbits, ref_group_length;
    unsigned char *p, *d, *mask_pointer;
    double ref_val,factor_10, factor_2, factor;
    float missing1, missing2;
    int n_sub_missing;
    int pack, offset;
    unsigned clocation;
    unsigned int ngroups, ref_group_width, nbit_group_width, len_last, npnts;
    int nbits_group_len, group_length_factor;
    int *group_refs, *group_widths, *group_lengths, *group_offset, *udata;
    unsigned int *group_clocation, *group_location;

    int m1, m2, mask, last, penultimate;
    int extra_vals[2];
    int min_val;
    int ctable_5_4, ctable_5_6,  bitmap_flag, extra_octets;


    extra_vals[0] = extra_vals[1] = 0;
    pack = code_table_5_0(sec);
    if (pack != 2 && pack != 3) return 0;

    p = sec[5];
    ref_val = ieee2flt(p+11);
    factor_2 = Int_Power(2.0, int2(p+15));
    factor_10 = Int_Power(10.0, -int2(p+17));
    ref_val *= factor_10;
    factor = factor_2 * factor_10;
    nbits = p[19];
    ngroups = uint4(p+31);
    bitmap_flag = code_table_6_0(sec);
    ctable_5_6 = code_table_5_6(sec);

    if (pack == 3 && (ctable_5_6 != 1 && ctable_5_6 != 2)) 
	fatal_error_i("unsupported: code table 5.6=%d", ctable_5_6);

    extra_octets = (pack == 2) ? 0 : sec[5][48];

    if (ngroups == 0) {
	if (bitmap_flag == 255) {
            for (i = 0; i < ndata; i++) data[i] = ref_val;
            return 0;
        }
        if (bitmap_flag == 0 || bitmap_flag == 254) {
            mask_pointer = sec[6] + 6;
            mask = 0;
            for (i = 0; i < ndata; i++) {
                if ((i & 7) == 0) mask = *mask_pointer++;
                data[i] = (mask & 128) ?  ref_val : UNDEFINED;
                mask <<= 1;
            }
            return 0;
        }
        fatal_error("unknown bitmap", "");
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

#pragma omp parallel
{
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
	    for (i = 0; i < ngroups; i++) group_widths[i] += ref_group_width;
	}


#pragma omp section
	{
	    unsigned int i;
	    // read the group lengths

	    if (ctable_5_4 == 1) {
		rd_bitstream(d+(nbits*ngroups+7)/8+(ngroups*nbit_group_width+7)/8,
		0,group_lengths, nbits_group_len, ngroups-1);

		for (i = 0; i < ngroups-1; i++) {
		    group_lengths[i] = group_lengths[i] * group_length_factor + ref_group_length;
		}
		group_lengths[ngroups-1] = len_last;
	    }
	}

    }


#pragma omp single
    {
        d += (nbits*ngroups + 7)/8 +
             (ngroups * nbit_group_width + 7) / 8 +
             (ngroups * nbits_group_len + 7) / 8;

	// do a check for number of grid points and size
	clocation = offset = n = j = 0;
    }

#pragma omp sections
    {


#pragma omp section
        {
	    unsigned int i;
            for (i = 0; i < ngroups; i++) {
	        group_location[i] = j;
	        j += group_lengths[i];
	        n += group_lengths[i]*group_widths[i];
            }
        }

#pragma omp section
	{
	    unsigned int i;
            for (i = 0; i < ngroups; i++) {
	        group_clocation[i] = clocation;
	        clocation = clocation + group_lengths[i]*(group_widths[i]/8) +
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
    if (d + (n+7)/8 - sec[7] != GB2_Sec7_size(sec))
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
	    for (k = 0; k < group_lengths[i]; k++) {
		udata[j++] += group_refs[i];
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
		    for (k = 0; k < group_lengths[i]; k++) udata[j++] = INT_MAX;
		}
		else {
		    for (k = 0; k < group_lengths[i]; k++) udata[j++] += group_refs[i];
		}
	    }
	    else {
	        m1 = (1 << group_widths[i]) - 1;
	        for (k = 0; k < group_lengths[i]; k++) {
		    if (udata[j] == m1) udata[j] = INT_MAX;
		    else udata[j] += group_refs[i];
		    j++;
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
		    for (k = 0; k < group_lengths[i]; k++) udata[j++] = INT_MAX;
		}
		else {
		    for (k = 0; k < group_lengths[i]; k++) udata[j++] += group_refs[i];
		}
	    }
	    else {
	        m1 = (1 << group_widths[i]) - 1;
	        m2 = m1 - 1;
	        for (k = 0; k < group_lengths[i]; k++) {
		    if (udata[j] == m1 || udata[j] == m2) udata[j] = INT_MAX;
		    else udata[j] += group_refs[i];
		    j++;
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
		while (i < npnts) {
		    if (udata[i] == INT_MAX) i++;
		    else {
			udata[i] += last + min_val;
			last = udata[i++];
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
#pragma omp parallel for schedule(static) private(i)
	    for (i = 0; i < ndata; i++) {
		data[i] = (udata[i] == INT_MAX) ? UNDEFINED : 
			ref_val + udata[i] * factor;
	    }
	}
        else if (bitmap_flag == 0 || bitmap_flag == 254) {
	    n = 0;
	    mask = 0;
            mask_pointer = sec[6] + 6;
            for (i = 0; i < ndata; i++) {
                if ((i & 7) == 0) mask = *mask_pointer++;
		if (mask & 128) {
		    if (udata[n] == INT_MAX) data[i] = UNDEFINED;
		    else data[i] = ref_val + udata[n] * factor;
		    n++;
		}
		else data[i] = UNDEFINED;
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
