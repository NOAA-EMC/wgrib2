#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * the set options  4/2019 Public Domain Wesley Ebisuzaki
 *
 * routines to make a blank GDS
 *
 * 4/2019 1st version
 */

/*
 * HEADER:100:set_gds:misc:1:makes new gds (section 3), X=size in bytes
 */


int f_set_gds(ARG1) {

    unsigned char *new_sec3;
    int size_new_sec3, i;

    if (mode < 0) return 0;
    size_new_sec3 = atoi(arg1);
    if (size_new_sec3 <  5) fatal_error("set_gdt: X=length of GDT (Sec 3)","");
    new_sec3 = (unsigned char *) malloc(sizeof(unsigned char) * (size_t) size_new_sec3);
    if (new_sec3 == NULL) fatal_error("new_gds: memory allocation","");

    /* size of sec3 */
    uint_char(size_new_sec3, new_sec3);
    /* id for sec3 */
    new_sec3[4] = 3;
    for (i = 5; i < size_new_sec3; i++) new_sec3[i] = 255;

    update_sec3(sec, new_sec3);
    free(new_sec3);
    return 0;
}
