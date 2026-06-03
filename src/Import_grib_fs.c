/** @file
 * @brief Read GRIB2 files with fixed string search for data.
 * 
 * Based on Import_grib.c
 * @author Public Domain: Wesley Ebisuzaki @date 3/2019
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
 * HEADER:100:import_grib_fs:misc:2:read grib2 file (Y) sequentially for record that matches X (fixed string)
 */

/**
 * Sets the file pointer to the beginning of the file, and reads sequentially until it finds a 
 * record that matches the search string. It like a -if_fs for reading a grib file. 
 * 
 * The option -import_grib reads the input file sequentially. The -import_grib_fs option allows 
 * you select the field to read. 
 * 
 * ## Usage
 * -import_grib_fs STRING FILE
 *
 * Reads a grib message that matchs STRING from FILE. The match is by fixed string, not regular 
 * expression. 
 *
 * STRING is a subset of the match inventory (see -match_inv) starting with d=YYYYMMDDHH
 * 
 * FILE is the grib file for reading by -import_grib_fs
 * 
 * -g2clib 2 is not supported
 * 
 * Conversion to we:sn and we:ns is supported.
 * 
 * -not and -match do not affect -import_grib
 * 
 * @note Grid size (if it can be determined) must match the current grid.
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * Here is a script that reads the CORe and CFS analyses, and finds the average 500 mh height. CORe and CFS 
 * are on different grids, so the first step is to convert them to to same grid.
 * 
 * @code{.sh}
 * #!/bin/sh
 * #
 * # ave two Z500 fields from totally different systems (CORe, CFS)
 * #
 * # Step 1, convert to same grid
 * # Step 2, use -import_grib_fs (v3.0.0+) to read the field
 * #         use -rpn to average the two fields
 * #         change center to unknown because the metadata is not descriptive
 * #
 * # Note: for Step 2,
 * #        -match ":d=2000010100:HGT:500 mb:"  is not needed
 * #        -import_grib_fs "d=2000010100:HGT:500 mb:anl:"  file2.grb
 * #          can be replaced by -import_grib file2.grb
 * # I made the code more complicated because if file1 and file2 were already on
 * # same grid, you can avoid Step 1.
 *
 * file1=/cpc/cfsr/reanalyses/corefv3/ens_mean/3hr/pgb/2000/01/pgb_2000010100_ensmean
 * file2=/cpc/cfsr/archive/6hr_h/pgb.g2/2000/01/pgb.2000010100.g2
 *
 * # Step 1
 * wgrib2 $file1 -match ":d=2000010100:HGT:500 mb:" -new_grid_winds earth -new_grid ncep grid 3 file1.grb
 * wgrib2 $file2 -match ":d=2000010100:HGT:500 mb:" -new_grid_winds earth -new_grid ncep grid 3 file2.grb
 * echo "\n\n"
 *
 * # Step 2
 * wgrib2 file1.grb -match ":d=2000010100:HGT:500 mb:" -rpn "0.5:*:sto_0" \
 *    -import_grib_fs "d=2000010100:HGT:500 mb:anl:"  file2.grb \
 *    -rpn "0.5:*:rcl_0:+" -set_center 255 -grib_out ave.grb
 * exit 0
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 3/2019
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
