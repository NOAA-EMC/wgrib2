#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "grb2.h"
#include "wgrib2.h"

/*
 * parse_msg.c public domain 2007                              Wesley Ebisuzaki
 *
 * 1/2007 cleanup M. Schwarb
 * 6/2009 fix repeated bitmaps W. Ebisuzuaki
 * 1/2011 added seq input W. Ebisuzaki
 * 12/2014 W. Ebisuzaki, own file, updated to use sec[]
 */


/*
 * with grib 1, a message = 1 field
 * with grib 2, a message can have more than one field
 *
 * this routine parses a grib2 message that has already been read into buffer
 *
 * parse_1st_msg .. returns 1st message starting at sec[0], fills in sec[]
 *   note sec[9] is used to store pointer to last valid bitmap
 */ 

int parse_1st_msg(unsigned char **sec) {

	unsigned char *p;
 
	if (sec[0] == NULL) fatal_error("parse_1st_msg .. sec[0] == NULL","");
	sec[2] = sec[6] = NULL;
	sec[9] = NULL;			/* last valid bitmap */
	p = sec[0] + 16;

	while (sec[8] - p > 0) {
	    if (p[4] > 8) fatal_error_i("parse_1st_msg illegal section %d", (int) p[4]);
	    sec[p[4]] = p;

	    /* Section 6: bitmap */
	    if (p[4] == 6) {
		if (p[5] == 0) {
		    sec[9] = p;		/* last valid bitmap */
		}
		else if (p[5] >= 1 && p[5] <= 253) {
	            fatal_error("parse_1st_msg: predefined bitmaps are not handled","");
		}
		else if (p[5] == 254) {
	            fatal_error("parse_1st_msg: illegal grib msg, bitmap not defined code, table 6.0=254","");
		}
	    }

	    /* last section */
	    if (p[4] == 7) {
		return 0;
	    }
	    p += uint4(p);
	}
	fatal_error("parse_1st_msg illegally format grib","");
	return 1;
}

int parse_next_msg(unsigned char **sec) {

	unsigned char *p, *end_of_msg;
 
	end_of_msg = sec[0] + GB2_MsgLen(sec);
	p = sec[7];
	if (p[4] != 7) {
            fatal_error("parse_next_msg: parsing error","");
	}
	p += uint4(p);
	if (p > end_of_msg) fatal_error("bad grib fill","");

	while (p < sec[8]) {
	    sec[p[4]] = p;

	    // code to handle code table 6.0
	    if (p[4] == 6) {
		if (p[5] == 0) {
		    sec[9] = p;		/* last valid bitmap */
		}
		else if (p[5] >= 1 && p[5] <= 253) {
	            fatal_error("parse_next_msg: predefined bitmaps are not handled","");
		}
		else if (p[5] == 254) {
		    if ((sec[6] = sec[9]) == NULL) {
	                fatal_error("parse_1st_msg: illegal grib msg, bitmap not defined code, table 6.0=254","");
		    }
		}
	    }
	    if (p[4] == 7) {		// end of message .. save on sec[]
		return 0;
	    }
	    p += uint4(p);
	    if (p > end_of_msg) fatal_error("bad grib fill","");
	}
	return 1;
}
