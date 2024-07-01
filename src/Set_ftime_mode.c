#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * control ftime output
 * 1/2023 Public Domain   Wesley Ebisuaki
 *
 * 0 - default
 * 1 - stop changing 60 sec -> 1 min, 60 min -> 1 hour,  24 hour -> 1 day
 */

/*
 * HEADER:100:set_ftime_mode:setup:1:set ftime mode X,  0 - default,  1 - do not simplify time codes
 */

int ftime_mode = 0;

int f_set_ftime_mode(ARG1) {

    if (strcmp(arg1,"0") == 0) {
	ftime_mode = 0;
    }
    else if (strcmp(arg1,"1") == 0) {
	ftime_mode = 1;
    }
    else fatal_error("set_ftime_mode: input must  be 0 or 1");
    return  0;
}
