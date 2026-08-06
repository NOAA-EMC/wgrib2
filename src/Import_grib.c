/** @file 
 * @brief Read from GRIB2 file for data.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 12/2014 | W. Ebisuzaki | Initial
 * 8/2021 | W. Ebisuzaki | Fix NCEP scaling problem
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 12/2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Use G2C library flag. */
extern int use_g2clib;

/** Current output order type. */
extern enum output_order_type output_order;

/** Desired output order type. */
extern enum output_order_type output_order_wanted;

/*
 * HEADER:100:import_grib:misc:1:read grib2 file (X) for data
 */

/**
 * Import data from GRIB2 file.
 * 
 * Wgrib2 will decode the grib message and save the decoded grid point values in a floating 
 * point array (DATA). The -import options read grid point values from a specified file and 
 * replace the values of DATA. The size of DATA and imported grid should match. The -import 
 * options are often used to read data that is later written out as a grib message. 
 * 
 * Note that the import functions will reset the scaling and precision of the grib writing 
 * (new files) to the default (ECMWF-style, 12 bits). Any -set_metadata should be done after 
 * the -import functions. 
 * 
 * ## Scan Order
 * The grib message's scan order is called the "input" scan order (wgrib2 -grid). Wgrib2 
 * converts this to the "output" scan order. (This is the scan order for options like: -bin, 
 * -text, -cvs.) The import file needs to be in the "output" scan order. Of course, you can 
 * change the output scan order using the -order option. 
 * 
 * ## Import Format
 * The file that you import needs to be in a special format. 
 * 
 * - grib: grib2 message
 * - bin: native single point format
 * - ieee: IEEE single point format
 * - bin: may have a f77 style header depending on the -header option
 * - ieee: may have a f77 style header depending on the -header option
 * - ieee: may be little or big (default) ending depending on options
 * - text: may have a "nx ny" header depending on the -header option (see -text option)
 * 
 * ## Usage
 * -import_grib FILE
 * 
 * FILE is the grib2 file to read.
 * 
 * -g2clib 2 is not supported.
 * 
 * Conversion to we:sn and we:ns is supported.
 * 
 * Reads next grid (message or submessage)
 * 
 * -not and -match do not affect -import_grib
 * 
 * @note Grid size (if it can be determined) must match the current grid.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2014
 */
int f_import_grib(ARG1) {
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
        if (save == NULL) fatal_error("import_grib: memory allocation","");
        decode = 1;
        i = fopen_file(&(save->input), arg1, "rb");
        if (i != 0) fatal_error("import_grib: %s could not be opened", arg1);
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
        if (save->submsg == 0) {
            msg = rd_grib2_msg_seq_file(save->sec, &(save->input), &(save->pos), &(save->len), &(save->num_submsg));
            if (msg == NULL) fatal_error("import_grib: record not found","");
            if (parse_1st_msg(save->sec) != 0) fatal_error("import_grib: record not parsed correctly","");
            save->submsg = (save->num_submsg == 1) ? 0 : 1;
        }
	    else {
            if (parse_next_msg(save->sec) != 0) fatal_error("import_grib: record not parsed correctly","");
            save->submsg = (save->num_submsg == save->submsg+1) ? 0 : save->submsg + 1;
        }

        /* save->sec[] is defined */
        get_nxny(save->sec, &nx, &ny, &npnts, &res, &scan);

        if (npnts != ndata) 
            fatal_error_uu("import_grib: size mismatch (%u/%u)", npnts, ndata); 

        if (use_g2clib != 0 && use_g2clib != 1)
            fatal_error_i("import_grib: only g2clib = 0 or 1 supported (%d)", use_g2clib);

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

        if (unpk_grib(save->sec, data)) fatal_error("import_grib: unpk_grib","");

        /* convert to standard output order we:sn */

        if (output_order_wanted == wesn) to_we_sn_scan(data,scan,npnts,nx,ny,0);
        else if (output_order_wanted == wens) to_we_ns_scan(data,scan,npnts,nx,ny,0);

    }
    return 0;
}
