/** @file
 * @brief Converts PDT 0..6 -> 5, 8..15 -> 9
 * @author Public Domain: Wesley Ebisuzaki @date 2/2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * HEADER:100:set_prob:misc:5:X/Y forecasts Z=Code Table 4.9 A=lower limit B=upper limit
 */

/**
 * Converts some common non-probability Product Definition Template (PDT) to a probability PDT. 
 * Then it adds the metadata for the probability part of the PDT.
 * 
 * ### Step 1
 * If current PDT is 0,1,2,3,4 or 6, then the PDT is converted to 6 (4.6) using the -set_pdt +5 
 * option.
 * 
 * If current PDT is 8,10,11,12,13,14 or 15, the he PDT is converted to 9 (4.9) using the -set_pdt 
 * +9 option.
 * 
 * ### Step 2
 * If the current PDT does not have Code Table 4.9 entry, print a warning and return.
 * 
 * ### Step 3
 * Fill in the various probability metadata using arguments to the option.
 * 
 * ## Usage
 * -set_prob X Y Z A B
 *
 * X = forecast probability number
 * Y = total number of forecast probabilities
 * Z = probability type (see Code Table 4.9)
 * A = lower limit
 * B = upper limit
 * 
 * X, Y, Z, A, or B can have the value "". When the value "" is used, the previous value is not 
 * changed.
 * 
 * @param ARG5 List of function arguments set by wgrib2's main() function (see @ref ARG5). These arguments 
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
int f_set_prob(ARG5) {

    int pdt, val, scale;
    double value;
    unsigned char *p;

    if (mode < 0) return 0;
    pdt = code_table_4_0(sec);

    switch(pdt) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 6:
            f_set_pdt(call_ARG1(inv_out, NULL, "+5"));
            break;
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            f_set_pdt(call_ARG1(inv_out, NULL, "+9"));
            break;
    }
    p = code_table_4_9_location(sec);
    if (p == NULL) {
        fprintf(stderr,"\nWARNING: set_prob does not work with PDT %d\n", pdt);
        return 0;
    }
 
    if (arg1 != NULL && strcmp(arg1,"") != 0)
        p[-2] = (unsigned char) atoi(arg1);
    if (arg2 != NULL && strcmp(arg2,"") != 0)
        p[-1] = (unsigned char) atoi(arg2);
    if (arg3 != NULL && strcmp(arg3,"") != 0)
       p[0] = (unsigned char) atoi(arg3);

    // encode lower limit
    if (arg4 != NULL && strcmp(arg4,"") != 0) {
        value = atof(arg4);
        best_scaled_value(value, &scale, &val);
        scaled_char(scale,  val, p+1);
    }

    // encode upper limit
    if (arg5 != NULL && strcmp(arg5,"") != 0) {
        value = atof(arg5);
        best_scaled_value(value, &scale, &val);
        scaled_char(scale, val, p+6);
    }
    return 0;
}
