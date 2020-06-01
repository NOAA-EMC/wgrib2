#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * Set_percentile.c
 *
 * converts PDT 0..6 -> 5    8..15 -> 9
 *   
 *
 * 2/2015: Public Domain: Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:set_prob:misc:5:X/Y forecasts Z=Code Table 4.9 A=lower limit B=upper limit
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
  
    p[-2] = (unsigned char) atoi(arg1);
    p[-1] = (unsigned char) atoi(arg2);
    p[0] = (unsigned char) atoi(arg3);

    // encode lower limit
    value = atof(arg4);
    best_scaled_value(value, &scale,&val);
    p[1] = scale & 255;
    uint_char(val, p+2);

    // encode upper limit
    value = atof(arg5);
    best_scaled_value(value, &scale,&val);
    p[6] = scale & 255;
    uint_char(val, p+7);
    return 0;
}
