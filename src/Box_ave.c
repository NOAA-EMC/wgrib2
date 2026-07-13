/** @file
 * @brief Smoothing of a field by doing a box average
 * @author Public Domain: Wesley Ebisuzaki @date 3/2018
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Boxave.c
 *
 * smoothing of a field by doing a box average
 *
 * 3/2018: Public Domain: Wesley Ebisuzaki
 *
 * critical_weight:
 *   if critical_weight == -1
 *        if (data[i][j] == UNDEFINED) then
 *            data[i][j] = UNDEFINED
 *           else
 *            data[i][j] = cell_average(data)
 *           endif
 *
 *   if critical_weight != -1
 *           if number of defined values in cell > critical_weight then
 *            data[i][j] = cell_average(data)
 *           else
 *            data[i][j] = UNDEFINED
 *           endif
 *             
 */

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

/** Scan mode flag. */
extern int scan;

/** Number of grid points in the x-direction. */
extern unsigned int nx_;

/** Number of grid points in the y-direction. */
extern unsigned int ny_;

/*
 * HEADER:000:box_ave:misc:3:box average X=odd integer (lon) Y=odd integer (lat) critical_weight
 */

/**
 * Does a spatial smoothing by doing a simple box average of the data field.
 * Amount of smoothing can be controlled by the size of the box. Can be used on regional
 * and global fields. To identify global fields, you can use the option -cyclic.
 * 
 * ## Usage
 * -box_ave DX DY CRITICAL_WEIGHT
 * 
 * DX=width of box (in grid points), DX has to be an odd positive integer
 * DY=height of box (in grid points), DY has to be an odd positive integer
 * 
 * The box average is the mean value for a box of DX x DY centered on the grid point.
 *
 * CRITICAL_WEIGHT
 *    -1: grid(i,j) = UNDEFINED    if original grid(i,j) is undefined
 *                    = box average  if original grid(i,j) is defined
 *    not -1: let wt = number of grid points that are defined in the box
 *        grid(i,j) = UNDEFINED     if wt <= WT
 *          = box average   if wt > WT
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @note The speed of -box_ave is O(NX*NY*DY). The O(NX*NY) method was slower because of 
 * poor cache utilization and false sharing.
 * 
 * ## Example:
 * 
 * I had a high-resolution Gaussian grid and wanted to convert it to a 1x1 degree grid. There were about 
 * 81 grid points in a 1 degree cell. The budget interpolation in -new_grid worked but it was slow and 
 * worked by taking 25 bilinear interpolations and averaging them to make the budget interpolation. So 
 * the pre-existing solution was slow and slighly inaccurate. To interpolate scalars to the 1x1 grid, you 
 * can run a box_average with 9x9 grid and then use -new_grid to get the cell average values. This method 
 * is, as expected, slightly smoother than the budget interpolation of -new_grid. For vectors, you have to 
 * use the budget interpolation of the -new_grid option.
 * 
 * @author Wesley Ebisuzaki @date 3/2018
 */
int f_box_ave(ARG3) {

    unsigned int i,j,k, j0, j1, m;
    int nxx, nyy, tmpwt, *xwt, crit_wt, is_cyclic;
    double tmpsum, *xsum;

    if (mode == -1) {
        decode = 1;
    }
    if (mode < 0) return 0;

    /* processing mode */

    if (nx_ == 0  || ny_ == 0) {
        fprintf(stderr,"box_ave: not done nx=%u, ny=%u\n", nx_, ny_);
        return 0;
    }
    if (GDS_Scan_staggered(scan)) {
        fprintf(stderr,"box_ave: not done, staggered grid\n");
        return 0;
    }
    if ((scan >> 4) != 0 && (scan >> 4) != 4) {
        fprintf(stderr,"box_ave: not done, need we:sn or we:ns grids\n");
        return 0;
    }

    is_cyclic = cyclic(sec);
    if (mode) fprintf(stderr,"box_ave: cyclic=%d\n",is_cyclic);

    nxx = atoi(arg1);
    nyy = atoi(arg2);
    if (nxx <= 0 || (nxx & 1) == 0) fatal_error("box_ave: bad arg1 %s", arg1);
    if (nyy <= 0 || (nyy & 1) == 0) fatal_error("box_ave: bad arg2 %s", arg2);
    if ((unsigned int) nxx >= nx_) fatal_error("box_ave: arg1 >= nx","");
    if ((unsigned int) nyy >= ny_) fatal_error("box_ave: arg2 >= ny","");

    nxx = nxx / 2;
    nyy = nyy / 2;
    crit_wt = atoi(arg3);

    xsum = (double *) malloc(sizeof(double) * (size_t) ndata);
    xwt = (int *) malloc(sizeof(int) * (size_t) ndata);
    if (xsum == NULL || xwt == NULL) fatal_error("box_ave: memory allocation","");

#ifdef USE_OPENMP
#pragma omp parallel
    {
#pragma omp for private(i,j,k,tmpwt,tmpsum,m) schedule(static)
#endif
        for (j = 0; j < ny_; j++) {

            /* do ix == 0 */

            tmpwt = 0;
            tmpsum = 0.0;
            k = j * nx_;
            for (i=0; i <= nxx; i++) {
                if (DEFINED_VAL(data[i+k])) {
                    tmpwt++;
                    tmpsum += data[i+k];
                }
	        }
            if (is_cyclic) {
                for (i=nx_ - nxx; i < nx_; i++) {
                    if (DEFINED_VAL(data[i+k])) {
                        tmpwt++;
                        tmpsum += data[i+k];
                    }
                }
            }
            xwt[k] = tmpwt;
            xsum[k] = tmpsum;

            /* do ix = 1 .. nx_-1 */
            
            for (i = 1; i < nx_; i++) {

                /* remove old value data[k+i+1-nxx */
                    
                if (i-1 >= nxx) {
                    m = i - 1 - nxx;
                    if (DEFINED_VAL(data[m+k])) {
                        tmpwt--;
                        tmpsum -= data[m+k];
                    }
                }
                else if (is_cyclic) {
                    m = nx_ - (nxx - i + 1);
                    if (DEFINED_VAL(data[m+k])) {
                        tmpwt--;
                        tmpsum -= data[m+k];
                    }
                }

                /* add new value data[k+i+nxx]*/

                if (i < nx_ - nxx) {
                    m = i + nxx;
                    if (DEFINED_VAL(data[m+k])) {
                        tmpwt++;
                        tmpsum += data[m+k];
                    }
                }
                else if (is_cyclic) {
                    m = nxx - (nx_- i);
                    if (DEFINED_VAL(data[m+k])) {
                        tmpwt++;
                        tmpsum += data[m+k];
                    }
                }
                xwt[k+i] = tmpwt;
                xsum[k+i] = tmpsum;

            }
        }
            /* at this point xsum, xwt is calculated for entire grid 

            could use the same technique to add up the xsum values but
            1. probably not cache friendly for large ny_
            2. probably have some false cache sharing

            To avoid 1 and 2, make j the outer loop variable */

#ifdef USE_OPENMP
#pragma omp for private(i,j,j0,j1,k,tmpwt,tmpsum) schedule(static)
#endif
        for (j = 0; j < ny_; j++) {
            j0 = j > nyy ? j-nyy : 0;
            j1 = j < ny_ - nyy ? j + nyy : ny_-1;
            for (i = 0; i < nx_; i++) {
                tmpwt = 0;
                tmpsum = 0.0;
                for (k = j0; k <= j1; k++) {
                    if (xwt[i+k*nx_] > 0) {
                        tmpwt += xwt[i+k*nx_];
                        tmpsum += xsum[i+k*nx_];
                    }
                }
                if (crit_wt == -1) {
                    data[i+j*nx_] = DEFINED_VAL(data[i+j*nx_]) ? tmpsum / tmpwt : UNDEFINED;
                }
                else {
                    data[i+j*nx_] = tmpwt > crit_wt ? tmpsum / tmpwt : UNDEFINED;
                }
            }
        }
#ifdef USE_OPENMP
    }
#endif
    free(xsum);
    free(xwt);
    return 0;
}
