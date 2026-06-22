/** @file
 * @brief Pack GRIB2 data with simple packing.
 * @author Public Domain: Wesley Ebisuzaki @date 2005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"

#ifdef USE_OPENMP
#include <omp.h>
#else
/** Default number of threads if USE_OPENMP = OFF */
#define omp_get_num_threads()		1
#endif

/**
 * Write a GRIB2 file using simple packing.
 *
 * @param sec Pointer to the GRIB2 sections. Sections 0 to 4 are predefined.
 * @param data Pointer to array of values to encode into GRIB2.
 * @param ndata Number of data values.
 * @param use_scale Use scaling flag.
 * @param dec_scale Decimal scaling factor.
 * @param bin_scale Binary scaling factor.
 * @param wanted_bits Number of bits wanted for packing.
 * @param max_bits Maximum number of bits allowed for packing.
 * @param out Pointer to output file structure.
 *
 * @return 0 on success, 1 on failure.
 *
 * @author Wesley Ebisuzaki @date 2005
 */
int simple_grib_out(unsigned char **sec, float *data, unsigned int ndata, 
        int use_scale, int dec_scale, int bin_scale, int wanted_bits, 
        int max_bits, struct seq_file *out) {

    unsigned int n_defined;
    int i;
    unsigned char *sec0, *sec1, *sec2 , *sec3, *sec4, *sec5, *sec6, *sec7;

    /* required passed sections */
    sec0 = sec[0];
    sec1 = sec[1];
    sec2 = sec[2];
    sec3 = sec[3];
    sec4 = sec[4];
    /* make a sections 5-7 */
    n_defined = ndata;
    sec6 = mk_bms(data, &n_defined);			// make bitmap section

    mk_sec5and7(data, n_defined, &sec5, &sec7,use_scale,dec_scale,bin_scale, 
        wanted_bits, max_bits);		// make sec 5 and 7
    i = wrt_sec(sec0, sec1, sec2, sec3, sec4, sec5, sec6, sec7, out);

    free(sec5);
    free(sec6);
    free(sec7);

    return i;
}

/*
 * make sec 5 and 7 using simple packing
 */

/**
 * Make section 5 and 7 using simple packing.
 *
 * @param data Pointer to array of values to encode into GRIB2.
 * @param n Number of data values.
 * @param sec5 Pointer to store the created section 5.
 * @param sec7 Pointer to store the created section 7.
 * @param use_scale Use scaling flag.
 * @param dec_scale Decimal scaling factor.
 * @param bin_scale Binary scaling factor.
 * @param wanted_bits Number of bits wanted for packing.
 * @param max_bits Maximum number of bits allowed for packing.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 */
int mk_sec5and7(float *data, unsigned int n, unsigned char **sec5, unsigned char **sec7, 
        int use_scale, int dec_scale, int bin_scale, int wanted_bits, int max_bits) {

    float min_val, max_val, ncep_min_val;
    int nbits, binary_scale, j;
    double ref, frange, scale, dec_factor;
    size_t sec5_size, sec7_size;
    unsigned char *p;
    unsigned int i, k, di;

/*    int nthreads; */

    binary_scale = bin_scale;
    if (n == 0) {				// all undefined
        nbits = 0;
        ref = ncep_min_val = 0.0;
    }
    else {
        min_max_array_all_defined(data, n, &min_val, &max_val);

        ncep_min_val = min_val;
        if (use_scale == 0) {
            /* ecmwf style */
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
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
                for (i = 0; i < n; i++) {
                    data[i] *= dec_factor;
                }
            }

            scale = ldexp(1.0, -binary_scale);
            ref = min_val;
            frange = floor ((max_val - ref)*scale + 0.5);
            frexp(frange, &nbits);
            if (nbits > max_bits) {
                binary_scale += (nbits - max_bits);
                nbits = max_bits;
            }
        }

        /* scale data by ref, binary_scale and dec_scale */
        if (binary_scale) {
            scale = ldexp(1.0, -binary_scale);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
            for (i = 0; i < n; i++) {
                data[i] = (data[i] - ref)*scale;
            }
        }
        else {
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
            for (i = 0; i < n; i++) {
                data[i] = data[i] - ref;
            }
        }
    }
    sec5_size = 21;
    sec7_size = 5 + (nbits * (n / 8)) + (nbits * (n % 8) + 7) / 8;

    // section 7
    *sec7 = p = (unsigned char *) malloc(sec7_size);
    if (p == NULL) fatal_error("mk_sec5and7: memory allocation","");
    uint_char(sec7_size, p);
    p[4] = 7;

    if (n != 0) {
//      single thread version
//        flist2bitstream(data,p + 5,n,nbits);

//      flist2bitstream can run in parallel if the loop has
//      increments of 8.  Then each conversion to a bitstream
//      starts on a byte boundary.  
//

        /* can parallized if di is a multiple of 8,  large di reduces flist2bits overhead
        * schedule static in order to reduce false sharing
        * di should be long to reduce subroutine overhead
        */
        di = 128;		/* multiple of 8 */
#ifdef USE_OPENMP
#pragma omp parallel for private(i,k) schedule(static)
#endif
        for (i = 0; i < n; i+= di) {
            k  = n - i;
            if (k > di) k = di;
            flist2bitstream(data + i, p + 5 + (i/8)*nbits, k, nbits);
        }


/* this version has problems with oneAPI
#pragma omp parallel private(i,k)
        {
#pragma omp single
            {
                nthreads = omp_get_num_threads();
	        di = (n + nthreads - 1) / nthreads;
   	        di = ((di + 7) | 7) ^ 7;
            }
#pragma omp for
      	    for (i = 0; i < n; i+= di) {
	        k  = n - i;
	        if (k > di) k = di;
                flist2bitstream(data + i, p + 5 + (i/8)*nbits, k, nbits);
	    }
        }
*/
    }

    // section 5

    // fix for buggy NCEP decoders
    // for constant fields, they ignore the decimal scaling
    if (nbits == 0) {
        dec_scale = binary_scale = 0;
        ref = ncep_min_val;
    }

    *sec5 = p = (unsigned char *) malloc(sec5_size);
    if (p == NULL) fatal_error("mk_sec5and7: memory allocation","");
    uint_char(sec5_size, p);		// length of section 5
    p[4] = 5;				// section 5
    uint_char(n, p+5);			// number of defined points
    uint2_char(0,p+9);			// template 5.0
    flt2ieee(ref,p+11);			// ieee reference value
    int2_char(binary_scale,p+15);
    int2_char(-dec_scale,p+17);
    p[19] = nbits;
    p[20] = 0;				// template 5.1 - set to floating

    return 0;
}

