/** @file
 * @brief Converts PDT 0,1 -> 2    8,11 -> 12. Changes code table 4.7 and adds "number of 
 * ensemble members".
 * @author Public Domain: Wesley Ebisuzaki @date 11/2011
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * HEADER:100:set_ensm_derived_fcst:misc:2:convert PDT 0,1,2 -> 2, 8,11,12 -> 12, X=code table 4.7 Y=num ens members
 */

/**
 * Changes Product Definition Template (PDT) 0,1,2 -> 2 or 8,11,12 -> 12. PDT 2, and 12 are for 
 * "ensemble mean derived forecasts". Effectively this options describes the forecast as being 
 * derived from an ensemble of forecasts. The types derived forecasts is listed in [code table 4.7]
 * (https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-7.shtml). The "cluster" 
 * derived forecast should not be used with this option as PDT 2 and 12 do not have metadata to 
 * describe the cluster that was used. 
 * 
 * ## Usage
 * -set_ensm_derived_fcst X Y
 * 
 * X = [code table 4.7]
 * (https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-7.shtml).
 * 
 * Y = 0..255  number of forecasts in the ensemble
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 sst.grb2  -set_ensm_derived_fcst 1 10 -grib newsst.grb
 * 1:0:d=1981110100:TMP:surface:0-1 month ave anl:wt ens-mean
 * @endcode
 * 
 * <pre>
 * -set_ensm_derived_fcst 1 10
 *      1 = weighted mean of all members (code table 4.7)
 *      10 = number of ensemble members

 * -grib newsst.grb
 *      save the new grib file in newsst.grb
 * </pre>
 * 
 * @author Wesley Ebisuzaki @date 11/2011
 */
int f_set_ensm_derived_fcst(ARG2) {

    int code_table_4_7, num_ens;
    int i, n, n0, pdt;
    unsigned char *sec4;

    if (mode < 0) return 0;

    code_table_4_7 = atoi(arg1);
    if (code_table_4_7 < 0 || code_table_4_7 > 255) 
        fatal_error("set_ensm_derived_fcst: code table 4.7 0..255 found %s", arg1);

    num_ens = atoi(arg2);
    if (num_ens == -1) num_ens = 255;		/* undefined == -1 or 255 */
    if (num_ens < 0 || num_ens > 255) 
        fatal_error("set_ensm_derived_fcst: num ens emembers 0..255 found %s", arg2);

    pdt = code_table_4_0(sec);

    /* all ready pdt == 2 or 12 */
    if (pdt == 2 || pdt == 12) {
        if (code_table_4_7 != 255) sec[4][34] = code_table_4_7;
        if (num_ens != 255) sec[4][35] = num_ens;
        return 0;
    }

    n0 = GB2_Sec4_size(sec);

    if (pdt == 0 || pdt == 1) {
        n = 36;
    }
    else if (pdt == 8) {
        n = n0 + 2;
    }
    else if (pdt == 11) {
        n = n0 - 1;
    }
    else {
        fprintf(stderr,"set_ensm_derived_fcst: only works with product defn template 0,1,2,8,11,12\n");
        return 0;
    }

    sec4 = (unsigned char *) malloc(n * sizeof(unsigned char));
    if (sec4 == NULL) fatal_error("set_ensm_derived_fcst: memeor allocation error","");

    // now to add ensemble information
    if (pdt == 0 || pdt == 1) {
        for (i = 0; i < 34; i++) sec4[i] = sec[4][i];
        sec4[34] = code_table_4_7;
        sec4[35] = num_ens;
    }
    else if (pdt == 8) {
        for (i = 0; i < 34; i++) sec4[i] = sec[4][i];
        sec4[34] = code_table_4_7;
        sec4[35] = num_ens;
        for (i = 36; i < n; i++) sec4[i] = sec[4][i-2];
    }
    else if (pdt == 11) {
        for (i = 0; i < 34; i++) sec4[i] = sec[4][i];
        sec4[34] = code_table_4_7;
        sec4[35] = num_ens;
        for (i = 36; i < n; i++) sec4[i] = sec[4][i+1];
    }

    uint_char(n, sec4);                   // length of sec[4]
    sec4[7] = 0;
    sec4[8] = pdt >= 8 ? 12 : 2;

    update_sec4(sec, sec4);
    free(sec4);
    
    return 0;
}
