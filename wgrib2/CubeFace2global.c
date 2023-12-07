#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#ifdef WMO_VALIDATION


/*
 * CubeFace2global.c     4/2019 Public Domain, Wesley Ebisuzaki
 *
 * A cubed sphere grib message can consist of one face or all 6 faces.
 *
 * This routine convert single faces to a 6-face format.
 * Can use to merge 1-6 faces to global cubed sphere format
 *
 * 4/2019:  v1.0 Wesley Ebisuzaki
 */

extern int decode, file_append, save_translation;
extern unsigned int nx_, ny_;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;

/*
 * HEADER:100:cubeface2global:output:2:write faces X as global cubed grid to Y: X=list of faces to exclude
 */

int f_cubeface2global(ARG2) {

    struct local_struct {
        struct seq_file out;
        unsigned char *clone_sec[9];
	unsigned int nx_, ny_;
	int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
	enum output_grib_type grib_type;
	int good_tiles[7], has_data;
	size_t size_global;
	float *global;
    };
    struct local_struct *save;

    unsigned int i, global_size;
    int tile, write_global;
    unsigned char *gds;

    if (mode == -1) {			// initialize

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));

	/* list of tiles to retain */

	for (i = 0; i <= 6; i++) {
	     save->good_tiles[i] = 1;
	}
	for (i = 0; i < strlen(arg1); i++) {
	    if (isdigit((unsigned char) arg1[i])) {
		tile = arg1[i] - '0';
                if (tile > 6) fatal_error_i("cubeface2global: invalid tile to mask %d", tile);
		save->good_tiles[tile] = 0;
	    }
	}

        if (fopen_file(&(save->out), arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }

        save_translation = decode = 1;
	save->size_global = 0;
	save->global = NULL;
	save->has_data = 0;
	init_sec(save->clone_sec);
    }
    else if (mode == -2) {		// finalize
	save = *local;
	if (save->has_data) {
	    gds = save->clone_sec[3];
            GDS_Gnom_tile(gds) = 0;
	    uint_char(save->size_global,gds+6);
            grib_wrt(save->clone_sec, save->global, save->size_global, nx_, 6*ny_, use_scale, dec_scale,
                bin_scale, wanted_bits, max_bits, grib_type, &(save->out));
	    if (flush_mode) fflush_file(&(save->out));
	}
	if (save->size_global != 0) free(save->global);
	free_sec(save->clone_sec);
	fclose_file(&(save->out));
	free(save);
    }
    else if (mode >= 0) {		// process a field
	save = *local;
	if (code_table_3_1(sec) != 60) return 0;			// not a cubed sphere grid

	gds = sec[3];
        tile = GDS_Gnom_tile(gds);
        if (tile < 0 || tile > 6) fatal_error_i("cubeface2global: invalid tile=%d", tile);
        if (save->good_tiles[tile] == 0) return 0;

        if (tile == 0) return 0;					// already global
	// will fix in later upgrade

	/* check to see if global grid has < 4GB of points */
	if (ndata > UINT_MAX/6) fatal_error("cubface2global: grid too big for global","");

	global_size = 6*ndata;
	gds = save->clone_sec[3];
	if (save->has_data) {
	    // see if data needs to be written out
	    GDS_Gnom_tile(gds) = tile;
	    if (same_sec0(save->clone_sec, sec) == 1 && 
		    same_sec1(save->clone_sec, sec) == 1 &&
		    same_sec3(save->clone_sec, sec) == 1 && 
		    same_sec4(save->clone_sec, sec) == 1) {
		write_global = 0;
	    }
	    else {
		write_global = 1;
	    }
	}
	else {
	    write_global = 0;
	}
	fprintf(stderr, " cf2g write_global %d has_data %d ", write_global,save->has_data);

	if (write_global) {			// write out old data in global
            GDS_Gnom_tile(gds) = 0;
	    uint_char(save->size_global,gds+6);
            grib_wrt(save->clone_sec, save->global, save->size_global, save->nx_, 6*save->ny_, 
		save->use_scale, save->dec_scale, save->bin_scale, save->wanted_bits, 
		save->max_bits, save->grib_type, &(save->out));
	    if (flush_mode) fflush_file(&(save->out));
	    save->has_data = 0;
	}

	if (save->has_data == 0) {		// init global
            copy_sec(sec, save->clone_sec);
	    if (global_size != save->size_global) {
	        if (save->size_global) free(save->global);
	        save->global = (float *) malloc(sizeof (float) * (size_t) global_size) ;
		if (save->global == NULL) fatal_error("cubeface2global: memory allocation","");
	  	save->size_global = global_size; 
	    }
	    for (i = 0; i < global_size; i++) {
		save->global[i] = UNDEFINED;
	    }
	    save->has_data = 1;
	    save->nx_ = nx_;
	    save->ny_ = ny_;
	    save->use_scale = use_scale;
	    save->dec_scale = dec_scale;
	    save->bin_scale = bin_scale;
	    save->wanted_bits = wanted_bits;
	    save->max_bits = max_bits;
	    save->grib_type = grib_type;
	}

	// save data in save->global in translated order

	undo_output_order(data, save->global + (tile-1)*ndata, ndata);
    }
    return 0;
}	
#else
int f_cubeface2global(ARG2) {
    fatal_error("cubeface2global: not installed","");
    return 0;	
}
#endif
