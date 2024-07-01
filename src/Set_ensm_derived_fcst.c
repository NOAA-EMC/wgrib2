#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * Set_ensm_derived_fcst.c
 *
 * converts PDT 0,1 -> 2    8,11 -> 12
 *  changes code table 4.7 and adds "number of ensemble members"
 *
 * 11/2011: Public Domain: Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:set_ensm_derived_fcst:misc:2:convert PDT 0,1,2 -> 2, 8,11,12 -> 12, X=code table 4.7 Y=num ens members
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
