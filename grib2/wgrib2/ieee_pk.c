#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * write grib-2 ieee file
 * public domain 2008 Wesley Ebisuzaki
 */

int ieee_grib_out(unsigned char **sec, float *data, unsigned int ndata, struct seq_file *out) {

    unsigned int n_defined, i;
    int j;
    unsigned char *p, *sec0, *sec1, *sec2, *sec3, *sec4, *sec5, *sec6, *sec7;
#ifdef IEEE_BITMAP
    float *data_tmp;
#endif

    /* required passed sections */
    sec0 = sec[0];
    sec1 = sec[1];
    sec2 = sec[2];
    sec3 = sec[3];
    sec4 = sec[4];

    /* make a new section 6 */

#ifdef IEEE_BITMAP
    data_tmp = (float *) malloc(((size_t) ndata) * sizeof(float));
    for (i = 0; i < ndata; i++) {
	data_tmp[i] = data[i];
    }
    n_defined = ndata;
    sec6 = mk_bms(data_tmp, &n_defined);			// make bitmap section
    if (sec6 == NULL) fatal_error("grib_out ieee memory allocation sec6","");
#else
    n_defined = ndata;
    sec6 = (unsigned char *) malloc(6);
    if (sec6 == NULL) fatal_error("grib_out ieee memory allocation sec6","");
    uint_char(6 * sizeof (unsigned char), sec6);
    sec6[4] = 6;			// section 5
    sec6[5] = 255;			// no bitmap
#endif
    
    /* data representation section */

    sec5 = (unsigned char *) malloc(12 * sizeof(unsigned char));
    if (sec5 == NULL) fatal_error("grib_out ieee memory allocation sec5","");
    uint_char(12 * sizeof (unsigned char), sec5);
    sec5[4] = 5;			// section 5
    uint_char(ndata, sec5+5);		// number of points
    uint2_char(4,sec5+9);		// data template 4
    sec5[11] = 1;			// precision: ieee 32-bit


    /* data section */
    i = (unsigned int) (4 * (size_t) n_defined);
    if (i != (4 * (size_t) n_defined)) 
	fatal_error("ieee_pk: grib2 data section is limited to 4G bytes","");
    sec7 = (unsigned char *) malloc(5 + ((size_t) n_defined) * 4);
    if (sec7 == NULL) fatal_error("ieee_pk: memory allocation sec7","");
    uint_char(5 + 4 * (size_t) n_defined, sec7);
    sec7[4] = 7;
    p = sec7 + 5;
    j = 0;

#pragma omp parallel for private(i) schedule(static)
    for (i = 0; i < n_defined; i++) {
#ifdef IEEE_BITMAP
	flt2ieee_nan(data_tmp[i], p + (i<<2) );
#else
	flt2ieee_nan(data[i], p + (i<<2) );
#endif
    } 

#ifdef IEEE_BITMAP
    free(data_tmp);
#endif

    j = wrt_sec(sec0, sec1, sec2, sec3, sec4, sec5, sec6, sec7, out);

    free(sec5);
    free(sec6);
    free(sec7);

    return j;
}
