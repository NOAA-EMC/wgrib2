#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Warn_old_g2
 *  warn if reading complex file that old g2lib will croak on
 *
 *  v 0.1 experimental
 *
 * 12/2012: Public Domain: Wesley Ebisuzaki
 *
 */


/*
 * HEADER:100:warn_old_g2:inv:0:warn if old g2lib would have problem
 */

int f_warn_old_g2(ARG0) {
    unsigned int pack, order, bytes, sign1, sign2;
    if (mode >= 0) {
        pack = code_table_5_0(sec);
        if (pack != 3) return 0;		// complex packing with spatial differences
	order = sec[5][47];			// order = 1 or 2
	bytes = sec[5][48];
	sign1 = sec[7][5] & 128;
	if (order == 2) {
	    sign2 = sec[7][5+bytes] & 128;
	}
	else {
	    sign2=0;
	}

        if (mode > 0) {
	    sprintf(inv_out, "complex packing order %u, bytes %u sign %u %u ",order, bytes, sign1, sign2);
	    inv_out += strlen(inv_out);
	}

	if (sign1 || sign2) {
	    sprintf(inv_out, "*** WARNING: old g2lib will fail ***");
  	    inv_out += strlen(inv_out);
	}
    }
    return 0;
}
