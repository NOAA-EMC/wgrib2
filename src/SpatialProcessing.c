/** @file
 * @brief Show spatial processing information (Product Definition Template 4.15).
 * @author Public Domain: Wesley Ebisuzaki  @date 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:400:spatial_proc:inv:0:show spatial processing, pdt=4.15
 */

/**
 * Prints the spatial processing information (Product Definition Template 4.15).
 * 
 * Product Definition Template 4.15 is used to describe spatial processing of a forecast or 
 * analysis. For example, you interpolate a model grid to an output grid. For some fields 
 * you may use nearest neighbor interpolation and for other fields you may use bilinear 
 * interpolation. The North American Regional Reanalysis used both types of interpolation 
 * for surface and near surface fields. Another use of PDT 15 is in aviation. You might want 
 * the maximum clear air turbulence when the interpolate from a fine grid to a coarse grid. 
 * 
 * The option, -spatial_proc, prints the spatial properties that are encoded in PDT 4.15. This 
 * output is part of the of the standard inventory, -s. 
 * 
 * ## Usage
 * -spatial_proc
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 file1 -spatial_proc
 * 1:0:d=2015101200:CAT:200 mb:12 hour fcst:spatial max:missing interpolation
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 file2 -spatial_proc
 * 1:0:d=2015101200:PRMSL:mean sea level:anl:spatial none:bilinear interpolation
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2010
 */
int f_spatial_proc(ARG0) {
    int val, val1, center;
    const char *string;
    if (mode >= 0 && GB2_ProdDefTemplateNo(sec) == 15) {
        center = GB2_Center(sec);
        val1 = code_table_4_10(sec);
        string = NULL;
        switch(val1) {
#include "CodeTable_4.10.dat"
        }
        if (val1 != 255 && string) sprintf(inv_out, "spatial %s:", string);
        else sprintf(inv_out, "spatial none:");
        inv_out += strlen(inv_out);

        val = code_table_4_15(sec);
        string = NULL;
        switch(val) {
#include "CodeTable_4.15_short.dat"
        }
        if (string) sprintf(inv_out, "%s", string);
        inv_out += strlen(inv_out);

        if (mode > 0) {
            sprintf(inv_out,",code table 4.15=%d,#points=%d", val, sec[4][36]);
        }
    }
    return 0;
}
