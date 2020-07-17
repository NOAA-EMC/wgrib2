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
 * converts PDT 0..6 -> 6    8..15 -> 10
 *   
 *
 * 2/2015: Public Domain: Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:set_percentile:misc:1:convert PDT 0..6 -> 6, 8..15 -> 10, X=percentile (0..100)
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
