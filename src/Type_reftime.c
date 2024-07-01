#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Type of reference time
 *
 * 2023: Public Domain: Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:type_reftime:inv:0:type of reference time
 */

int f_type_reftime(ARG0) {
    const char *string;

    if (mode < 0) return 0;
    switch(code_table_1_2(sec)) {
	case 0: string = "type_reftime=analysis"; break;
	case 1: string = "type_reftime=start of forecast"; break;
	case 2: string = "type_reftime=verifying time of forecast"; break;
	case 3: string = "type_reftime=observation time"; break;
	case 4: string = "type_reftime=local time"; break;
        default: string="type_reftime=unknown"; break;
    }
    strcat(inv_out, string);
    return 0;
}
