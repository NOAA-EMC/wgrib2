#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Precision.c     10/2024  Public Domain    Wesley Ebisuzaki
 *
 * options dealing with the precision/scaling of input
 */

/*
 * HEADER:510:scaling:inv:0:scaling for packing (old format)
 */

int f_scaling(ARG0) {

    int dec, bin, nbits;
    double base;
    if (mode < 0) return 0;
    if (scaling(sec, &base, &dec, &bin, &nbits) == 0) {
       sprintf(inv_out,"scaling ref=%g dec_scale=%d bin_scale=%d nbits=%d", base, dec, bin, nbits);
    }
    return 0;
}

/*
 * HEADER:510:scale:inv:0:scale for packing
 */
int f_scale(ARG0) {

    int dec, bin, nbits;
    double base;
    if (mode < 0) return 0;
    if (scaling(sec, &base, &dec, &bin, &nbits) == 0) {
       sprintf(inv_out,"scale=%d,%d", dec, bin);
    }
    return 0;
}

