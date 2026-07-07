/** @file
 * @brief Converts PDT 0..6 -> 6, 8..15 -> 10.
 * @author Public Domain: Wesley Ebisuzaki @date 2/2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * HEADER:100:set_percentile:misc:1:convert PDT 0..6 -> 6, 8..15 -> 10, X=percentile (0..100)
 */

/**
 * Add or change the percentile of a forceast.
 * 
 * Probabilistic forecasts can be presented as percentiles. For example, 10% forecast of 
 * temperature means that 1 time out of 10, the expected temperature will less than the 
 * forecasted temperature. 
 * 
 * Percentile forecasts use product definition templates (PDT) of 6 or 10 depending whether 
 * the forecast is for a single time or a time interval. The -set_percentile option converts 
 * pdt 0..6 -> 6 and 8..15 -> 10. 
 * 
 * Percentiles are also supported by the -set_metadata option. 
 * 
 * ## Usage
 * -set_percentile X 
 * 
 * X = percentile (0 to 100)
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 *
 * @author Wesley Ebisuzaki @date 2/2015
 */
int f_set_percentile(ARG1) {

    int pdt, percent;
    unsigned char *p;

    if (mode < 0) return 0;

    pdt = code_table_4_0(sec);
    percent = atoi(arg1);
    if (percent < 0) percent = 0;
    if (percent > 100 ) percent = 100;

    switch(pdt) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 5:
            f_set_pdt(call_ARG1(inv_out, NULL, "+6"));
            break;
        case 8:
        case 9:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            f_set_pdt(call_ARG1(inv_out, NULL, "+10"));
            break;
    }

    p = percentile_value_location(sec);
    if (p != NULL) *p = (unsigned char) percent;

    return 0;
}
