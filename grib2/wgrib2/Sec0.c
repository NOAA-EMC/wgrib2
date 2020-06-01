#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 12/2006: Public Domain Wesley Ebisuzaki
 * 1/2007 Cleanup M. Schwarb
 */

/*
 * HEADER:200:Sec0:inv:0:contents of section0
 */

int f_Sec0(ARG0) {

    unsigned char *p;
    if (mode >= 0) {
	p = sec[0];
	sprintf(inv_out,
           "Sec0=%c%c%c%c reserved 0x%.2x%.2x Discipline=%d Grib_Edition=%d len=%lu"
	   , p[0], p[1], p[2], p[3], p[4], p[5], (int) GB2_Discipline(sec), (int) GB2_Edition(sec),
	   (unsigned long int) GB2_MsgLen(sec));
    }
    return 0;
}

/*
 * HEADER:200:table:inv:0:parameter table
 */

int f_table(ARG0) {

    if (mode >= 0) {
	sprintf(inv_out, "discipline=%d", GB2_Discipline(sec));
	inv_out += strlen(inv_out);
	sprintf(inv_out," master_table=%d", GB2_MasterTable(sec));
	inv_out += strlen(inv_out);
	sprintf(inv_out," local_table=%d", GB2_LocalTable(sec));
    }
    return 0;
}

