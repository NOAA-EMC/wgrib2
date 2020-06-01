#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int decode, last_message;

/*
 * HEADER:100:model_version_date:inv:0:prints model date code
 */

int f_model_version_date(ARG0) {
    unsigned char *p;
    if (mode >= 0) {
        p = year_of_model_version_date_location(sec);
        if (p != NULL) {
	    sprintf(inv_out,"model_version_date=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d:", uint2(p),(int) p[2],
              (int) p[3], (int) p[4], (int) p[5], (int) p[6]);
        }
    }
    return 0;
}
