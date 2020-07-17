#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * for partial http transfers, you need to know start and finish of the grib message
 * this option writes out the range of the record.
 *
 * See how trivial adding new functions has become!
 */

extern long int pos;
extern unsigned long int len;

/*
 * HEADER:100:range:inv:0:print out location of record in bytes, 0 = first byte
 */
int f_range(ARG0) {
    if (mode >= 0) sprintf(inv_out, "range=%ld-%ld",pos,pos+len-1);
    return 0;
}
