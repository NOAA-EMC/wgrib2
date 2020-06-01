#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Public Domain 2010: Wesley Ebisuzaki
 */

/*
 * HEADER:400:spatial_proc:inv:0:show spacial processing, pdt=4.15
 */
int f_spatial_proc(ARG0) {
    int val, val1;
    const char *string;
    if (mode >= 0 && GB2_ProdDefTemplateNo(sec) == 15) {

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
