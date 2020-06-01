#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Undefine.c
 *
 *  Some routines that undefine grid point values for later use
 *
 * 10/2007: Public Domain: Wesley Ebisuzaki
 * 1/2008 lat and lon changed from float to double
 *
 */


extern int decode, latlon;
extern double *lat, *lon;
extern int nx_, ny_, scan;

/*
 * HEADER:100:undefine:misc:3:sets grid point values to undefined X=(in-box|out-box) Y=lon0:lon1 Z=lat0:lat1
 */

/*
 * this routine sets the data grid points to UNDEFINED
 * use: select certain regions for further processing
 * example: -stats (min/max/ave) value of defined grid points
 * spreadsheet output
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
