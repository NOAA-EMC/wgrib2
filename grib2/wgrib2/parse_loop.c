#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * parse_loop
 *
 * many options have loop definitions such as
 *
 *   if_n, if_rec, for and for_n
 *
 * this routine parse the start,end and step
 *  
 * 8/2010 in public domain Wesley Ebisuzaki
 */


int parse_loop(const char *string, int *start, int *end, int *step) {

    int i;

    i = sscanf(string,"%d:%d:%d", start, end, step);

    if (i == 2) {
	*step = 1;
    }
    else if (i == 1) {
	i = sscanf(string,"%d::%d", start, step);
	if (i == 2) {
	    *end = INT_MAX;
	}
	else {
	    *step = 1;
	    *end = INT_MAX;
        }
    } 
    else if (i != 3) fatal_error("parse_loop: expected start:end:step found %s", string);

    if (*start <= 0) fatal_error("parse_loop: start <= 0 %s", string);
    if (*end < *start) fatal_error("parse_loop: end < start %s", string);
    if (*step <= 0) fatal_error("parse_loop: step <= 0 %s", string);

    return 0;
}
