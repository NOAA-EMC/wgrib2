#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_TIGGE
/*
 * Tigge.c .. enable TIGGE grib table
 *
 * 2007: Public Domain, Wesley Ebisuzaki
 *
 */

int tigge;

/*
 * HEADER:100:tigge:setup:0:use modified-TIGGE grib table
 */

int f_tigge(ARG0) {
    if (mode == -1) tigge = 1;
    return 0;
}
#else
int f_tigge(ARG0) {
  if (mode == -1) {fprintf(stderr,"tigge is not installed\n"); return 1;}
  return 0;
}
#endif

