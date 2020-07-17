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
 * HEADER:100:set_ens_num:misc:3:convert PDT 0,1 -> 1,  8,11 -> 11, X=code table 4.6 Y=pert num Z=num ens members -1=No Change
 */
int f_set_ens_num(ARG3) {

    int i, n, pdt, type_ens, ens_fcst, num_ens;
    unsigned char sec4[SET_PDT_SIZE];
    unsigned char *p;

    if (mode < 0) return 0;
    pdt = code_table_4_0(sec);

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

    if (pdt != 0 && pdt != 8) {
	fprintf(stderr,"set_ens_num: only promotes pdt 0 and 8\n");
	return 0;
    }

    n = GB2_Sec4_size(sec);
    if (n+3 > SET_PDT_SIZE)  fatal_error_i("set_ens_num: prog error, increase SET_PDT_SIZE %d", SET_PDT_SIZE);

    // now to add ensemble information
    for (i = 0; i < 34; i++) sec4[i] = sec[4][i];

    sec4[34] = (type_ens >= 0) ? type_ens : 255;
    sec4[35] = (ens_fcst >= 0) ? ens_fcst : 255;
    sec4[36] = (num_ens >= 0) ? num_ens : 255;

    for (i = 34; i <= n; i++) sec4[i+3] = sec[4][i];

    uint_char(n+3, sec4);                   // length of sec[4]
    sec4[7] = 0;                            // pdt = 0 -> 1, 8 -> 11
    sec4[8] = pdt == 8 ? 11 : 1;
    update_sec4(sec, sec4);
    return 0;
}
