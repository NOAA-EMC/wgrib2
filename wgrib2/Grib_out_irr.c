#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Grib_out_irr
 *
 * 10/2011 Public Domain by Wesley Ebisuzaki
 *
 */

extern int latlon, decode;
extern double *lat, *lon;
extern int file_append, flush_mode;
extern enum output_order_type output_order;
extern int use_scale, dec_scale, bin_scale, max_bits, wanted_bits;
extern enum output_grib_type grib_type;

/*
 * HEADER:100:grib_out_irr:output:2:writes irregular grid grib  (GDT=130 not adopted) X=(all|defined) Y=(output file)
 */

int f_grib_out_irr(ARG2) {

    float *data_tmp;
    int all, j;
    unsigned int i, n;
    unsigned char *gds, *old_gds, *p;
    struct seq_file *save;

    if (mode == -1) {
        latlon = decode = 1;
	if (strcmp(arg1,"defined") && strcmp(arg1,"all")) fatal_error("grib_out_irr: %s should be all or defined", arg1);
	*local = save = (struct seq_file *) malloc(sizeof(struct seq_file));
	if (save == NULL) fatal_error("grib_out_irr: memory allocation","");
        if (fopen_file(save, arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
	return 0;
    }
    save = (struct seq_file *) *local;

    if (mode == -2) {
	fclose_file(save);
        free(save);
	return 0;
    }

    if (lat == NULL || lon == NULL) fatal_error("grid_out_irr: failed, no lat-lon information","");

    all = strcmp(arg1,"all") == 0;
    if (all) {
	n = ndata;
    }
    else {
	for (i = n = 0; i < ndata; i++) {
	    if (DEFINED_VAL(data[i])) n++;
	}
    }

    if (n == 0) {
	fprintf(stderr,"grib_out_irr: no grid points to write out, no write\n");
	return 0;
    }

    if ( (30 + 8 * (size_t) n) != (size_t) (30 + 8 * n)) 
	fatal_error("grib_out_irr: too many grid points, sec3 overflow","");

    if ((gds = (unsigned char *) malloc(30 + 8 * (size_t) n)) == NULL) fatal_error("grib_out_irr: memory allocation","");

    if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) n)) == NULL) fatal_error("grib_out_irr: memory allocation","");

    /* sec3 = grid defintion */
    uint_char(30+n*8, gds);
    gds[4] = 3;            // sec3
    gds[5] = 0;            // use table 3.1
    uint_char(n, gds+6);
    gds[10] = 0;           // no optional list octets
    gds[11] = 0;
    uint2_char(130, gds+12);

    p = code_table_3_2_location(sec);
    if (p == NULL) {  // no earth descripition
        for (i = 14; i < 30; i++) {
            gds[i] = 255;
        }
    }
    else {
        for (i = 14; i < 30; i++) {
            gds[i] = p[i-14];
        }
    }

    if (all) {
	for (i = 0; i < ndata; i++) {
	    int_char( (int) (lat[i] * 1000000.0), gds + 30 + i*8);
	    int_char( (int) (lon[i] * 1000000.0), gds + 34 + i*8);
	    data_tmp[i] = data[i];
	}
    }
    else {
	for (j = i = 0; i < ndata; i++) {
	    if (DEFINED_VAL(data[i])) {
	        int_char( (int) (lat[i] * 1000000.0), gds + 30 + j*8);
	        int_char( (int) (lon[i] * 1000000.0), gds + 34 + j*8);
	        data_tmp[j++] = data[i];
	    }
	}
    }

    old_gds = sec[3];
    sec[3] = gds;
    grib_wrt(sec, data_tmp, n, n, 1, use_scale, dec_scale, 
		bin_scale, wanted_bits, max_bits, grib_type, save);
    sec[3] = old_gds;
    if (flush_mode) fflush_file(save);

    free(data_tmp);
    free(gds);

    return 0;
}
