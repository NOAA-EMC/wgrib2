#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"
/*
 * Set_ens_num.c
 *
 * converts PDT 0,1 -> 1,   8,11 -> 11
 *
 * 11/2011: Public Domain: Wesley Ebisuzaki
 * 2/2015 Wesley Ebisuzaki: can be called from set_metadata now
 */


/*
 * HEADER:100:set_ens_num:misc:3:ensemble member info, X=code table 4.6 Y=pert num Z=num ens members -1=No Change
 */
int f_set_ens_num(ARG3) {

    int i, pdt, type_ens, ens_fcst, num_ens, ens_pdt;
    unsigned char new_sec4[SET_PDT_SIZE];
    unsigned char *p;

    if (mode < 0) return 0;

    type_ens = atoi(arg1);
    ens_fcst = atoi(arg2);
    num_ens = atoi(arg3);

    p = code_table_4_6_location(sec);
    if (p != NULL) {
	p[0] = (unsigned char) type_ens;
	p[1] = (unsigned char) ens_fcst;
	p[2] = (unsigned char) num_ens;
	return 0;
    }

    pdt = code_table_4_0(sec);

/*
 *  promote pdts
 *	0 -> 1
 *	8 -> 11
 *	32 -> 33
 *	40 -> 41
 *	42 -> 43
 *	44 -> 45
 *	46 -> 47
 *	48 -> 49
 *	53 -> 54
 *	55 -> 56
 *	57 -> 58
 *	67 -> 68
 *	70 -> 71
 *	72 -> 73
 */

    switch(pdt) {
	case 0: ens_pdt = 1; break;
	case 8: ens_pdt = 11; break;
	case 32: ens_pdt = 33; break;
	case 40: ens_pdt = 41; break;
	case 42: ens_pdt = 43; break;
	case 44: ens_pdt = 45; break;
	case 46: ens_pdt = 47; break;
	case 48: ens_pdt = 49; break;
	case 53: ens_pdt = 54; break;
	case 55: ens_pdt = 56; break;
	case 57: ens_pdt = 58; break;
	case 67: ens_pdt = 68; break;
	case 70: ens_pdt = 71; break;
	case 72: ens_pdt = 73; break;
	default:
		fprintf(stderr,"set_ens_num: could not promote to ensemble pdt (%d)\n", pdt);
		return 0;
    }
    i = new_pdt(sec, new_sec4, ens_pdt, -1, 1, NULL);
    if (i == 0) update_sec4(sec, new_sec4);
    p = code_table_4_6_location(sec);
    if (p != NULL) {
	p[0] = (unsigned char) type_ens;
	p[1] = (unsigned char) ens_fcst;
	p[2] = (unsigned char) num_ens;
	return 0;
    }
    fatal_error_i("set_ens_num: program error, failed to promote to ensemble pdt (%d)", pdt);
    return 0;
}
