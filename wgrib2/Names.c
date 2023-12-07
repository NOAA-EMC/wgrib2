#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Names.c
 * 1/2020: Public Domain, Wesley Ebisuzaki
 *
 * NCEP has its naming convention
 * ECMWF has its naming convention
 * TIGGE (obsolete?) has its naming convention
 *
 * -names is grib name sources ecmwf or ncep
 */

/*
 * HEADER:100:names:setup:1:grib name convention, X=DWD, dwd, ECMWF, ecmwf, NCEP, ncep
 */

extern int names;

int f_names(ARG1) {

    if (mode != -1) return 0;

    if (strcmp("ncep",arg1) == 0) names = NCEP;
    else if (strcmp("NCEP",arg1) == 0) names = NCEP;
    else if (strcmp("ecmwf",arg1) == 0) names = ECMWF;
    else if (strcmp("ECMWF",arg1) == 0) names = ECMWF;
    else if (strcmp("dwd",arg1) == 0) names = DWD1;
    else if (strcmp("DWD",arg1) == 0) names = DWD1;
    else fatal_error("names: unrecognized argument %s", arg1);
    return 0;
}
