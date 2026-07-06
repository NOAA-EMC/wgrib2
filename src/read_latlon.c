/** @file
 * @brief Routine for reading latlon from a string.
 * @author Public Domain: Wesley Ebisuzaki @date 7/2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include "wgrib2.h"

/** Current geolocation type. */
extern enum geolocation_type geolocation;

/**
 * Read latitude and longitude values from a string.
 *
 * @param arg Input string containing latitude and longitude values.
 * @param lon Pointer to the output array for longitudes.
 * @param lat Pointer to the output array for latitudes.
 *
 * @return Number of valid latitude/longitude pairs read, or 0 on error.
 * 
 * @author Wesley Ebisuzaki @date 7/2019
 */
unsigned int read_latlon(const char *arg, double **lon, double **lat) {
    unsigned int n_out, i;
    int j, k;
    double *llat, *llon;
    n_out = 0;
    for (i = 0; i < strlen(arg); i++) {
        if (arg[i] == ':') n_out++;
    }
    if (n_out % 2 == 0) return 0;
    n_out = (n_out+1)/2;
        
    *lat = llat = (double *) malloc(sizeof(double) * (size_t) n_out);
    *lon = llon = (double *) malloc(sizeof(double) * (size_t) n_out);

    k = sscanf(arg, "%lf:%lf%n", llon, llat, &j);
    if (k != 2) fatal_error("get_latlon_list, no grid point locations","");
    for (i=1; i < n_out; i++) {
        arg += j;
        k = sscanf(arg, ":%lf:%lf%n", llon+i, llat+i, &j);
        if (k != 2) fatal_error("get_latlon_list: missing grid point locations: %s",arg);
    }

    geolocation = external;
    /* check values */
    for (i=0; i < n_out; i++) {
        if (llat[i] > 90.0 || llat[i] < -90.0) return (unsigned int) 0;
        if (llon[i] < 0.0) llon[i] += 360.0;
        if (llon[i] >= 360.0) llon[i] -= 360.0;
        if (llon[i] < 0.0 || llon[i] > 360.0) return (unsigned int) 0;
    }
    return n_out;
}
