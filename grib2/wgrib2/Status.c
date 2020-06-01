#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Status: provide some status of the internal state of wgrib2
 *
 *  v 0.1 experimental
 *
 * 4/2015: Public Domain: Wesley Ebisuzaki
 */

extern int file_append, flush_mode;


/*
 * HEADER:100:status:misc:1:X  X=file
 */



int f_status(ARG1) {

    if (strncmp(arg1,"file",4) == 0) {
	fprintf(stderr,"mode=%d\n", mode);
	fprintf(stderr,"flush mode=%d\n", flush_mode);
	// inv_out += strlen(inv_out);
	fprintf(stderr,"file_append=%d\n", file_append);
	// inv_out += strlen(inv_out);
	status_ffopen();
    }
    return 0;

}
