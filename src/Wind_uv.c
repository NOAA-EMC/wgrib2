/** @file
 * @brief Converts from wind speed/direction to UGRD/VGRD and writes it to a GRIB file.
 * Based on Wind_speed.c
 * @author Public Domain: Wesley Ebisuzaki @date 4/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "wmath.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

/** Number of grid points in the x direction. */
extern unsigned int nx_;

/** Number of grid points in the y direction. */
extern unsigned int ny_;

/** Flush of output flag. */
extern int flush_mode;

/** Use scaling flag. */
extern int use_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/** Number of bits wanted. */
extern int wanted_bits;

/** Maximum number of bits. */
extern int max_bits;

/** Current output GRIB type. */
extern enum output_grib_type grib_type;

/*
 * HEADER:100:wind_uv:output:1:calculate UGRD/VGRD from speed/dir, X = output gribfile
 */

/**
 * Takes the wind speed and wind direction, computes the zonal (UGRD) and meridional (VGRD) 
 * wind components, then writes out the wind components in grib format.
 * 
 * If the wind direction is earth (grid) relative, the UGRD and VGRD winds are earth (grid) 
 * relative. 
 * 
 * You can save computer time by using the -match option to restricting the decoding of 
 * records to wind speed (WIND) and wind direction (WDIR). 
 * 
 * ## Usage
 * -wind_uv output_grib_file
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180 -wind_dir wind.grb -wind_speed wind.grb -match "(UGRD|VGRD)"
 * 4.1:86046:d=2009060500:UGRD:200 mb:180 hour fcst:ENS=+19
 * 4.2:86046:d=2009060500:VGRD:200 mb:180 hour fcst:ENS=+19
 * 8.1:226831:d=2009060500:UGRD:250 mb:180 hour fcst:ENS=+19
 * 8.2:226831:d=2009060500:VGRD:250 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 wind.grb -wind_uv uv.grb
 * 1:0:d=2009060500:WDIR:200 mb:180 hour fcst:ENS=+19
 * 2:97922:d=2009060500:WIND:200 mb:180 hour fcst:ENS=+19
 * 3:179554:d=2009060500:WDIR:250 mb:180 hour fcst:ENS=+19
 * 4:277476:d=2009060500:WIND:250 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 uv.grb
 * 1:0:d=2009060500:UGRD:200 mb:180 hour fcst:ENS=+19
 * 2:89777:d=2009060500:UGRD:200 mb:180 hour fcst:ENS=+19
 * 3:179554:d=2009060500:UGRD:250 mb:180 hour fcst:ENS=+19
 * 4:269331:d=2009060500:UGRD:250 mb:180 hour fcst:ENS=+19
 * @endcode
 * @author Wesley Ebisuzaki @date 4/2019
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

#ifdef USE_OPENMP
#pragma omp parallel for private(i,u,v)
#endif
            for (i = 0; i < ndata; i++) {
                if (!UNDEFINED_VAL(save->speed[i])) {
                    if (save->speed[i] == 0.0) {
                    // wind dir may not be defined .. NOAA way
                        save->dir[i] = 0.0;
                        save->speed[i] = 0.0;
                    }
                    else if (!UNDEFINED_VAL(save->dir[i])) {
                        u = -save->speed[i]*sin(save->dir[i] * DEG_TO_RAD);
                        v = -save->speed[i]*cos(save->dir[i] * DEG_TO_RAD);
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
