#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Grib_out
 *
 * routines to encode data into grib2
 *
 * 12/2007 Public Domain by Wesley Ebisuzaki
 * 05/2016 Public domain DWD
 *
 */

extern int decode, nx, ny, scan;
extern int file_append, flush_mode;
extern enum output_order_type output_order;
extern int use_scale, input_scale, dec_scale, bin_scale, max_bits, wanted_bits;
extern int save_translation;
extern enum output_grib_type grib_type;
extern int use_bitmap;

/*
 * HEADER:100:set_grib_type:misc:1:set grib type = jpeg, simple, ieee, complex(1|2|3), aec, same
 */

int f_set_grib_type(ARG1) {
    int pack;
    if (strcmp(arg1,"simple") == 0 || strcmp(arg1,"s") == 0) grib_type = simple;
#ifdef USE_JASPER
    else if(strcmp(arg1,"jpeg") == 0 || strcmp(arg1,"j") == 0) grib_type = jpeg;
#endif
    else if (strcmp(arg1,"ieee") == 0 || strcmp(arg1,"i") == 0) grib_type = ieee_packing;
    else if (strcmp(arg1,"complex1") == 0 || strcmp(arg1,"c1") == 0) grib_type = complex1;
    else if (strcmp(arg1,"complex2") == 0 || strcmp(arg1,"c2") == 0) grib_type = complex2;
    else if (strcmp(arg1,"complex3") == 0 || strcmp(arg1,"c3") == 0) grib_type = complex3;

    else if (strcmp(arg1,"complex1-bitmap") == 0 || strcmp(arg1,"c1b") == 0) {
	grib_type = complex1;
	use_bitmap = 1;
    }
    else if (strcmp(arg1,"complex2-bitmap") == 0 || strcmp(arg1,"c2b") == 0) {
	grib_type = complex2;
	use_bitmap = 1;
    }
    else if (strcmp(arg1,"complex3-bitmap") == 0 || strcmp(arg1,"c3b") == 0) {
	grib_type = complex3;
	use_bitmap = 1;
    }

#ifdef USE_AEC
    else if (strcmp(arg1,"aec") == 0 || strcmp(arg1,"a") == 0) grib_type = aec;
#endif
    else if (strcmp(arg1,"same") == 0) {
	if (mode >= 0) {
            pack = code_table_5_0(sec);
	    if (pack == 0) grib_type = simple;
	    else if (pack == 2) grib_type = complex1;
	    else if (pack == 3) {
	        if (code_table_5_6(sec) == 1) grib_type = complex2;
	        else grib_type = complex3;
	    }
	    else if (pack == 4) grib_type = ieee_packing;
#ifdef USE_JASPER
	    else if (pack == 40) grib_type = jpeg;
#endif
#ifdef USE_AEC
	    else if (pack == 42) grib_type = aec;
#endif
	    // cannot duplicate output grib type
	    else grib_type = complex1;
	}
	else grib_type = complex1;
    }
    else fatal_error("set_grib_type: bad type %s", arg1);
    return 0;
}

/*
 * HEADER:100:set_bitmap:misc:1:use bitmap when creating complex packed files X=1/0
 */
int f_set_bitmap(ARG1) {
    if (mode >= -1) {
	use_bitmap = atoi(arg1);
    }
    return 0;
}
 
/*
 * HEADER:100:grib_out:output:1:writes decoded/modified data in grib-2 format to file X
 */

int f_grib_out(ARG1) {

    float *data_tmp;
    struct seq_file *save;

    if (mode == -1) {
        save_translation = decode = 1;

        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("memory allocation grib_out","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
	save = *local;
	fclose_file(save);
    }
    else if (mode >= 0) {
	save = *local;
	if ((data_tmp = (float *) malloc(((size_t) ndata) * sizeof(float))) == NULL)
	 	fatal_error("memory allocation - data_tmp","");
        undo_output_order(data, data_tmp, ndata);

        grib_wrt(sec, data_tmp, ndata, nx, ny, use_scale, dec_scale, 
		bin_scale, wanted_bits, max_bits, grib_type, save);
        if (flush_mode) fflush_file(save);
	free(data_tmp);
    }
    return 0;
}

/*
 * grib_wrt - writes out grib file
 *
 * under some conditions, data will be changed
 */

int grib_wrt(unsigned char **sec, float *data, unsigned int ndata, unsigned int nx, unsigned int ny, int use_scale, int dec_scale, 
	int bin_scale, int wanted_bits, int max_bits, enum output_grib_type grib_type, struct seq_file *out) {

    if (grib_type == simple) simple_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, max_bits, out); 
    else if (grib_type == ieee_packing) ieee_grib_out(sec, data, ndata, out);
#ifdef USE_JASPER
    else if (grib_type == jpeg) jpeg2000_grib_out(sec, data, ndata, nx, ny, use_scale, dec_scale, bin_scale, wanted_bits, max_bits, out);
#endif
    else if (grib_type == complex1) complex_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, 
	max_bits, 1, use_bitmap, out); 
    else if (grib_type == complex2) complex_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, 
	max_bits, 2, use_bitmap, out); 
    else if (grib_type == complex3) complex_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, 
	max_bits, 3, use_bitmap, out); 
#ifdef USE_AEC
    else if (grib_type == aec) aec_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, max_bits, out);
#endif
    return 0;
}

/*
 * HEADER:100:set_grib_max_bits:misc:1:sets scaling so number of bits does not exceed N in (new) grib output
 */

int f_set_grib_max_bits(ARG1) {
    int i;

    i = atoi(arg1);
    if (mode >= -1) {
	if (i < 0) fatal_error_i("set_grib_max_bits: %d is less than zero", i);
	if (i > 25) fatal_error_i("set_grib_max_bits: %d > 25", i);
    }
    if (mode >= -1) max_bits = i;
    return 0;
}
/*
 * HEADER:100:grib_max_bits:inv:0:maximum bits used in grib encoding
 */
int f_grib_max_bits(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,"grib_max_bits=%d", max_bits);
    }
    return 0;
}

/*
 * based on mk_BMS (from gribw)
 *
 * public domain 12/2007 Wesley Ebisuzaki
 *
 * note: data[] is changed (undefined values are eliminated)
 */

unsigned char *mk_bms(float *data, unsigned int *ndata) {

    int bms_size;
    unsigned char *bms, *cbits;
    unsigned int nn, i, start, c, imask, i0;

    nn = *ndata;

    /* find first grid point with undefined data */
    for (i = 0; i < nn; i++) {
	if (UNDEFINED_VAL(data[i])) break;
    }

    if (i == nn) {				/* all defined values, no need for bms */
	bms = (unsigned char *) malloc(6);
	if (bms == NULL) fatal_error("mk_bms: memory allocation problem","");
	uint_char(6, bms);		// length of section 6
	bms[4] = 6;			// section 6
	bms[5] = 255;			// no bitmap
        return bms;
    }

    bms_size = 6 + (nn+7) / 8;
    bms = (unsigned char *) malloc(bms_size);
    if (bms == NULL) fatal_error("mk_bms: memory allocation problem","");

    uint_char(bms_size, bms);		// length of section 6
    bms[4] = 6;				// section 6
    bms[5] = 0;				// has bitmap

    /* bitmap is accessed by bytes, make i0=i/8 bytes of bitmap */
    cbits = bms + 6;
    i0 = i >> 3;
    for (i = 0; i < i0; i++) {
	*cbits++ = 255;
    }

    /* start processing data, skip i0*8 */

    c = 0;
    imask = 128;
    i0 = i0 << 3;
    start = i0;
    for (i = i0; i < nn; i++) {
	if (DEFINED_VAL(data[i])) {
	    c += imask;
	    data[start++] = data[i];
	}
	if ((imask >>= 1) == 0) {
	    *cbits++ = c;
	    c = 0;
	    imask = 128;
	}
    }
    if (imask != 128) *cbits = c;
    *ndata = start;
    return bms;
}


/*
 * HEADER:100:set_bin_prec:misc:1:X use X bits and ECMWF-style grib encoding
 */

int f_set_bin_prec(ARG1) {
    int i;

    if (mode >= -1) {
        i = atoi(arg1);
	if (i < 0) i = 0;
	if (i > max_bits) i = max_bits;
	wanted_bits = i;
	use_scale = 0;
    }
    return 0;
}

/*
 * HEADER:100:precision:inv:0:precision of packing
 */

int f_precision(ARG0) {
    if (mode >= 0) {
	if (use_scale == 0) sprintf(inv_out, "encode %d bits", wanted_bits);
	else sprintf(inv_out,"encode i*2^%d*10^%d", bin_scale, dec_scale);
    }
    return 0;
}


/*
 * HEADER:100:set_scaling:misc:2:set decimal scaling=X/same binary scaling=Y/same  new grib messages
 *
 * if arg1/arg2 == "same" then use the same value as input or -set_scaling (if after input)
 */

int f_set_scaling(ARG2) {

    struct local_struct {
        int dec, bin;
	int use_previous_dec, use_previous_bin;
    };
    struct local_struct *save;

    if (mode == -1) {
        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("set_scaling: memory allocation ","");

	save->use_previous_dec = (strcmp(arg1,"same") == 0);
	save->use_previous_bin = (strcmp(arg2,"same") == 0);
        save->dec = atoi(arg1);
        save->bin = atoi(arg2);
        use_scale = 1;
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
	/* set scaling on,  use_scale = 1
           either keep old values or reset to new values
         */
        use_scale = 1;
       	if (save->use_previous_dec == 0) dec_scale = save->dec;
       	if (save->use_previous_bin == 0) bin_scale = save->bin;
	/* if previous values are bad then  input_scale = 0
           if previous values are used then don't use scaling
         */
	if (input_scale == 0 && (save->use_previous_dec || save->use_previous_bin)) use_scale = 0;

    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
	free(save);
    }
    return 0;
}


/*
 * grib: convert linear list of ints to a bitstream
 *
 * in public domain : 2007 Wesley Ebisuzaki
 */


static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};

void flist2bitstream(float *list, unsigned char *bitstream, unsigned int ndata, int nbits) 
{

    int cbits, jbits;
    unsigned int j, c;

    if (nbits == 0) {
	return;
    }
    if (nbits < 0) fatal_error_i( "flist2bitstream nbits < 0!  nbits = %d", nbits);

    cbits = 8;
    c = 0;
    while (ndata-- > 0) {
	/* note float -> unsigned int .. truncate */
        j = (unsigned int) (*list++ + 0.5);
	jbits = nbits;
	while (cbits <= jbits) {
	    if (cbits == 8) {
	        jbits -= 8;
	        *bitstream++ = (j >> jbits) & 255;
	    }
	    else {
	        jbits -= cbits;
	        *bitstream++ = (c << cbits) + ((j >> jbits) & mask[cbits]);
		cbits = 8;
	        c = 0;
	    }
	}
	/* now jbits < cbits */
	if (jbits) {
	    c = (c << jbits) + (j & mask[jbits]);
	    cbits -= jbits;
	}
    }
    if (cbits != 8) *bitstream++ = c << cbits;
}

int set_order(unsigned char **sec, enum output_order_type order) {
    int flag_3_4;

    if (order == raw) return 0;
    flag_3_4 = flag_table_3_4(sec) & 15;
    if (order == wesn) return set_flag_table_3_4(sec, flag_3_4 | (4 << 4));
    if (order == wens) return set_flag_table_3_4(sec, flag_3_4);
    return 1;
}

/*
 * write sections as a grib message
 */

int wrt_sec(unsigned const char *sec0, unsigned const char *sec1, unsigned const char *sec2, 
    unsigned const char *sec3, unsigned const char *sec4, unsigned const char *sec5, 
    unsigned const char *sec6, unsigned const char *sec7, struct seq_file *file) {

    size_t size;
    unsigned char s[16];
    int i;
    unsigned int s1, s2, s3, s4, s5, s6, s7, s8;
   
    s1 = (sec1 ? uint4(sec1) : 0);
    s2 = (sec2 ? uint4(sec2) : 0);
    s3 = (sec3 ? uint4(sec3) : 0);
    s4 = (sec4 ? uint4(sec4) : 0);
    s5 = (sec5 ? uint4(sec5) : 0);
    s6 = (sec6 ? uint4(sec6) : 0);
    s7 = (sec7 ? uint4(sec7) : 0);
    s8 = GB2_Sec8_size;

    size = (size_t) GB2_Sec0_size + s1 + s2 + s3 + s4 + s5 + 
      (size_t) s6 + (size_t) s7 + s8;

    for (i = 0; i < 8; i++) s[i] = sec0[i];
    uint8_char(size, s+8);
    fwrite_file((void *) s, sizeof(char), 16, file);

    if (sec1) {
	if (fwrite_file((void *)sec1, sizeof(char), s1, file) != s1) return 1;
    }
    if (sec2) {
	if (fwrite_file((void *)sec2, sizeof(char), s2, file) != s2) return 1;
    }
    if (sec3) {
	if (fwrite_file((void *)sec3, sizeof(char), s3, file) != s3) return 1;
    }
    if (sec4) {
	if (fwrite_file((void *)sec4, sizeof(char), s4, file) != s4) return 1;
    }
    if (sec5) {
	if (fwrite_file((void *)sec5, sizeof(char), s5, file) != s5) return 1;
    }
    if (sec6) {
	if (fwrite_file((void *)sec6, sizeof(char), s6, file) != s6) return 1;
    }
    if (sec7) {
	if (fwrite_file((void *)sec7, sizeof(char), s7, file) != s7) return 1;
    }
    s[0] = s[1] = s[2] = s[3] = 55; /* s = "7777" */
    if (fwrite_file((void *) s, sizeof(char), 4, file) != 4) return 1;
    return 0;
}
