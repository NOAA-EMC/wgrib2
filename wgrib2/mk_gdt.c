#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

//#if USE_IPOLATES == 2

/* mk_gdt.c 
 *
 * this routine takes the *sec[] and makes a g2lib igdtmpl(*) array for ipolates2 library
 *    igdt is used by NCEP grib and ipolates2 libraries
 *
 * problems: 
 *           ipolates2 is in a state of flux
 * solution: 
 *
 * INPUT:
 *
 * unsigned char **sec = grib2 message
 *     only use sec[3][*]
 *
 * OUTPUT:
 *
 * public domain 4/2019 Wesley Ebisuzaki
 *
 * support input none
 */

/* RADIUS_EARTH_IPOLATES is the radius of the earth as used by the IPOLATES library */
/* to make ipolates work with other radius .. scale distances */
#define RADIUS_EARTH_IPOLATES	6371200.0
#define IP_FACTOR (RADIUS_EARTH_IPOLATES/radius)

struct gdt_defn {
    int template_num;
    char *seq;
};

/* these are the table for the various grid temlates to go from *sec[3] */

const struct gdt_defn gdt_table[] = {
// 3.0: Lat/Lon grid
// { 0, {1,1,4,1,4,1,4,4,4,4,4,-4,4,1,-4,4,4,4,1} }
  { 0, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01" }, 

// 3.1: Rotated Lat/Lon grid
// { 1, 22, 0, {1,1,4,1,4,1,4,4,4,4,4,-4,4,1,-4,4,4,4,1,-4,4,4} },
   { 1, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01\x0c\x04\x04" },

// 3.10: Mercator
// {10, 19, 0, {1,1,4,1,4,1,4,4,4,-4,4,1,-4,-4,4,1,4,4,4} },
  {10, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x0c\x04\x01\x0c\x0c\x04\x01\x04\x04\x04" },

// 3.20: Polar Stereographic Projection
// {20, 18, 0, {1,1,4,1,4,1,4,4,4,-4,4,1,-4,4,4,4,1,1} },
  {20, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01\x01" },

// 3.30: Lambert Conformal
// {30, 22, 0, {1,1,4,1,4,1,4,4,4,-4,4,1,-4,4,4,4,1,1,-4,-4,-4,4} },
   {30, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01\x01\x0c\x0c\x0c\x04" },

// 3.40: Guassian Lat/Lon
// {40, 19, 0, {1,1,4,1,4,1,4,4,4,4,4,-4,4,1,-4,4,4,4,1} },
   {40, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01" }, 

// 3.32768: Rot Lat/Lon E-grid (Arakawa)
//         {32768, 19, 0, {1,1,4,1,4,1,4,4,4,4,4,-4,4,1,-4,4,4,4,1} },
         {32768, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01" },

// 3.32769: Rot Lat/Lon Non-E Staggered grid (Arakawa)
// {32769, 21, 0, {1,1,4,1,4,1,4,4,4,4,4,-4,4,1,-4,4,4,4,1,4,4} },
         {32769, "\x01\x01\x04\x01\x04\x01\x04\x04\x04\x04\x04\x0c\x04\x01\x0c\x04\x04\x04\x01\x04\x04" },
};

int mk_gdt(unsigned char **sec, int *igdtnum, int *igdttmpl, int *igdtleni) {
    int gdt, i, n, *out, seq_len, center;
    char *seq;
    unsigned char *in;

    gdt = code_table_3_1(sec);
    if (gdt < 0) return 1;
    if (gdt >= 32768) {		/* ip2lib only supports NCEP local grids */
	center = GB2_Center(sec);
	if (center != NCEP) return 1;
    }

    *igdtnum = gdt;

    n = sizeof(gdt_table) / sizeof(gdt_table[9]);

    /* find entry into gdt_table */
    for (i = 0; i  < n; i++) {
	if (gdt_table[i].template_num == gdt) break;
    }
    if (i > n) return 2;
    seq = gdt_table[i].seq;
    seq_len = strlen(seq);
    if (*igdtleni < seq_len) fatal_error_i("mk_gdt: gdt buffer too small %d", *igdtleni);
    *igdtleni = seq_len;

    /* create igdttmpl table */

    in = sec[3]+14;
    out = igdttmpl;
    while (*seq) {
	switch(*seq) {
	    case 1:
		*out++ = *in;
		in += 1;
		break;
	    case 2:
		*out++ = (int) uint2(in);
		in += 2;
		break;
	    case 4:
		*out++ = (int) uint4(in);
		in += 4;
		break;
	    case 9:
		*out++ = int1(in);
		in += 1;
		break;
	    case 10:
		*out++ = int2(in);
		in += 2;
		break;
	    case 12:
		*out++ = int4(in);
		in += 4;
		break;
	    default: fatal_error_i("mk_gdt: illegal value in sequence %d", *in);
	}
	seq++;
    } 

    return 0;
}

// #endif
