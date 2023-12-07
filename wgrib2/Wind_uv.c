#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Wind_uv - converts from wind speed/direction to UGRD/VGRD
 *
 *  v 0.1 experimental based on Wind_speed.c
 *
 * 3/2009: Public Domain: Wesley Ebisuzaki (wind_speed.c)
 * 4/2019: Public Domain: Wesley Ebisuzaki (wind_uv.c)
 *
 */

extern int decode, file_append, save_translation;
extern unsigned int nx_, ny_;
extern int flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;

extern enum output_grib_type grib_type;

/*
 * HEADER:100:wind_uv:output:1:calculate UGRD/VGRD from speed/dir, X = output gribfile
 */


int f_wind_uv(ARG1) {

    struct local_struct {
        float *dir;
        float *speed;
        int has_dir, has_speed;
        int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
	enum output_grib_type grib_type;
        unsigned char *clone_sec_dir[9];
        unsigned char *clone_sec_speed[9];
        struct seq_file out;
    };
    struct local_struct *save;

    unsigned int i;
    int is_dir, is_speed;
    float *data_tmp, u, v;
    int discipline, mastertab, parmcat, parmnum;

    if (mode == -1) {			// initialization
        save_translation = decode = 1;

	// allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation -wind_dir","");

        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
	save->has_dir = save->has_speed = 0;
	init_sec(save->clone_sec_speed);
	init_sec(save->clone_sec_dir);
	return 0;
    }
    save = *local;
    if (mode == -2) {			// cleanup
	if (save->has_dir == 1) {
	    fprintf(stderr,"WARNING: -wind_uv, unused WDIR\n");
	    free(save->dir);
	    free_sec(save->clone_sec_dir);
	}
	if (save->has_speed == 1) {
	    fprintf(stderr,"WARNING: -wind_uv, unused WIND\n");
	    free(save->speed);
	    free_sec(save->clone_sec_speed);
	}
	fclose_file(&(save->out));
	free(save);
	return 0;
    }

    if (mode >= 0) {			// processing

	// get variable name parameters

        discipline = GB2_Discipline(sec);
        mastertab = GB2_MasterTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);

	if (mode == 99) fprintf(stderr,"wind_uv %d %d %d %d\n",mastertab,discipline,parmcat,parmnum);

	is_dir = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 0);
	is_speed = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 1);

	if (is_dir) {		// save data
	    if (save->has_dir) {
	        fprintf(stderr,"WARNING: -wind_uv, unused WDIR\n");
	        free(save->dir);
	        free_sec(save->clone_sec_dir);
	    }
            copy_sec(sec, save->clone_sec_dir);
	    copy_data(data,ndata,&(save->dir));
            GB2_ParmNum(save->clone_sec_dir) = 1;		// set id to speed
	    save->has_dir = 1;
	}
	if (is_speed) {		// save data
	    if (save->has_speed) {
	        fprintf(stderr,"WARNING: -wind_uv, unused WIND\n");
	        free(save->speed);
	        free_sec(save->clone_sec_speed);
	    }
            copy_sec(sec, save->clone_sec_speed);
	    copy_data(data,ndata,&(save->speed));
	    save->has_speed = 1;
	    save->use_scale = use_scale;
	    // save->use_scale = 0;
	    save->dec_scale = dec_scale;
	    save->bin_scale = bin_scale;
	    save->wanted_bits = wanted_bits;
	    save->max_bits = max_bits;
	    save->grib_type = grib_type;
	}

        if (save->has_speed == 0 || save->has_dir == 0)  return 0;

	// check for if dir and speed match

        if (same_sec0(save->clone_sec_dir, save->clone_sec_speed) == 1 &&
            same_sec1(save->clone_sec_dir, save->clone_sec_speed) == 1 &&
            same_sec3(save->clone_sec_dir, save->clone_sec_speed) == 1 &&
            same_sec4(save->clone_sec_dir, save->clone_sec_speed) == 1) {

	    // calculate wind U and V

	    if (mode == 99) fprintf(stderr,"\n-wind_dir: calc wind U/V\n");

            if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("wind_uv: memory allocation","");

#pragma omp parallel for private(i,u,v)
	    for (i = 0; i < ndata; i++) {
                if (!UNDEFINED_VAL(save->speed[i])) {
		    if (save->speed[i] == 0.0) {
			// wind dir may not be defined .. NOAA way
		        save->dir[i] = 0.0;
		        save->speed[i] = 0.0;
		    }
                    else if (!UNDEFINED_VAL(save->dir[i])) {
		        u = -save->speed[i]*sin(save->dir[i] * 3.14159265359/180.0);
		        v = -save->speed[i]*cos(save->dir[i] * 3.14159265359/180.0);
		        save->dir[i] = u;
		        save->speed[i] = v;
		    }
		    else {
		        save->dir[i] = save->speed[i] = UNDEFINED;
		    }
		}
		else {
		    save->dir[i] = save->speed[i] = UNDEFINED;
		}
	    }

            GB2_ParmNum(save->clone_sec_dir) = 2;		// set id to U
	    undo_output_order(save->dir, data_tmp, ndata);
            grib_wrt(save->clone_sec_dir, data_tmp, ndata, nx_, ny_, save->use_scale, save->dec_scale, 
		save->bin_scale, save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

            GB2_ParmNum(save->clone_sec_dir) = 3;		// set id to V
	    undo_output_order(save->speed, data_tmp, ndata);
            grib_wrt(save->clone_sec_dir, data_tmp, ndata, nx_, ny_, save->use_scale, save->dec_scale, 
		save->bin_scale, save->wanted_bits, save->max_bits, save->grib_type, &(save->out));

            if (flush_mode) fflush_file(&(save->out));

            // cleanup

            free(data_tmp);
            free(save->dir);
            free(save->speed);
            free_sec(save->clone_sec_dir);
            free_sec(save->clone_sec_speed);
	    save->has_speed = save->has_dir = 0;
	}
    }
    return 0;
}
