#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Fix_ncep_2.c
 *
 *  Some routines for probablilty forecasts
 *
 * 11/2008: Public Domain: Wesley Ebisuzaki
 * 12/2008: fixed return code
 * 1/2008: ncep bug fix
 * 3/2018: move ncep bug fix to Fix_ncep_2.c
 *
 */

#define LOWER_LIMIT scaled2flt(INT1(p[1]), int4(p+2) )
#define UPPER_LIMIT scaled2flt(INT1(p[6]), int4(p+7) )

extern int fix_ncep_2_flag;


/*
 * HEADER:100:fix_ncep_2:setup:0:ncep bug fix 2, probability observation < -ve number
 */

int f_fix_ncep_2(ARG0) {
    fix_ncep_2_flag = 1;
    return 0;
}

int fix_ncep_2(unsigned char **sec) {
    unsigned char *p;
    int i;

    p = code_table_4_9_location(sec);
    if (p == NULL) return 0;

    if ((p[2] & 0x0c) == 0x0c) {	// fix bug
	i = int4_comp(p+2);
	int_char(i, p+2);
    }
    if ((p[7] & 0x0c) == 0x0c) {	// fix bug
	i = int4_comp(p+7);
	int_char(i, p+7);
    }
    return 0;
}
