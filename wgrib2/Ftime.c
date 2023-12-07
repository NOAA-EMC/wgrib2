#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"
/*
 * Public Domain 2017: Wesley Ebisuzaki
 *
 * allow multiple versions of ftimeX, set_ftimeX to be available
 *
 * note: ftimeX, set_ftimeX should not (1) require initialization, (2) require cleanup
 * (3) require static variables.
 */

#define DEFAULT_FTIME	2

#ifndef DEFAULT_FTIME
int version_ftime = 1;
#else
int version_ftime = DEFAULT_FTIME;
#endif
/*
 * HEADER:440:ftime:inv:0:same as ftime2
 */

int f_ftime(ARG0) {
    if (mode < 0) return 0;
    return f_ftime2(call_ARG0(inv_out,NULL));
    // if (version_ftime == 1) return f_ftime1(call_ARG0(inv_out,NULL));
    // if (version_ftime == 2) return f_ftime2(call_ARG0(inv_out,NULL));
    // return 1;
}

/*
 * HEADER:100:set_ftime:misc:1:same as set_ftime2
 */

int f_set_ftime(ARG1) {
    if (mode < 0) return 0;
    return f_set_ftime2(call_ARG1(inv_out,NULL,arg1));
    // if (version_ftime == 1) return f_set_ftime1(call_ARG1(inv_out,NULL,arg1));
    // if (version_ftime == 2) return f_set_ftime2(call_ARG1(inv_out,NULL,arg1));
    // return 1;
}


// /*
//  * HEADER:440:set_version_ftime:setup:1:set version of ftime X=1 (deprecated), 2
//  */
// int f_set_version_ftime(ARG1) {
//    if (mode != -1) return 0;
//   version_ftime = atoi(arg1);
//    if (version_ftime < 1 || version_ftime > 2) fatal_error_i("set_version_ftime: %d not valid version", version_ftime);
//    return 0;
// }
