#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_JASPER

#include <jasper/jasper.h>

/*
 *  writes out jpeg2000 compressed grib message
 */

int jpeg2000_grib_out(unsigned char **sec, float *data, unsigned int ndata, 
  int nx, int ny, int use_scale, int dec_scale, int bin_scale, int wanted_bits, 
  int max_bits, struct seq_file *out) {

    unsigned int n_defined, j;
    int iy, ix, jpclen;
    unsigned char *cdata, *p;
    unsigned char *sec0, *sec1, *sec2 , *sec3, *sec4, *sec5, *sec6, *sec7;
    double fmin, frange, scale, dec_factor;
    float ref, min_val, max_val, ncep_min_val;
    int i, k, nbits, nbytes;

    int ltype, ratio, retry;
    char *outjpc;

    /* required passed sections */
    sec0 = sec[0];
    sec1 = sec[1];
    sec2 = sec[2];
    sec3 = sec[3];
    sec4 = sec[4];

    /* make a sections 5-7 */
    n_defined = ndata;
    sec6 = mk_bms(data, &n_defined);                    // make bitmap section, eliminate undefined grid values

    if ((unsigned int) (nx*ny) == n_defined) {
        ix = nx;
        iy = ny;
    }
    else {
	ix = n_defined;
 	iy = 1;
    }

    if (n_defined != 0) {
	min_max_array_all_defined(data, n_defined, &min_val,  &max_val);
    }
    else {
	min_val = max_val = 0.0;
    }

    ncep_min_val = min_val;

    if (use_scale == 0) {
        /* ecmwf style */
        fmin = min_val;
        frange = max_val - fmin;
        ref = fmin;
	dec_scale = 0;
        if (frange != 0.0) {
            frexp(frange, &i);
            bin_scale = i - wanted_bits;
            nbits = wanted_bits;
            scale = ldexp(1.0, -bin_scale);
            frange = floor((max_val-fmin)*scale + 0.5);
            frexp(frange, &i);
            if (i != nbits) bin_scale++;
        }
        else {
            bin_scale = nbits = 0;
	    scale = 1;
        }
    }
    else {
	if (dec_scale) {
	    dec_factor = Int_Power(10.0, -dec_scale);
	    min_val *= dec_factor;
	    max_val *= dec_factor;
            for (j = 0; j < n_defined; j++) {
                data[j] *= dec_factor;
            }
	}
	ref = min_val;
        scale = ldexp(1.0, -bin_scale);
//      i = (int) ( (max_val - ref)*scale + 0.5);
//	frange = (double) i;
        frange  = floor( (max_val - ref)*scale + 0.5);
        frexp(frange, &nbits);

        if (nbits > max_bits) {
	    bin_scale += (nbits - max_bits);
	    nbits = max_bits;
        }
    }

    if (bin_scale) {
        scale = ldexp(1.0, -bin_scale);
        for (j = 0; j < n_defined; j++) {
            data[j] = (data[j] - ref)*scale;
        }
    }
    else {
        for (j = 0; j < n_defined; j++) {
            data[j] = data[j] - ref;
        }
    }
    if (nbits > 0 && n_defined > 0) {

        nbytes = (nbits + 7) / 8;
        if (nbytes > 4) fatal_error_i("jpeg2000_grib_out number of bytes is %d > 4", nbytes);

        /* Pack integers into bytes */

        cdata = (unsigned char *) malloc(nbytes * (size_t) n_defined);
        if (cdata == NULL) fatal_error("memory alloc jpeg encoding","");
        p = cdata; 
        if (nbytes == 1) {
            for (j = 0; j < n_defined; j++) {
    	        i = (int) floor(data[j]+0.5);
	        *p++ = i & 255;
	    }
        }
        else if (nbytes == 2) {
            for (j = 0; j < n_defined; j++) {
	        i = (int) floor(data[j]+0.5);
	        p[0] = (i >> 8) & 255;
	        p[1] = i & 255;
	        p += 2;
	    }
        }
        else if (nbytes > 0) {
            for (j = 0; j < n_defined; j++) {
	        i = (int) floor(data[j]+0.5);
	        for (k = 1; k <= nbytes; k++) {
	            p[nbytes - k] = i & 255;
		    i = i >> 8;
	        }
	        p += nbytes;
	    }
        }

//    jas_init();
        ltype = 0;
        ratio = 1;
        retry = 0;

        jpclen = 4*n_defined+200;
        outjpc = (char *) malloc(jpclen);

        i = enc_jpeg2000_clone(cdata,ix,iy,nbits,ltype,ratio,retry,outjpc,jpclen);

        // we try to catch following error: "error: too few guard bits (need at least x)"
        if (i == -3) {
            retry = 1;
            i = enc_jpeg2000_clone(cdata,ix,iy,nbits,ltype,ratio,retry,outjpc,jpclen);
        }

	free(cdata);
	if (i <= 0) fatal_error_i("enc_jpeg error %d", i);
    }
    else {   // nbits == 0 || n_defined == 0
	i = 0;
	outjpc = (char *) malloc(1);
    }

    /* data representation section */

    // fix for buggy NCEP library 1/2009
    // NCEP routines ignore decimal scaling with nbits == 0
    if (nbits == 0) {
	ref = ncep_min_val;
	bin_scale = dec_scale = 0;
    }

    sec5 = (unsigned char *) malloc(23 * sizeof(unsigned char));
    if (sec5 == NULL) fatal_error("grib_out jpeg memory allocation sec5","");
    uint_char(23 * sizeof (unsigned char), sec5);
    sec5[4] = 5;                        // section 5
    uint_char(n_defined, sec5+5);       // number of points
    uint2_char(40,sec5+9);              // data template 4 - jpeg2000
    flt2ieee(ref,sec5+11);		// reference value
    int2_char(bin_scale,sec5+15);	// binary scaling
    int2_char(-dec_scale,sec5+17);	// decimal scaling
    sec5[19] = nbits;
    sec5[20] = 0;			// 0 - float 1=int
    sec5[21] = 0;			//code 5.40 -lossless
    sec5[22] = 255;			// undefined
    
    sec7 = (unsigned char *) malloc(i+5);
    if (sec7 == NULL) fatal_error("grib_out jpeg memory allocation sec7","");
    uint_char(i+5, sec7);
    sec7[4] = 7;                        // section 7
    for (k = 0; k < i; k++) sec7[5+k] = outjpc[k];

    i = wrt_sec(sec0, sec1, sec2, sec3, sec4, sec5, sec6, sec7, out);

    free(outjpc);
    free(sec5);
    free(sec6);
    free(sec7);

    return i;
}

#endif
