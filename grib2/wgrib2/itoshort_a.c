#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"

/* int2char.c  10/2013 public domain Wesley Ebisuzaki
 *
 * returns a string with short format integer
 *
 */

void itoshort_a(char *string, int i) {
    int itmp, exp;

    if (i == 0) {
	strcpy(string, "0");
	return;
    }

    if (i < 0) {
	*string++ = 'n';
	i = -i;
    }

    exp = 0;
    itmp = i;
    while (i % 10 == 0) { i /=  10; exp++; }
    if (exp >= 3) sprintf(string, "%de%d", i, exp);
    else sprintf(string, "%d", itmp);
    return;
}

