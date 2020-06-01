#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Fix_ncep_4.c
 *
 * sets flag controlling whether DX and DY are defined
 *
 * 8/2008: Public Domain: Wesley Ebisuzaki
 *
 */
extern int fix_ncep_4_flag;


/*
 * HEADER:100:fix_ncep_4:setup:0:fixes NCEP grib2 files where DX and DY are undefined
 */
int f_fix_ncep_4(ARG0) {
    fix_ncep_4_flag = 1;
    return 0;
}


int fix_ncep_4(unsigned char **sec) {
    int i;
    i = flag_table_3_3(sec);
    if (i >= 0) {
	i = i | 48;
	set_flag_table_3_3(sec, i);
    }
    return 0;
}
