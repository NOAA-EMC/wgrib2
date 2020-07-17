#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Grib_out_irr2
 *
 * 7/2017 Public Domain by Wesley Ebisuzaki
 *
 * writes data using grid definition template 101
 * note: data size of input and output do not have to match to
 * allow people to write out GDT 101 unstructured grids
 *
 * note: Y=grid no
 *         if grid is not defined by center, use -1
 * note: Z=grid ref
 *         use -1 for no reference number
 * note: A=UUID
 *         use uuidgen (linux) to generate unique uuid
 */

extern int decode;
extern int file_append, flush_mode;
extern enum output_order_type output_order;
extern int use_scale, dec_scale, bin_scale, max_bits, wanted_bits;
extern enum output_grib_type grib_type;

/*
 * HEADER:100:grib_out_irr2:output:5:writes irregular grid grib  GDT 101 X=npnts Y=grid_no Z=grid_ref A=UUID B=(output file)
 */

int f_grib_out_irr2(ARG5) {

    float *data_tmp;
    int table_3_2;
    unsigned int i, n;
    struct seq_file *save;
    unsigned char gdt101[35], *g, *old_gds;

    if (mode == -1) {
        decode = 1;
	*local = save = (struct seq_file *) malloc(sizeof(struct seq_file));
	if (save == NULL) fatal_error("grib_out_irr: memory allocation","");
        if (fopen_file(save, arg5, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("grib_out_irr2: Could not open %s", arg3);
        }
	return 0;
    }

    save = (struct seq_file *) *local;
    if (mode == -2) {
	fclose_file(save);
        free(save);
	return 0;
    }

    n = (unsigned int) strtoul(arg1,NULL,10);

    g = &(gdt101[0]);
    uint_char(35, g);				/* length of Section 3 */
    g += 4;
    *g++ = 3;					/* section 3 */
    *g++ = 0;					/* source of grid definition */
    uint_char(n, g);				/* number of data points */
    g += 4;
    *g++ = 0;
    *g++ = 0;					/* table 3.11 */
    uint2_char(101,g);				/* template number */
    g += 2;

    table_3_2 = code_table_3_2(sec);
    if (table_3_2 == 1 || table_3_2 == 3 || table_3_2 == 7) 
	fatal_error("grib_out_irr2: user-defined earth shape is not supported by GDT 101","");
    *g++ = (unsigned char) table_3_2;

    i = atoi(arg2);				/* grid number */
    *g++ = (i >> 16) & 255;
    *g++ = (i >> 8) & 255;
    *g++ = i & 255;

    i = atoi(arg3);
    *g++ = (unsigned char) i;			/* number of grid in reference */

    /* UUID */
    if (strcmp(arg4,"0") == 0) {		/* nill UUID */
        for (i = 0; i < 16; i++) *g++ = 0;
    }
    else {
	i = sscanf(arg4, "%2hhx%2hhx%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
            g+0, g+1, g+2, g+3, g+4, g+5, g+6, g+7, g+8, g+9, g+10, g+11, g+12, g+13, g+14, g+15);
	if (i != 16) fatal_error("grib_out_irr2: bad uuid %s",arg4);
	g += 16;
    }

    data_tmp = (float *) malloc(n  * sizeof(float));
    if (data_tmp == NULL) fatal_error("grib_out_irr2: memory allocation","");
    if (ndata != n) fprintf(stderr,"grib_out_irr2: Warning data size=%u, requested size=%u\n", ndata, n);
    if (ndata >= n) {
	for (i = 0; i < n; i++) data_tmp[i] = data[i];
    }
    else {
	for (i = 0; i < ndata; i++) data_tmp[i] = data[i];
        for (i = ndata; i < n; i++) data_tmp[i] = UNDEFINED;
    }

    old_gds = sec[3];
    sec[3] = &(gdt101[0]);
    grib_wrt(sec, data_tmp, n, n, 1, use_scale, dec_scale, 
		bin_scale, wanted_bits, max_bits, grib_type, save);
    sec[3] = old_gds;
    if (flush_mode) fflush_file(save);

    free(data_tmp);
    return 0;
}
