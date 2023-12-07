#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * import_latlon
 *
 *  routine to use lat-lon data from external file
 *
 * 2/2020: Public Domain: Wesley Ebisuzaki
  *
 */

extern int latlon, decode;
extern double *lat, *lon;
extern enum geolocation_type geolocation;

/*
 * HEADER:100:import_lonlat:misc:1:read lon-lat data from binary file
 */
int f_import_lonlat(ARG1) {
    struct seq_file *save;
    unsigned int zero, ndata_ll;
    char id[8];
    int j;
    unsigned int i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("export_latlon: memory allocation","");
        if (fopen_file(save, arg1, "r") != 0) {
            free(save);
            fatal_error("import_lonlat: Could not open %s", arg1);
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
	if (lat == NULL) {
	    lat = (double *) malloc(sizeof(double) * (size_t) ndata);
            if (lat == NULL) fatal_error_i("memory allocation error in grid_def #lat=%d", (int) ndata);
	}
	if (lon == NULL) {
	    lon = (double *) malloc(sizeof(double) * (size_t) ndata);
            if (lon == NULL) fatal_error_i("memory allocation error in grid_def #lat=%d", (int) ndata);
	}

	/* file:  'wgrib2ll ' // ndata // 0 // lon // lat */

        j = fread_file(id, sizeof(char), (size_t) 8, save);
	/* 6/2022 if EOF try from beginning of file */
	if (j == 0) {
	    if (fseek_file(save, 0l, SEEK_SET) != 0) fatal_error("import_lonlat: fseek_file error","");
            j = fread_file(id, sizeof(char), (size_t) 8, save);
	}
	if (j == 0) fatal_error("import_lonlat: empty or missing file","");
	if (j != 8) fatal_error("import_lonlat: missing header","");
	if (strncmp(id,"wgrib2ll",8) != 0) fatal_error("import_lonlat: bad header","");
	if (mode > 0) {
	    fprintf(stderr, "header=");
	    for (i = 0; i < 8; i++) fprintf(stderr,"%c", id[i]);
	    fprintf(stderr, "\n");
	}
        if (fread_file(&ndata_ll, sizeof(unsigned int) , (size_t) 1, save) != 1) fatal_error("import_lonlat: ndata","");
        if (mode > 0) fprintf(stderr, "import_lonlat: ndata=%u\n", ndata_ll);
        if (fread_file(&zero, sizeof(unsigned int), (size_t) 1, save) != 1) fatal_error("import_lonlat: bad header","");
	if (zero != 0) fatal_error("import_lonlat: zero was expected","");
	if (ndata != ndata_ll) fatal_error("import_lonlat: grid size does not match","");
	if (fread_file(&lon[0], sizeof(double), (size_t) ndata, save) != ndata) fatal_error("import_lonat:lon read","");
	if (fread_file(&lat[0], sizeof(double), (size_t) ndata, save) != ndata) fatal_error("import_lonat:lat read","");
	geolocation = external;
   }
   return 0;
}
