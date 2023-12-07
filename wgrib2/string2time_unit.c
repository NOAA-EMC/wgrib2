#include <stdio.h>
#include <string.h>
#include "wgrib2.h"

/*
 * string2time_unit
 *
 * convert GrADS time units to grib2 units
 *
 * 12/2019 Public Domain Wesley Ebisuzaki
 *
 */

int string2time_unit(char *string) {
    int unit;
    unit = -1;
    if (strncmp(string,"hr",3) == 0) unit = 1;
    else if (strncmp(string,"dy",3) == 0) unit = 2;
    else if (strncmp(string,"mo",3) == 0) unit = 3;
    else if (strncmp(string,"yr",3) == 0) unit = 4;
    else if (strncmp(string,"mn",3) == 0) unit = 0;
    return unit;
}
