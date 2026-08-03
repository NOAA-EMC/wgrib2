/** @file
 * @brief Some routines that undefine grid point values for later use.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 10/2007 | W. Ebisuzaki | Initial
 * 1/2008 | W. Ebisuzaki | lat and lon changed from float to double
 * @author Public Domain: Wesley Ebisuzaki @date 10/2007
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Scan mode. */
extern int scan;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Number of grid points in the x-direction. */
extern unsigned int nx_;

/** Number of grid points in the y-direction. */
extern unsigned int ny_;

/*
 * HEADER:100:undefine:misc:3:sets grid point values to undefined X=(in-box|out-box) Y=lon0:lon1 Z=lat0:lat1
 */

/*
 * this routine sets the data grid points to UNDEFINED
 * use: select certain regions for further processing
 * example: -stats (min/max/ave) value of defined grid points
 * spreadsheet output
 */

/**
 * Sets the data grid points to UNDEFINED.
 * 
 * The grid points are have to be inside or outside a user defined lat-lon box. This option 
 * can be used to limit the output when writing text output. For example, you were only 
 * interested in the UK, you could use this option to undefine the grid points outside the 
 * of UK. Then when you write the data in spread-sheet format, you would get a much smaller 
 * output. This option can also be used to find the regional average using the stat option. 
 * 
 * ## Usage
 * -undefine (in-box|out-box) lon0:lon1 lat0:lat1
 * 
 * in-box:  decoded grid points inside the box are set to undefined
 * out-box: decoded grid points outside the box are set to undefined
 * lon0:lon1  west-east longitudes of the box
 * lat0:lat1  south-north latitudes of the box
 * 
 * Points on the box boundary are considered to be in the box.
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 test.grb2 -undefine out-box 350:10 -10:10 -stats
 * 1:0:ndata=65160:undef=64719:mean=94.1229:min=58.1:max=125.2
 * @endcode
 * 
 * The above line calculates the statistics for the box -10W-10E 10S-10N 
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -undefine out-box 10:30 20:40 -undefine in-box 12:28 22:38 -bin boundary.bin
 * @endcode
 * 
 * The above line undefines the grid points outside of a box and then undefines the grid points of a smaller 
 * box that is contained in the first box. Then it writes the data as a binary file. The data file contains the 
 * data points for a perimeter of a box. Why would someone want to do that? Think "horizontal boundary conditions 
 * for a regional model". For this to work well, a module to write the data out in grib-2 needs to be written. 
 * To work in the (i,j) coordinates, see the -ijundefine option. 
 * 
 * @author Wesley Ebisuzaki @date 10/2007
 */
int f_undefine(ARG3) {
    struct local_struct {
        int in;
        double lon0, lon1, lat0,lat1;
    };
    struct local_struct *save;
    double x,y;
    unsigned int i;

    if (mode == -1) {
        decode = latlon = 1;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_undefine","");

        if (strcmp("in-box", arg1) && strcmp("out-box", arg1)) {
            fatal_error("f_undefine expecting in-box/out-box not %s",arg1);
        }
        save->in =(strcmp("in-box", arg1) == 0);

        if (sscanf(arg2,"%lf:%lf", &x, &y) != 2) {
            fatal_error("undef: bad longitudes lon0:lon1  %s", arg2);
        }
        if (x > y) y += 360.0;
        if (x > y) fatal_error("undef: bad longitudes lon0:lon1  %s", arg2);
        if (y-360 > x ) fatal_error("undef: bad longitudes lon1 too big %s", arg2);
        
        if (x < 0.0) { x += 360.0; y += 360.0; }

        save->lon0 = x;
        if (y < x) y += 360.0;
        if (y < x) y += 360.0;
        save->lon1 = y;

        if (sscanf(arg3,"%lf:%lf", &x, &y) != 2) {
            fatal_error("undef: bad latitudes lat0:lat1  %s", arg3);
        }
        if (x > y) fatal_error("undef: bad latitudes lat0 > lat1  %s", arg3);
        save->lat0 = x;
        save->lat1 = y;
    }
    else if (mode == -2) {
        free(*local);
    }

    if (mode < 0) return 0;
    save = (struct local_struct *) *local;

    if (lat == NULL || lon == NULL) {
        fprintf(stderr,"f_undef does nothing, no lat-lon information\n");
        return 0;
    }

    if (save->in == 0) {
        for (i = 0; i < ndata; i++) {
            x = lon[i];
            if (x < save->lon0) x += 360.0;
            if (x > save->lon1 || lat[i] < save->lat0 || lat[i] > save->lat1)
                data[i] = UNDEFINED;
        }
    }
    else {
        for (i = 0; i < ndata; i++) {
            x = lon[i];
            if (x < save->lon0) x += 360.0;
            if (x <= save->lon1 && lat[i] >= save->lat0 && lat[i] <= save->lat1)
                data[i] = UNDEFINED;
        }
    }

    return 0;
}

/*
 * HEADER:100:ijundefine:misc:3:sets grid point values to undefined X=(in-box|out-box) Y=ix0:ix1 Z=iy0:iy1  ix=(1..nx) iy=(1..ny)
 */

/* this routine sets the data grid points to UNDEFINED
 * use: select certain regions for further processing
 * example: -stats (min/max/ave) value of defined grid points
 * spreadsheet output uses i,j coordinates, i = 1..nx j = 1..iy;
 */

/**
 * Sets the selected grid values to undefined. The grid points are have to be inside or outside 
 * a user defined (i,j) box. I and j are the column and row number of the "raw" data starting 
 * from 1. This option can be used to limit the output when writing output. For example, you were 
 * only interested in the UK, you could use this option to undefine the grid points outside the 
 * of UK. Then when you write the data in spread-sheet format, you would get a much smaller output. 
 * This option can also be used to find the regional average using the stat option. Note that the 
 * -ijundefine option changes the in-memory values of the grid points. If you want to alter the 
 * grib file, you will have to write out the in-memory grid point values using the the -grib_out 
 * option. 
 * 
 * ## Usage
 * -ijundefine (in-box|out-box) ix0:ix1 iy0:iy1
 * 
 * in-box:  decoded grid points inside the box are set to undefined
 * out-box: decoded grid points outside the box are set to undefined
 * ix0:ix1 columns limits (1 <=  ix0 <= ix1 < nx)
 * iy0:iy1 row limits (1 <= iy0 <= iy1 <  ny)
 * Note: the order of the data should be in default mode (we:sn).
 * Note: ix0, iy0 is the lower left hand corner (w/s).
 * 
 * Points on the box boundary are considered to be in the box.
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 test.grb2 -ijundefine out-box 1:1 1:10 -stats
 * @endcode
 * 
 * The above line calculates the statistics for ix=1, iy=1..10. 
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -ijundefine out-box 10:30 20:40 -ijundefine in-box 11:29 21:39 -bin boundary.bin
 * @endcode
 * 
 * The above line undefines the grid points outside of a box and then undefines the grid points of a smaller 
 * box that is contained in the first box. Then it writes the data as a binary file. The data file contains 
 * the data points for a perimeter of a box. Why would someone want to do that? Think "horizontal boundary 
 * conditions for a regional model". For this to work well, a module to write the data out in grib-2 needs to 
 * be written. BTW the binary file will compress to an extremely small file. 
 * 
 * @author Wesley Ebisuzaki @date 10/2007
 */
int f_ijundefine(ARG3) {
    struct local_struct {
        int in;
        unsigned int ix0, ix1, iy0, iy1;
    };
    struct local_struct *save;
    long unsigned int x,y;
    unsigned int i, ix, iy, nyy, nxx;
    unsigned int x0, x1, y0, y1;

    if (mode == -1) {
        decode = 1;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_undefine","");

        if (strcmp("in-box", arg1) && strcmp("out-box", arg1)) {
            fatal_error("f_undefine expecting in-box/out-box not %s",arg1);
        }
        save->in =(strcmp("in-box", arg1) == 0);

        if (sscanf(arg2,"%lu:%lu", &x, &y) != 2) {
            fatal_error("undef: bad ix0:ix1  %s", arg2);
        }

        save->ix0 = x-1;
        save->ix1 = y-1;

        if (sscanf(arg3,"%lu:%lu", &x, &y) != 2) {
            fatal_error("undef: bad iy0:iy0  %s", arg3);
        }
        save->iy0 = x-1;
        save->iy1 = y-1;
    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;

    save = (struct local_struct *) *local;
    x0 = save->ix0;
    x1 = save->ix1;
    y0 = save->iy0;
    y1 = save->iy1;

    if (GDS_Scan_staggered(scan)) fatal_error("ijundefine does not support staggered grids","");

    nxx = nx_ > 0 ? nx_ : 1;
    nyy = ny_ > 0 ? ny_ : 1;

    if (save->in == 0) {
        i = 0;
        for (iy = 0; iy < nyy; iy++) {
            for (ix = 0; ix < nxx; ix++) {
                if (ix < x0 || ix > x1 || iy < y0 || iy > y1) data[i] = UNDEFINED;
                i++;
            }
        }
    }
    else {
        i = 0;
        for (iy = 0; iy < nyy; iy++) {
            for (ix = 0; ix < nxx; ix++) {
                if (ix >= x0 && ix <= x1 && iy >= y0 && iy <= y1) data[i] = UNDEFINED;
                i++;
            }
        }
    }
    return 0;
}

/*
 * HEADER:100:undefine_val:misc:1:grid point set to undefined if X=val or X=low:high
 */ 

/**
 * Sets the grid points to undefined depending on the value of the grid point. If a single value 
 * is specified, grid values within 0.1 percent are set to undefined. If two values are specified, 
 * the values that are within that range are set to undefined. 
 * 
 * Note: the ability to handle ranges was always available but undocumented. My mistake. That's 
 * what you get when you delay writing the documentation. 
 * 
 * ## Usage
 * -undefine_val VALUE
 *
 * Grid values within 0.1 percent of VALUE are set to undefined.
 *
 * -undefine_val "VALUE1:VALUE2"
 * Grid values that are within the range are set to undefined. (i.e., VALUE1 <= grid_value <= VALUE2)
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2007
 */
int f_undefine_val(ARG1) {

#define DELTA 0.001

    struct local_struct {
        double val_low, val_high;
    };
    struct local_struct *save;
    
    double val;
    unsigned int i;
    int j;

    if (mode == -1) {
        decode = 1;
        *local = save = (struct local_struct *) malloc(sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_undefine_val","");
   
        j = sscanf(arg1,"%lf:%lf", &(save->val_low), &(save->val_high));
        if (j != 2) {
            val = atof(arg1);
            if (val >= 0.0) {
                save->val_low = val * (1.0-DELTA);
                save->val_high = val * (1.0+DELTA);
            }
            else {
                save->val_low = val * (1.0+DELTA);
                save->val_high = val * (1.0-DELTA);
            }
        }
    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;

    save = (struct local_struct *) *local;
    if (data == NULL) fatal_error("undefine_val: grid not decoded","");
    for (i = 0 ; i < ndata; i++) {
        if (data[i] >= save->val_low  && data[i] <= save->val_high)
                data[i] = UNDEFINED;
    }
    return 0;
}
