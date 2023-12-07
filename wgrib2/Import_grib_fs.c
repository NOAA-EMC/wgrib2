#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Import_grib_fs.c
 *
 * 3/2019: Public Domain: Wesley Ebisuzaki
 *         based on Import_grib.c (public domain)
 *
 *  like import_grib but includes a fixed string search
 */

extern int decode, use_g2clib;
extern enum output_order_type output_order, output_order_wanted;

/*
 * HEADER:100:import_grib_fs:misc:2:read grib2 file (Y) sequentially for record that matches X (fixed string)
 */

int f_import_grib_fs(ARG2) {
    unsigned int i;
    unsigned char *msg;
    int j, center, nx, ny, res, scan;
    unsigned int npnts;

    struct local_struct {
        long int pos, submsg;
        unsigned long int len;
        int num_submsg;
        struct seq_file input;
	unsigned char *sec[10];       /* sec[9] = last valid bitmap */
    };
    struct local_struct *save;
 
    if (mode == -1) {
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("import_grib_fs: memory allocation","");
        decode = 1;
	i = fopen_file(&(save->input), arg2, "rb");
	if (i != 0) fatal_error("import_grib_fs: %s could not be opened", arg2);
	save->submsg = 0;
	save->pos = 0;
    }
    else if (mode == -2) {
	save = *local;
	fclose_file(&(save->input));
	free(save);
    }
    else if (mode >= 0) {
	save = *local;

	/* go to the beginning of the file */

        fseek_file(&(save->input), 0L, SEEK_SET);
	save->pos = 0;
	save->submsg = 0;
	save->len = 0;
	save->num_submsg = 0;

	{
	    unsigned char **sec;
	    char inv_out[INV_BUFFER];
	    int mode, match;

	    do {
	        sec = save->sec;
	        inv_out[0] = 0;
	        mode = 0;

	        if (save->submsg == 0) {
	            msg = rd_grib2_msg_seq_file(save->sec, &(save->input), &(save->pos), &(save->len), &(save->num_submsg));
                    if (msg == NULL) fatal_error("import_grib_fs: record not found","");
                    if (parse_1st_msg(save->sec) != 0) fatal_error("import_grib_fs: record not parsed correctly","");
	            save->submsg = (save->num_submsg == 1) ? 0 : 1;
	        }
	        else {
                    if (parse_next_msg(save->sec) != 0) fatal_error("import_grib_fs: record not parsed correctly","");
	            save->submsg = (save->num_submsg == save->submsg+1) ? 0 : save->submsg + 1;
	        }

	        // make match inventory

	        f_match_inv(call_ARG0(inv_out, NULL));
	        match = strstr(inv_out, arg1) != NULL;

		// make -s inventory for stderr

		if (match) f_s(call_ARG0(inv_out, NULL));
	    } while (match == 0);

	    /* want to get some ouput that help use determine right field read */
	    fprintf(stderr,"(import_grib_fs:%s)\n", inv_out);
	}

	/* save->sec[] is defined */
        get_nxny(save->sec, &nx, &ny, &npnts, &res, &scan);

        if (npnts != ndata) 
             fatal_error_uu("import_grib_fs: size mismatch (%u/%u)", npnts, ndata); 

        if (use_g2clib != 0 && use_g2clib != 1)
             fatal_error_i("import_grib_fs: only g2clib = 0 or 1 supported (%d)", use_g2clib);

        if (use_g2clib == 1) {  // introduce g2clib constant field error
            /* g2clib ignores decimal scaling for constant fields make internal decoders look like g2clib*/
            center = GB2_Center(save->sec);
            j = code_table_5_0(save->sec);            // type of compression
            if ( (j == 0 && save->sec[5][19] == 0) || ((j == 2 || j == 3) && int4(save->sec[5] + 31) == 0) ||
                 (j == 40 && save->sec[5][19] == 0) || (j == 41 && save->sec[5][19] == 0) ||
                 (center == NCEP && j == 40000 && save->sec[5][19] == 0) ||
                 (center == NCEP && j == 40010 && save->sec[5][19] == 0)  ) {
                        save->sec[5][17] = save->sec[5][18] = 0;
            }
        }

        if (unpk_grib(save->sec, data)) fatal_error("import_grib_fs: unpk_grib","");

        /* convert to standard output order we:sn */

        if (output_order_wanted == wesn) to_we_sn_scan(data,scan,npnts,nx,ny,0);
        else if (output_order_wanted == wens) to_we_ns_scan(data,scan,npnts,nx,ny,0);

    }
    return 0;
}
