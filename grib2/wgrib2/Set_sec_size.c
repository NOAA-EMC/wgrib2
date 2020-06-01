#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * the set options
 *
 * routine resizes a section to a different length
 *   keeps old data, new data is set to 255
 *
 * 5/2011 Public Domain by Wesley Ebisuzaki
 */

/*
 * HEADER:100:set_sec_size:misc:2:resizes section , X=section number, Y=size in octets, DANGEROUS
 */


int f_set_sec_size(ARG2) {

    int section, n, old_size;
    int i, j;
    static unsigned char *new_sec[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} ;

    section = atoi(arg1);
    n = atoi(arg2);

    if (mode == -1) {	/* error checking */
        if (section < 0 || section > 8) fatal_error_i("set_sec_size: bad section %d", section);
        if (n < 0) fatal_error_i("set_sec_size: number of octets must be >= 0 not %d",n);
        if (section == 0 && n != 16) fatal_error("set_sec_size: sec 0 must be 16 octets","");
        if (section == 8 && n != 4) fatal_error("set_sec_size: sec 8 must be 4 octets","");
    }
    if (mode < 0) return 0;

    if (sec[section] == NULL) old_size = 0;
    else if (section == 0) old_size = 16;
    else if (section == 8) old_size = 4;
    else old_size = uint4(sec[section]);

    if (new_sec[section] != NULL) {
	free(new_sec[section]);
	new_sec[section] = NULL;
    }

    if (n == 0) {
	sec[section] =  NULL;
	return 0;
    }

    new_sec[section] = (unsigned char *) malloc(n * sizeof(unsigned char));
    if (new_sec[section] == NULL) fatal_error("set_sec_size: failed memory alloction","");

    for (i = 0; i < n; i++) new_sec[section][i] = 255;

    j = (old_size < n) ? old_size : n;
    for (i = 0; i < j; i++) new_sec[section][i] = sec[section][i];

    if (section >= 2 && section <= 7 && n >= 5) {
	uint_char(n, new_sec[section]);
	new_sec[section][4] = (unsigned char) section;
    }
    sec[section] = new_sec[section];
    return 0;
}
