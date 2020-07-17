#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "wgrib2.h"
// #define DEBUG

//
// public domain 7/2009 Wesley Ebisuzaki
//
// return number of bits for an unsigned int

// #define LEN_SEC_MAX 1023
// #define WIDTH_BITS 10
// #define LEN_SEC_MAX 255

// #define LEN_SEC_MAX 102300000
// #define LEN_SEC_MAX 1023
// #define LEN_BITS 10
// #define LEN_SEC_MAX 511
// #define LEN_BITS 9
///// #define LEN_SEC_MAX 255
///// #define LEN_BITS 8
// #define LEN_SEC_MAX 127
// #define LEN_BITS 7
// #define LEN_SEC_MAX 63
// #define LEN_BITS 6

static int find_nbits(unsigned int i) {
#if !defined __GNUC__ || __GNUC__ < 4
	int j;
	j = 0;

	while (i > 65535) {
	    i = i >> 16;
	    j += 16;
	}
	// i = 16 bits
	if (i > 255) {
	    i = i >> 8;
	    j += 8;
	}
	// i = 8 bits
	if (i > 15) {
	    i = i >> 4;
	    j += 4;
	}
	// i = 4 bits
	if (i > 3) {
	    i = i >> 2;
	    j += 2;
        }
	// i = 2 bits
	return (i >= 2) ? j + 2 : j + i;
#else
        return (i == 0) ? 0 : 8 * sizeof(unsigned int) - __builtin_clz(i);
#endif
}


struct section {
        int mn, mx, missing;    	// stats
        int i0, i1;                     // pointers to data[]
        struct section *head, *tail;
};

static int sizeofsection(struct section *s, int ref_bits, int width_bits, int has_undef) {
        if (s->mn == INT_MAX) return ref_bits + width_bits;     // all undef
        if (s->mn == s->mx) {
            if (s->missing == 0) return ref_bits + width_bits;
            return (s->i1-s->i0+1)*has_undef + ref_bits + width_bits;
        }
        return find_nbits(s->mx-s->mn + has_undef)*(s->i1-s->i0+1) + ref_bits + width_bits;
}

static int sizeofsection2(int mn, int mx, int n, int ref_bits, int width_bits, int has_undef_sec, int has_undef) {

        if (mn == INT_MAX) return ref_bits + width_bits;
        if (mn == mx) {
            if (has_undef_sec == 0) return ref_bits + width_bits;
            return n*has_undef + ref_bits + width_bits;
        }
        return find_nbits(mx-mn + has_undef)*n + ref_bits + width_bits;
}

static int size_all(struct section *s, int ref_bits, int width_bits, int has_undef) {
        int bits;

        bits = 0;
        while (s) {
            bits += sizeofsection(s, ref_bits, width_bits, has_undef);
            s = s->tail;
        }
        return (bits+7)/8;
}

static void move_one_left(struct section *s, int *v) {
    struct section *t;
    int val, i, j, k;

    t = s->tail;
    s->i1 += 1;
    t->i0 += 1;
    val = v[s->i1];

    // update s statistics

    if (val == INT_MAX) s->missing = 1;
    else {
	s->mx = s->mx > val ? s->mx : val;
	s->mn = s->mn < val ? s->mn : val;
    }

    // remove t?

    if (t->i0 > t->i1) {
        s->tail = t->tail;
	t = s->tail;
	if (t) t->head = s;
	return;
    }

    // update s statistics

    if (val == INT_MAX) {
	for (i = t->i0; i <= t->i1; i++) {
	    if (v[i] == INT_MAX) return;
	}
	t->missing = 0;
	return;
    }
    if (val == t->mx) {
	k = INT_MAX;
	for (j = 0, i = t->i0; i <= t->i1; i++) {
	    if (v[i] !=  INT_MAX) {
	    if (j == 0)  {
		k = v[i];
		j++;
	    }	
	    else k = k < v[i] ? v[i] : k;
	    }
	}
	t->mx = k;
	return;
    }
    if (val == t->mn) {
	k = INT_MAX;
	for (j = 0, i = t->i0; i <= t->i1; i++) {
	    if (v[i] !=  INT_MAX) {
	    if (j == 0)  {
		k = v[i];
 		j++;
	    }	
	    else k = k > v[i] ? v[i] : k;
	    }
	}
	t->mn = k;
	return;
    }
    return;
}

static void move_one_right(struct section *s, int *v) {
    struct section *t;
    int val, i, j, k;

    t = s->tail;
    s->i1 -= 1;
    t->i0 -= 1;
    val = v[t->i0];

    // update t statistics

    if (val == INT_MAX) t->missing = 1;
    else {
	t->mx = t->mx > val ? t->mx : val;
	t->mn = t->mn < val ? t->mn : val;
    }

    // if s is empty, copy t to s and recalculate

    if (s->i0 > s->i1) {
	s->i0 = t->i0;
	s->i1 = t->i1;
	s->tail = t->tail;

	s->mx = s->mn = INT_MAX;
	j = s->missing = 0;
	for (i = s->i0; i <= s->i1; i++) {
	    if (v[i] == INT_MAX) s->missing = 1;
	    else if (j == 0) {
		s->mx = s->mn = v[i];
		j++;
	    }
	    else {
		s->mx = s->mx > v[i] ? s->mx : v[i];
		s->mn = s->mn < v[i] ? s->mx : v[i];
	    }
	}
	return;
    }

    // update s statistics

    if (val == INT_MAX) {
	for (i = s->i0; i <= s->i1; i++) {
	    if (v[i] == INT_MAX) return;
	}
	s->missing = 0;
	return;
    }
    if (val == s->mx) {
	k = INT_MAX;
	for (j = 0, i = s->i0; i <= s->i1; i++) {
	    if (v[i] !=  INT_MAX) {
	    if (j == 0)  {
		k = v[i];
		j++;
	    }	
	    else k = k < v[i] ? v[i] : k;
	    }
	}
	s->mx = k;
	return;
    }
    if (val == s->mn) {
	k = INT_MAX;
	for (j = 0, i = s->i0; i <= s->i1; i++) {
	    if (v[i] !=  INT_MAX) {
	    if (j == 0)  {
		k = v[i];
 		j++;
	    }	
	    else k = k > v[i] ? v[i] : k;
	    }
	}
	s->mn = k;
	return;
    }
    return;
} 

static void exchange(struct section *s, int *v, int has_undef, int LEN_SEC_MAX) {
	struct section *t;
	int val0, val1, nbit_s, nbit_t;

	if (s == NULL) return;
	while ((t = s->tail) != NULL) {

	    // nbit_s = find_nbits(s->mx - s->mn + has_undef);
	    // nbit_t = find_nbits(t->mx - t->mn + has_undef);

	    if (s->mn == INT_MAX) nbit_s = 0;
	    else if (s->mn == s->mx) nbit_s = s->missing;
	    else nbit_s = find_nbits(s->mx - s->mn + has_undef);

	    if (t->mn == INT_MAX) nbit_t = 0;
	    else if (t->mn == t->mx) nbit_t = t->missing;
	    else nbit_t = find_nbits(t->mx - t->mn + has_undef);

	    if (nbit_s == nbit_t) { s = t; continue; }

	    val0 = v[s->i1];
	    val1 = v[t->i0]; 

	    if (s->missing == 1 || t->missing == 1) { s=t; continue; }
//	    if (val0 == INT_MAX || val1 == INT_MAX) { s=t; continue; }

	    if (nbit_s < nbit_t	&& val1 == INT_MAX) {
	        if ((s->i1-s->i0) < LEN_SEC_MAX && s->mx != s->mn)
		    move_one_left(s, v);
		else s = t;
		continue;
	    }

	    if (nbit_s > nbit_t	&& val0 == INT_MAX) {
	        if ((t->i1-t->i0) < LEN_SEC_MAX && t->mn != t->mx) {
		    move_one_right(s, v);
		}
		else s = t;
		continue;
	    }

//	    if (s->missing == 1 || t->missing == 1) { s=t; continue; }

// 3/2014   val0 = v[s->i1];
// 3/2014   val1 = v[t->i0];

	    if (nbit_s < nbit_t && (s->i1-s->i0) < LEN_SEC_MAX &&
		val1 >= s->mn && val1 <= s->mx) {
		move_one_left(s, v);
	    }
	    else if (nbit_s > nbit_t && (t->i1-t->i0) < LEN_SEC_MAX &&
		val0 >= t->mn && val0 <= t->mx) {
		move_one_right(s, v);
	    }
	    else s = s->tail;
	}
}



static void merge_j(struct section *h, int ref_bits, int width_bits, int has_undef, 
	int param, int LEN_SEC_MAX) {
    struct section *t, *m;
    int size_head, size_mid, size_tail, saving_mt, saving_hm;
    int min0, max0, min1, max1;

    size_head = size_mid = size_tail = 0;

    while (h && (m = h->tail) ) {

	t = m->tail;

	// h -> m -> t

	// find savings of merged h - m
	saving_hm = -1;
	min0 = max0 = min1 = max1 = 0;		// turn off error warnings
	if (m->i1 - h->i0  < LEN_SEC_MAX) {
	    if (m->mn == INT_MAX) {
		max0 = h->mx;
		min0 = h->mn;
	    }
	    else if (h->mn == INT_MAX) {
		max0 = m->mx;
		min0 = m->mn;
	    }
	    else {
	        min0 = h->mn < m->mn ? h->mn : m->mn;
	        max0 = h->mx > m->mx ? h->mx : m->mx;
	    }
	    if (max0-min0 <= param) {
	        if (size_head == 0) size_head = sizeofsection(h, ref_bits, width_bits, has_undef);
		if (size_mid == 0) size_mid = sizeofsection(m, ref_bits, width_bits, has_undef);
	        saving_hm = size_head + size_mid - sizeofsection2(min0, max0, m->i1-h->i0+1, ref_bits, 
		    width_bits, h->missing || m->missing , has_undef);
	    }
	}

	// find savings of merged m-t
	saving_mt = -1;
	if (t && t->i1 - m->i0  < LEN_SEC_MAX) {
	    if (m->mn == INT_MAX) {
		max1 = t->mx;
		min1 = t->mn;
	    }
	    else if (t->mn == INT_MAX) {
		max1 = m->mx;
		min1 = m->mn;
	    }
	    else {
	        min1 = m->mn < t->mn ? m->mn : t->mn;
	        max1 = m->mx > t->mx ? m->mx : t->mx;
	    }
	    if (max1-min1 <= param) {
		if (size_mid == 0) size_mid = sizeofsection(m, ref_bits, width_bits, has_undef);
	        if (size_tail == 0) size_tail = sizeofsection(t, ref_bits, width_bits, has_undef);
	        saving_mt = size_mid + size_tail - sizeofsection2(min1, max1, t->i1-m->i0+1, ref_bits, 
		width_bits, m->missing || t->missing,  has_undef);
	    }	
	}

	if (saving_hm >= saving_mt && saving_hm >= 0) {
	    // merge h and m
	    h->i1 = m->i1;
	    h->tail = m->tail;
	    h->mn = min0;
	    h->mx = max0;
	    h->missing = h->missing || m->missing;
	    m = h->tail;
	    if (m) m->head = h;
	    if (h->head) h = h->head;
            size_head = size_mid = size_tail = 0;
	}
	else if (saving_mt >= saving_hm && saving_mt >= 0) {
	    // merge m and t
	    m->i1 = t->i1;
	    m->tail = t->tail;
	    m->mn = min1;
	    m->mx = max1;
	    m->missing = m->missing || t->missing;
	    t = m->tail;
	    if (t) t->head = m;
            size_head = size_mid = size_tail = 0;
	}
	else {
	    // no merging
	    h = h->tail;
	    size_head = size_mid;
	    size_mid = size_tail;
	    size_tail = 0;
	}
    }
}


/*
 * writes out a complex packed grib message
 */


int complex_grib_out(unsigned char **sec, float *data, unsigned int ndata, 
   int use_scale, int dec_scale, int bin_scale, int wanted_bits, int max_bits, 
   int packing_mode, int use_bitmap, struct seq_file *out) {

    int j, j0, k, *v, binary_scale, nbits, has_undef, extra_0, extra_1;
    unsigned int i, ii;
    int vmn, vmx, vbits;
    unsigned char *sec0, *sec1, *sec2 , *sec3, *sec4, *sec5, *sec6, *sec7;
    double max_val, min_val, ref, frange, dec_factor, scale;
    float mn, mx;
    struct section start, *list, *list_backup,  *s;
    int ngroups, grefmx, glenmn, glenmx, gwidmn, gwidmx, len_last;
    int size_sec7;
    int *refs, *lens, *widths, *itmp, *itmp2;
    // int est_group_width = 12;
    int est_group_width = 6;
    unsigned int ndef, nndata, nstruct;

     int LEN_SEC_MAX = 127;
     int LEN_BITS = 7;

    ndef = 0;
#pragma omp parallel for private(i) reduction(+:ndef)
    for (i = 0; i < ndata; i++) {
        if (DEFINED_VAL(data[i])) ndef = ndef + 1;
    }

    /* required passed sections */
    sec0 = sec[0];
    sec1 = sec[1];
    sec2 = sec[2];
    sec3 = sec[3];
    sec4 = sec[4];

    if (ndef == 0) {	// all undefined values
        sec5 = (unsigned char *) malloc(47 * sizeof(unsigned char));
        if (sec5 == NULL) fatal_error("complex_grib_out memory allocation sec5","");
        uint_char(47, sec5);
        sec5[4] = 5;				// section 5
        uint_char(ndata, sec5+5);		// number of points
        uint2_char(2,sec5+9);			// data template 2
        flt2ieee((float) 0.0,sec5+11);		// reference value
        int2_char(0,sec5+15);			// binary scaling
        int2_char(0,sec5+17);			// decimal scaling
        sec5[19] = 8;				// num bits for packed val
	sec5[20] = 0;				// original = float
	sec5[21] = 1;				// general group splitting
	sec5[22] = 1;				// primary missing values
        flt2ieee((float) 9.999e20,sec5+23);	// missing value
	sec5[27] = sec5[28] = sec5[29] = sec5[30] = 255; // secondary missing value
        uint_char(1,sec5+31);			// one group
	sec5[35] = 0;				// group width reference
	sec5[36] = 8;				// group width bits
        uint_char(ndata,sec5+37);		// group length ref
	sec5[41] = 1;				// inc
        uint_char(ndata,sec5+42);		// len of last group
	sec5[46] = 8;				// group length width

        // no bitmap is used
        sec6 = (unsigned char *) malloc(6 * sizeof(unsigned char));
        if (sec6 == NULL) fatal_error("complex_grib_out memory allocation sec6","");
        uint_char(6, sec6);			// size of sec 6
        sec6[4] = 6;			// section 6
        sec6[5] = 255;			// section 6 - no bitmap

        sec7 = (unsigned char *) malloc(8);
        if (sec7 == NULL) fatal_error("complex_grib_out memory allocation sec7","");
        uint_char(8, sec7);			// size of section
        sec7[4] = 7;				// section 7
	sec7[5] = 255;				// group reference
	sec7[6] = 0;				// group width
	sec7[7] = 0;				// group length

        k = wrt_sec(sec0, sec1, sec2, sec3, sec4, sec5, sec6, sec7, out);

        free(sec5);
        free(sec6);
        free(sec7);
	return k;
    }	

    /* compute bitmap section */
    if (use_bitmap == 0 || ndef == ndata) {
        // no bitmap is used
        sec6 = (unsigned char *) malloc(6 * sizeof(unsigned char));
        if (sec6 == NULL) fatal_error("complex_bitmap_grib_out memory allocation sec6","");
        uint_char(6, sec6);                             // size of sec 6
        sec6[4] = 6;                                    // section 6
        sec6[5] = 255;                                  // no bitmap
    }
    else {
	i = ndata;
        sec6 = mk_bms(data, &i);
        if (i != ndef) fatal_error("complex_grib_out prog error 1","");
    }

    /* if bitmap is used:
       data[0..ndef-1] has grid point values, no undefined values
       if bitmap is not used:
       data[0..ndata] has grid point values, possible undefined values
     */

    nndata = use_bitmap ? ndef : ndata;
    has_undef = use_bitmap ? 0 : ndata != ndef;

    v = (int *) malloc( ((size_t) nndata) * sizeof(int));
    sec5 = (unsigned char *) malloc(packing_mode == 1 ? 47 : 49);
 
    if (v == NULL || sec5 == NULL) fatal_error("complex_grib_out memory allocation v","");

    if (min_max_array(data, ndata, &mn, &mx) != 0) fatal_error("complex_pk: min max error","");
    min_val = (double) mn;
    max_val = (double) mx;

// printf("min val %lf max val %lf\n",min_val,max_val);

    binary_scale = bin_scale;

    if (use_scale == 0) {		// ECMWF style
        ref = min_val;
        frange = max_val - ref;
        dec_scale = 0;
        if (frange != 0.0) {
            frexp(frange, &j);
            binary_scale = j - wanted_bits;
            nbits = wanted_bits;
            scale = ldexp(1.0, -binary_scale);
            frange = floor((max_val-ref)*scale + 0.5);
            frexp(frange, &j);
            if (j != nbits) binary_scale++;
        }
        else {
            binary_scale = nbits = 0;
            scale = 1;
        }
    }
    else {
        if (dec_scale) {
            dec_factor = Int_Power(10.0, -dec_scale);
            min_val *= dec_factor;
            max_val *= dec_factor;
	    if (has_undef) {
#pragma omp parallel for private(i)
                for (i = 0; i < nndata; i++) {
		    if (DEFINED_VAL(data[i])) data[i] *= dec_factor;
                }
            }
	    else {
#pragma omp parallel for private(i)
                for (i = 0; i < nndata; i++) {
		    data[i] *= dec_factor;
                }
	    }
        }
        scale = ldexp(1.0, -binary_scale);
        // ref = floor(min_val*scale)/scale;
	ref = min_val;
        frange  = floor( (max_val - ref)*scale + 0.5);
        frexp(frange, &nbits);
        if (nbits > max_bits) {
            binary_scale += (nbits - max_bits);
            nbits = max_bits;
        }
    }

    if (binary_scale) {
        scale = ldexp(1.0, -binary_scale);
	if (has_undef) {
#pragma omp parallel for private(i)
            for (i = 0; i < nndata; i++) {
	        if (DEFINED_VAL(data[i])) {
		    v[i] = floor((data[i] - ref)*scale + 0.5);
		    v[i] = v[i] >= 0 ? v[i] : 0;
		}
	        else v[i] = INT_MAX;
            }
	}
	else {
#pragma omp parallel for private(i)
            for (i = 0; i < nndata; i++) {
	        v[i] = floor((data[i] - ref)*scale + 0.5);
		v[i] = v[i] >= 0 ? v[i] : 0;
	    }
	}
    }
    else {
	scale = 1.0;
	if (has_undef) {
#pragma omp parallel for private(i)
            for (i = 0; i < nndata; i++) {
	        if (DEFINED_VAL(data[i])) {
		    v[i] = floor(data[i] - ref + 0.5);
		    v[i] = v[i] >= 0 ? v[i] : 0;
		}
	        else v[i] = INT_MAX;
            }
	}
	else {
#pragma omp parallel for private(i)
            for (i = 0; i < nndata; i++) {
		v[i] = floor(data[i] - ref + 0.5);
		v[i] = v[i] >= 0 ? v[i] : 0;
            }
	}
    }
// fprintf(stderr, "\n v[i] data \n");
// for (i = 0; i < nndata;i++) {
// fprintf(stderr," %d:%d ", i, v[i]);
// }
	// preprocessing
        // for (i = 0; i < N; i++) v[i] = u[i];
        // for (i = 0; i < N; i++) v[i] = u[i] - u[i-1];
        // for (i = 0; i < N; i++) v[i] = u[i] - 2*u[i-1] + u[i-2];

    vmx = vmn = 0;
    extra_0 = extra_1 = 0;		// turn off warnings

    if (packing_mode == 3) {
//        delta_delta(v, nndata, &vmn, &vmx, &extra_0, &extra_1);

	// single core version

	{
        int last, last0, penultimate;
	for (i = 0; i < nndata; i++) {
            if (v[i] != INT_MAX) {
                extra_0 = penultimate = v[i];
                v[i++] = 0;
                break;
	    }
	}
	for ( ; i < nndata; i++) {
            if (v[i] != INT_MAX) {
                extra_1 = last = v[i];
                v[i++] = 0;
		break;
	    }
	}

	for ( ; i < nndata; i++) {
            if (v[i] != INT_MAX) {
		last0 = v[i];
                v[i] = v[i] - 2*last + penultimate;
                penultimate = last;
                last = last0;
                vmn = vmn > v[i] ? v[i] : vmn;
                vmx = vmx < v[i] ? v[i] : vmx;
	    }
	}
	}
    }
    else if (packing_mode == 2) {

//        delta(v, nndata, &vmn, &vmx, &extra_0);

	// single core version
        {
             int last, last0;

	for (i = 0; i < nndata; i++) {
            if (v[i] != INT_MAX) {
                extra_0 = last = v[i];
                v[i++] = 0;
		break;
	   }
	}
	for ( ; i < nndata; i++) {
            if (v[i] != INT_MAX) {
		last0 = v[i];
                v[i] = v[i] - last;
		last = last0;
                vmn = vmn > v[i] ? v[i] : vmn;
                vmx = vmx < v[i] ? v[i] : vmx;
	    }
	}
        }
    }
    else if (packing_mode == 1) {
	// find min/max
	int_min_max_array(v, nndata, &vmn, &vmx);
    }

//fprintf(stderr , "\n pre process v[i] data extri_0 %d extra_1 %d\n",extra_0, extra_1);
//for (i = 0; i < nndata;i++) {
//fprintf(stderr," %d:%d ", i, v[i]);
//}
//fprintf(stderr,"\n");

#ifdef DEBUG
printf("2: vmx %d vmn %d nbits %d\n", vmx, vmn, find_nbits(vmx-vmn+has_undef));
#endif

#pragma omp parallel for private(i)
    for (i = 0; i < nndata; i++) {
	v[i] = (v[i] != INT_MAX) ? v[i] - vmn : INT_MAX;
    }
    vmx = vmx-vmn;
    vbits = find_nbits(vmx+has_undef);

    /* size of merged struct */

    ii = 0;
    nstruct = 1;
    for (i = 1; i < nndata; i++) {
        if (((i - ii + 1) > LEN_SEC_MAX) || (v[i] != v[ii])) {
	    nstruct++;
	    ii = i;
	}
    }

    list = (struct section *) malloc((size_t) nstruct * sizeof(struct section));
    if (list == NULL) fatal_error("complex_grib_out: memory allocation of list failed","");

    // initialize linked list

    ii = 0;
    list[0].mn = list[0].mx = v[0];
    list[0].missing = (v[0] == INT_MAX);
    list[0].i0 = list[0].i1 = 0;
    for (i = 1; i < nndata; i++) {
	// join last section
        if ((i - list[ii].i0 < LEN_SEC_MAX) && (v[i] == list[ii].mn)) {
	    list[ii].i1 = i;
	}
	// make new section
	else {
	    ii++;
    	    list[ii].mn = list[ii].mx = v[i];
            list[ii].missing = (v[i] == INT_MAX);
            list[ii].i0 = list[ii].i1 = i;
	}
    }
    list[0].head = NULL;
    list[ii].tail = NULL;
    start.tail = &list[0];

    if (nstruct != ii+1) fatal_error_ii("complex_pk, nstruct=%d wanted %d",nstruct,ii+1);

#pragma omp parallel for private(k)
    for (i = 1; i < nstruct; i++) {
        list[i].head = &list[i-1];
	list[i-1].tail = &list[i];
    }

// sequence : has_undef == 0 :   2**n - 1       1, 3, 7, ..
// sequence : has_undef == 1 :   2**n - 2       0, 2, 6

    k = has_undef ? 2 : 1;

    while (k < vmx/2) {
        merge_j(start.tail, vbits, LEN_BITS+est_group_width, has_undef, k, LEN_SEC_MAX);
#ifdef DEBUG
        j = size_all(start.tail, vbits, LEN_BITS+est_group_width,has_undef);
        printf(" complex start %d %d bytes\n", k, j);
#endif
	k = 2*k + 1 + has_undef;
    }

//  try making segment sizes larger
//  12/2015 need to segment size less 25 bits, bitstream software limitation

    list_backup = (struct section *) malloc(((size_t) nstruct) * sizeof(struct section));
    if (list_backup == NULL) fatal_error("complex_grib_out: memory allocation of list_backup failed","");

    j = size_all(start.tail, vbits, LEN_BITS+est_group_width,has_undef);
    j0 = j+1;
#ifdef DEBUG
        printf(" complex start inc segments size0 %d segsize %d\n",j,LEN_SEC_MAX);
#endif
    while (j < j0 && LEN_BITS < 25) {
	j0 = j;
	LEN_BITS++;
	LEN_SEC_MAX = LEN_SEC_MAX + LEN_SEC_MAX + 1;
        memcpy(list_backup,list, nstruct*sizeof(struct section));
        merge_j(start.tail, vbits, LEN_BITS+est_group_width, has_undef, k, LEN_SEC_MAX);
        j = size_all(start.tail, vbits, LEN_BITS+est_group_width,has_undef);
#ifdef DEBUG
        printf(" complex inc segments size size0 %d size1 %d segsize %d LEN_BITS=%d\n",j0,j,LEN_SEC_MAX, LEN_BITS);
#endif
	if (j > j0) {
	    memcpy(list,list_backup,nstruct*sizeof(struct section));
	    LEN_BITS--;
	    LEN_SEC_MAX = (LEN_SEC_MAX - 1) / 2;
	}
    }
    free(list_backup);

    exchange(start.tail, v, has_undef, LEN_SEC_MAX);
#ifdef DEBUG
    j = size_all(start.tail, vbits, LEN_BITS+est_group_width,has_undef);
    printf(" exchange  %d bytes\n", j);
#endif

    merge_j(start.tail, vbits, LEN_BITS+est_group_width, has_undef, vmx, LEN_SEC_MAX);
#ifdef DEBUG
    j = size_all(start.tail, vbits, LEN_BITS+est_group_width,has_undef);
    printf(" complex start %d %d bytes\n", vmx, j);
#endif

    // finished making segments
    // findout number of bytes for extra info (packing_mode 2/3)

    if (packing_mode != 1) {			// packing modes 2/3
	k = vmn >= 0 ? find_nbits(vmn)+1 : find_nbits(-vmn)+1;
	// + 1 work around for NCEP bug
	j = find_nbits(extra_0) + 1;
	if (j > k) k = j;

	if (packing_mode == 3) {
	    // + 1 work around for NCEP bug
	    j = find_nbits(extra_1) + 1;
	    if (j > k) k = j;
	}
	sec5[48] = (k+7)/8;			// number of bytes for extra and vmn
    }

    // scale the linked list
    s = start.tail;
    if (s == NULL) fatal_error("complex grib_out: program error 1","");
    ngroups = 0;				// number  of groups

    while (s) {
	ngroups++;
	s = s->tail;
    }

    lens = (int *) malloc(((size_t) ngroups) * sizeof(int));
    widths = (int *) malloc(((size_t) ngroups) * sizeof(int));
    refs = (int *) malloc(((size_t) ngroups) * sizeof(int));
    itmp = (int *) malloc(((size_t) ngroups) * sizeof(int));
    itmp2 = (int *) malloc(((size_t) ngroups) * sizeof(int));

    if (lens == NULL || widths == NULL || refs == NULL || itmp == NULL || itmp2 == NULL) 
	fatal_error("complex grib_out: memory allocation","");
// printf("linked list ngroups=%d\n", ngroups);
/*
    for (i = k = 0, s = start.tail; k < ngroups; k++, s=s->tail) {
       lens[k] = s->i1 - s->i0 + 1;
       i += lens[k];
       refs[k] = s->mn;
       if (s->mn == INT_MAX) widths[k] = 0;
       else if (s->mn == s->mx) widths[k] = s->missing;
       else widths[k] = find_nbits(s->mx-s->mn+has_undef);
    }
*/

    /* make vectors so we can OpenMP the loop */
    for (i = ii = 0, s = start.tail; ii < ngroups; ii++, s=s->tail) {
       lens[ii] = s->i1 - s->i0 + 1;
       i += lens[ii];
       refs[ii] = s->mn;
       itmp[ii] = s->mx;
       itmp2[ii] = s->missing;
    }
    if (i != nndata) fatal_error("complex grib_out: program error 2","");

#pragma omp parallel for private(i)
    for (i = 0; i < ngroups; i++) {
        if (refs[i] == INT_MAX) widths[i] = 0;
        else if (refs[i] == itmp[i]) widths[i] = itmp2[i];
        else widths[i] = find_nbits(itmp[i]-refs[i]+has_undef);
    }

    // group lengths
    len_last = lens[ngroups-1];			// length of last segment

    glenmn = glenmx = lens[0];
    gwidmx = gwidmn = widths[0];
    grefmx = refs[0] != INT_MAX ? refs[0] : 0;

/*
    for (k = 1; k < ngroups; k++) {
        glenmx = glenmx >= lens[k] ? glenmx : lens[k];
        glenmn = glenmn <= lens[k] ? glenmn : lens[k];
        gwidmx = gwidmx >= widths[k] ? gwidmx : widths[k];
        gwidmn = gwidmn <= widths[k] ? gwidmn : widths[k];
	if (refs[k] != INT_MAX && refs[k] > grefmx) grefmx = refs[k];
    }
*/

#pragma omp parallel private(i)
{
    int glenmn_thread, glenmx_thread, gwidmx_thread, gwidmn_thread, grefmx_thread;
    glenmn_thread = glenmx_thread = lens[0];
    gwidmn_thread = gwidmx_thread = widths[0];
    grefmx_thread = refs[0] != INT_MAX ? refs[0] : 0;

#pragma omp for nowait
    for (i = 1; i < ngroups; i++) { 
        glenmx_thread = glenmx_thread >= lens[i] ? glenmx_thread : lens[i];
        glenmn_thread = glenmn_thread <= lens[i] ? glenmn_thread : lens[i];
        gwidmx_thread = gwidmx_thread >= widths[i] ? gwidmx_thread : widths[i];
        gwidmn_thread = gwidmn_thread <= widths[i] ? gwidmn_thread : widths[i];
	if (refs[i] != INT_MAX && refs[i] > grefmx_thread) grefmx_thread = refs[i];
    }
#pragma omp critical
    {
	glenmx = glenmx >= glenmx_thread ? glenmx : glenmx_thread;
	glenmn = glenmn <= glenmn_thread ? glenmn : glenmn_thread;
        gwidmx = gwidmx >= gwidmx_thread ? gwidmx : gwidmx_thread;
        gwidmn = gwidmn <= gwidmn_thread ? gwidmn : gwidmn_thread;
        grefmx = grefmx >= grefmx_thread ? grefmx : grefmx_thread;
    }
}

    sec5[19] = find_nbits(grefmx+has_undef);

   // sec5 definitions

    uint_char(packing_mode == 1 ? 47 : 49, sec5);	// size of sec5
    sec5[4] = 5;					// section 5
    uint_char(nndata, sec5+5);				// number of points
    if (packing_mode == 1) uint2_char(2,sec5+9);	// data template 2
    else uint2_char(3,sec5+9);				// data template 3

   // same as grid template 5.0

    flt2ieee((float) ref,sec5+11);                       // reference value
    int2_char(binary_scale,sec5+15);                     // binary scaling
    int2_char(-dec_scale,sec5+17);                       // decimal scaling
    sec5[20] = 0;                                        // original = float

   // same as grid template 5.2

    sec5[21] = 1;                           // general group splitting
    sec5[22] = has_undef;		    // primary missing values or no missing values
    flt2ieee((float) 9.999e20,sec5+23);     // missing value
    sec5[27] = sec5[28] = sec5[29] = sec5[30] = 255; // secondary missing value
    uint_char(ngroups,sec5+31);                   // one group

    sec5[35] = gwidmn;                           // group width reference
    sec5[36] = find_nbits(gwidmx-gwidmn+has_undef);  // group width bits
#ifdef DEBUG
printf("group widthmn = %d, gwidmx %d, width bits max %d\n", gwidmn, gwidmx, sec5[36]);
#endif
    uint_char(glenmn,sec5+37);	 	         // group length ref
    sec5[41] = 1;  		                 // inc
    uint_char(len_last,sec5+42);                  // len of last group
    sec5[46] = find_nbits(glenmx-glenmn);            // group length width
    if (packing_mode == 2) sec5[47] = 1;
    if (packing_mode == 3) sec5[47] = 2;

    // calculate the size of the data section

    // basic size
    size_sec7 = 5;

    // extra octets
    if (packing_mode == 2) size_sec7 += 2*sec5[48];
    else if (packing_mode == 3) size_sec7 += 3*sec5[48];
  
    // group reference value
    size_sec7 +=  (ngroups * sec5[19] + 7)/8;

    // group widths
    size_sec7 +=  (ngroups * sec5[36] + 7)/8;

    // group lengths
    size_sec7 +=  (ngroups * sec5[46] + 7)/8;

    // size of packed grid points

    for (i = k = 0; i < ngroups; i++) {
	j  = lens[i] * widths[i] + k;
        size_sec7 += (j >> 3);
	k = (j & 7);
    }
    size_sec7 += k ? 1 : 0;

#pragma omp parallel for private(i)
    for (i = 0; i < ngroups; i++) {
	refs[i] = (refs[i] != INT_MAX) ? refs[i] :  ONES;
        itmp[i] = widths[i] - gwidmn;
        itmp2[i] = lens[i] - glenmn;
    }

    sec7 = (unsigned char *) malloc(size_sec7);
    if (sec7 == NULL) fatal_error("complex_grib_out memory allocation sec7","");

    // pack the values into a bitstream

    init_bitstream(sec7);
    add_bitstream(size_sec7>>16,16);
    add_bitstream(size_sec7,16);
    add_bitstream(7,8);				

    // write extra octets

    if (packing_mode == 2 || packing_mode == 3) {
	add_bitstream(extra_0,8*sec5[48]);
        if (packing_mode == 3) add_bitstream(extra_1,8*sec5[48]);
	k = vmn;
	if (k < 0) {
	    k = -vmn | (1 << (8*sec5[48]-1));
	}
        add_bitstream(k,8*sec5[48]);
        finish_bitstream();
    }

    // write the group reference values
    add_many_bitstream(refs, ngroups, sec5[19]);
    finish_bitstream();

    // write the group widths
    add_many_bitstream(itmp, ngroups, sec5[36]);
    finish_bitstream();

    // write the group lengths
    add_many_bitstream(itmp2, ngroups, sec5[46]);
    finish_bitstream();

    // write the data

/*
    s = start.tail;
    for (ii = 0; ii < ngroups; ii++, s=s->tail) {
	// number of bits to pack
	if (widths[ii]) {
//	    mask = (1 << widths[ii]) - 1;
	    for (j = 0; j < lens[ii]; j++) {
		v[j+s->i0] = (v[j+s->i0] == INT_MAX) ? ONES : v[j+s->i0] - s->mn;
	    }
            add_many_bitstream(v+s->i0, lens[ii], widths[ii]);
	}
    }
*/
    
    s = start.tail;
    for (i = 0; i < ngroups; i++, s=s->tail) {
	itmp[i] = s->i0;
	refs[i] = s->mn;
    }
#pragma omp parallel for private(i,j)
    for (i = 0; i < ngroups; i++) {
	if (widths[i]) {
	    for (j = 0; j < lens[i]; j++) {
		v[j+itmp[i]] = (v[j+itmp[i]] == INT_MAX) ? ONES : v[j+itmp[i]] - refs[i];
	    }
	}
    }
    for (i = 0; i < ngroups; i++) {
	if (widths[i]) {
            add_many_bitstream(v+itmp[i], lens[i], widths[i]);
	}
    }  

    finish_bitstream();

    j = wrt_sec(sec0, sec1, sec2, sec3, sec4, sec5, sec6, sec7, out);

    free(sec5);
    free(sec6);
    free(sec7);

    free(list);
    free(v);
    free(lens);
    free(widths);
    free(refs);
    free(itmp);

    return j;
}
