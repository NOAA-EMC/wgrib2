#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Inv.c     10/2024  Public Domain   Wesley Ebisuzak
 *
 */
extern int only_submsg;

/*
 * HEADER:100:submsg:misc:1:process submessage X (0=process all messages)
 */

int f_submsg(ARG1) {

    only_submsg = atoi(arg1);

    return 0;
}
