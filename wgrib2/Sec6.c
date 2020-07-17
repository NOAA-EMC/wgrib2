#include <stdio.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * routines involving Section 6
 * public domain 2006: Wesley Ebisuzaki
 * 1/2007: cleanup M. Schwarb
 */

/*
 * HEADER:700:bitmap:inv:0:bitmap mode
 */


int f_bitmap(ARG0) {

    int i;
    unsigned int nmiss;
    if (mode >= 0) {
	i = code_table_6_0(sec);
	if (i == 0) {
//	    nmiss = GB2_Sec3_npts(sec)-uint4(sec[5]+5);
	    nmiss = GB2_Sec3_npts(sec) - GB2_Sec5_nval(sec);
	    sprintf(inv_out,"bitmap %d undef pts", nmiss);
	    if (nmiss != missing_points(sec[6]+6, GB2_Sec3_npts(sec)))
		fatal_error("inconsistent number of undefined points","");
	}
	else if (i == 255) {
	   sprintf(inv_out,"no bitmap");
	}
	else if (i == 254) {
	    sprintf(inv_out,"old bitmap");
	}
	else {
	    sprintf(inv_out, "predefined bitmap #%d", i);
	}
    }
    return 0;
}

/*
 * HEADER:700:Sec6:inv:0:show bit-map section
 */

int f_Sec6(ARG0) {

    if (mode >= 0) {
	sprintf(inv_out,"Sec6 length %u bitmap indicator %u", uint4(sec[6]),
            (unsigned int) sec[6][5]);
    }
    return 0;
}
