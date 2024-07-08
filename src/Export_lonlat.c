#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * export_latlon
 *
 *  routine to save lat-lon data to external file
 *
 * 7/2019: Public Domain: Wesley Ebisuzaki
  *
 */

extern int latlon, decode;
extern double *lat, *lon;

/*
 * HEADER:100:export_lonlat:misc:1:save lon-lat data in binary file
 */
int f_export_lonlat(ARG1) {
    struct seq_file *save;
    unsigned int zero;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("export_latlon: memory allocation","");
        if (fopen_file(save, arg1, "w") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
	latlon = 1;
	/* need to decode to get value of ndata */
	decode = 1;
    }
    else if (mode == -2) {
        save = *local;
	fclose_file(save);
    }
    else if (mode >= 0) {
        save = *local;
	if (lat != NULL && lon != NULL) {
	    /* output file:  'wgrib2ll ' // ndata // 0 // lon // lat */
	    fwrite_file("wgrib2ll", 1, 8, save);
	    fwrite_file(&ndata, sizeof(unsigned int), (size_t) 1, save);
	    zero = 0;
	    fwrite_file(&zero, sizeof(unsigned int), (size_t) 1, save);
	    fwrite_file(&(lon[0]), sizeof(double), (size_t) ndata, save);
	    fwrite_file(&(lat[0]), sizeof(double), (size_t) ndata, save);
	}
    }
   return 0;
}
