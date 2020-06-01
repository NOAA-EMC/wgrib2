#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 2/2012 Public Domain: Wesley Ebisuzaki
 */

/*
 * HEADER:200:wave_partition:inv:0:ocean surface wave partition (pdt=4.52)
 */

int f_wave_partition(ARG0) {
    int pdt;
    if (mode >= 0) {
	pdt = GB2_ProdDefTemplateNo(sec);
        if (pdt == 52) {
            if (sec[4][13] == 255) sprintf(inv_out,"wave partition=?");
	    else sprintf(inv_out,"wave partition=%d",(int) sec[4][13]);
	    inv_out += strlen(inv_out);
            if (sec[4][12] == 255) sprintf(inv_out,"/?");
	    else sprintf(inv_out,"/%d",(int) sec[4][12]);
	}
    }
    return 0;
}
