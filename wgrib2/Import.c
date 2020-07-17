#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Import.c
 *
 *  Some routines to read data into data buffer
 *
 * 10/2008: Public Domain: Wesley Ebisuzaki
  *
 */

extern int decode;
extern int nx, ny, use_scale;
extern unsigned int nx_, ny_;
extern int ieee_little_endian, header;

/*
 * HEADER:100:import_text:misc:1:read text file (X) for data
 */
int f_import_text(ARG1) {
    unsigned int ix, iy;
    unsigned int i;
    float t;

    if (mode == -1) {
        if ((*local = (void *) ffopen(arg1, "r")) == NULL) 
            fatal_error("import_text: Could not open %s", arg1);
        decode = 1; 
    }
    else if (mode == -2) {
	ffclose((FILE *) *local);
    }
    else if (mode >= 0) {
	if (header) {
	    if (fscanf((FILE *) *local, "%u %u", &ix, &iy) != 2) {
                fatal_error("import_text: Could not read nx, ny in file %s", arg1);
            }
	    if (nx_ != ix) {
                fatal_error_u("import_text: nx=%u is wrong",ix);
	    }
	    if (ny_ != iy) {
                fatal_error_u("import_text: ny=%u is wrong",iy);
	    }
	}
	for (i = 0; i < ndata; i++) {
	    if (fscanf((FILE *) *local, "%f", &t) != 1)
                fatal_error_u("import_text: Could not read data. location=%u",i+1);
	    data[i] = t;
        }
        use_scale = 0;
    }
    return 0;
}

/*
 * HEADER:100:import_ieee:misc:1:read ieee file (X) for data
 */
int f_import_ieee(ARG1) {
    int i;
    struct seq_file *save;
    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("import_ieee: memory allocation","");
        if (fopen_file(save, arg1, "rb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        decode = 1;
    }
    else if (mode == -2) {
	save = (struct seq_file *) *local;
	fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = (struct seq_file *) *local;
	i = rdieee_file(data, ndata, header, save);
        if (i) fatal_error_i("import_ieee, error %d", i);
        use_scale = 0;
    }
    return 0;
}

/*
 * HEADER:100:import_bin:misc:1:read binary file (X) for data
 */

int f_import_bin(ARG1) {
    unsigned int i, j;
    struct seq_file *save;
    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("import_bin: memory allocation","");
        if (fopen_file(save, arg1, "rb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        decode = 1;
    }
    else if (mode == -2) {
        save = (struct seq_file *) *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = (struct seq_file *) *local;
        if (header) {
            if (fread_file((void *) &i, sizeof(unsigned int), 1, save) != 1) {
        	fclose_file(save);
        	free(save);
                fatal_error("import_bin: read error header (lack of data)","");
	    }
            if (i != sizeof(float) * (size_t) ndata) {
        	fclose_file(save);
        	free(save);
                fatal_error("import_bin: header record size wrong","");
	    }
        }
        j = fread_file(data, sizeof(float), ndata, save);
        if (j != ndata) fatal_error_uu("import_bin: read error read %u words, expected %u", j, ndata);
        if (header) {
            if (fread_file((void *) &i, sizeof(int), 1, save) != 1)
                fatal_error("import_bin: read error trailer","");
            if (i != sizeof(float) * ndata)
                fatal_error_u("import_bin: trailer record size wrong, %u", i);
        }
        use_scale = 0;                  // new field, unknown scaling
    }
    return 0;
}
