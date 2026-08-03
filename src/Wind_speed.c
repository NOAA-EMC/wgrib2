/** @file
 * @brief Calculate wind speed from U and V components. 
 * @author Public Domain: Wesley Ebisuzaki @date 3/2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
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
 * HEADER:100:wind_speed:output:1:calculate wind speed, X = output gribfile (U then V in datafile)
 */

/**
 * Takes the zonal and meridional winds, computes the wind speed and writes it out in grib 
 * format. 
 * 
 * For this option to work, the meridional wind must follow the corresponding zonal wind for 
 * that level, time and forecast hour. If the U and V in your grib file do not have this 
 * order, the file must be sorted. However, many NCEP forecast files already have this order. 
 * 
 * You can save computer time by using the -match option to restricting the decoding of records 
 * to U and V. 
 * 
 * ## Usage
 * -wind_speed output_grib_file
 * 
 * @note You can write the output of -wind_speed and -wind_dir to the same file.
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
 * $ wgrib2 wind.grb
 * 1:0:d=2009060500:WDIR:200 mb:180 hour fcst:ENS=+19
 * 2:97922:d=2009060500:WIND:200 mb:180 hour fcst:ENS=+19
 * 3:179554:d=2009060500:WDIR:250 mb:180 hour fcst:ENS=+19
 * 4:277476:d=2009060500:WIND:250 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 3/2009
 */
int f_wind_speed(ARG1) {

    struct local_struct {
        float *val;
        int has_u;
        unsigned char *clone_sec[9];
        struct seq_file out;
    };
    struct local_struct *save;

    unsigned int i;
    int is_u, is_v;
    int test_s0, test_s1, test_s3, test_s4;
    float *d1, *data_tmp;
    int discipline, mastertab, parmcat, parmnum;

    if (mode == -1) {			// initialization
        save_translation = decode = 1;

        // allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("wind_speed: memory allocation","");

        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }

        save->has_u = 0;
        init_sec(save->clone_sec);
        return 0;
    }

    save = *local;
    if (mode == -2) {			// cleanup
        if (save->has_u == 1) {
            fprintf(stderr,"WARNING: -wind_speed, unused UGRD\n");
            free(save->val);
            free_sec(save->clone_sec);
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
        // use_scale = 0;

        if (mode == 99) fprintf(stderr,"-wind_speed %d %d %d %d\n",mastertab,discipline,parmcat,parmnum);

        is_u = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 2);
        if (mode == 99 && is_u) fprintf(stderr,"\n-wind_speed: is u\n");

        if (is_u) {		// save data
            if (save->has_u) {
                fprintf(stderr,"WARNING: -wind_speed, unused UGRD\n");
                free(save->val);
                free_sec(save->clone_sec);
            }
            copy_sec(sec, save->clone_sec);
            copy_data(data,ndata,&(save->val));
            GB2_ParmNum(save->clone_sec) = 3;		// set id to V
            save->has_u = 1;
            return 0;
        }

        /* if not V return */
        is_v = (mastertab != 255) && (discipline == 0) && (parmcat == 2) && (parmnum == 3);
        if (!is_v) return 0;

        if (save->has_u == 0) {
            fprintf(stderr,"WARNING: -wind_speed, unused VGRD\n");
            return 0;
        }

        // check for correspond U and V

        test_s0 = test_s1 = test_s3 = test_s4 = 0;
        if ((test_s0 = same_sec0(sec,save->clone_sec)) == 1 &&
            (test_s1 = same_sec1(sec,save->clone_sec)) == 1 &&
            (test_s3 = same_sec3(sec,save->clone_sec)) == 1 &&
            (test_s4 = same_sec4(sec,save->clone_sec)) == 1) {

            // calculate wind speed

            d1 = save->val;
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (!UNDEFINED_VAL(data[i]) && !UNDEFINED_VAL(d1[i])) {
                    d1[i] = sqrt(data[i]*data[i] + d1[i] * d1[i]);
                }
                else d1[i] = UNDEFINED;
            }
            GB2_ParmNum(save->clone_sec) = 1;		// set id to wind speed

            // copy data to temp space

            if ((data_tmp = (float *) malloc(sizeof(float) * (size_t) ndata)) == NULL)
                fatal_error("wind_speed: memory allocation","");
            undo_output_order(save->val, data_tmp, ndata);
            grib_wrt(save->clone_sec, data_tmp, ndata, nx_, ny_, use_scale, dec_scale, 
                    bin_scale, wanted_bits, max_bits, grib_type, &(save->out));

            if (flush_mode) fflush_file(&(save->out));

            // cleanup
            free(data_tmp);
            free(save->val);
            free_sec(save->clone_sec);
            save->has_u = 0;
        }
        else {
            if (mode) {
                fprintf(stderr,"wind_speed: match failed sec0 %d sec1 %d sec3 %d sec4 %d\n",
                test_s0, test_s1, test_s3, test_s4);
            }
            fprintf(stderr,"WARNING: -wind_speed, unused VGRD, not corresponding, -v to see more\n");
        }
    }
    return 0;
}
