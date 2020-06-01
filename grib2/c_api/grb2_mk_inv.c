#include <stdio.h>
#include "c_wgrib2api.h"

/* C transation of fortran grb2_mk_inv(..) */

int grb2_mk_inv(char *grb, char *inv) {

    char *argv[7];

    argv[0] = "wgrib2_C_api";
    argv[1] = grb;
    argv[2] = "-rewind_init";
    argv[3] = grb;
    argv[4] = "-inv";
    argv[5] = inv;
    argv[6] = "-Match_inv";
   
    return wgrib2(7, argv);
}
