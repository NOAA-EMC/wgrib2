/** @file
 * @brief Inventory options for lat-lon values.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 12/2006 | W. Ebisuzaki | Initial
 * 1/2007 | M. Schwarb | Cleanup
 * 1/2008 | W. Ebisuzaki | lat and lon changed from float to double
 * 1/2011 | W. Ebisuzaki | replace new_GDS by GDS_change_no
 *
 * @author Public Domain: Wesley Ebisuzaki @date 12/2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Output file needed flag. */
extern int need_output_file;

/** Current output order type. */
extern enum output_order_type output_order;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Number of keys. */
extern int WxNum;

/** NDFD local section 2 keys. */
extern int WxText;

/** Scan order mode. */
extern int scan;

/** Number of grid points in the x direction */
extern int nx;

/** Number of grid points in the y direction */
extern int ny;

/** GDS change number. */
extern int GDS_change_no;

/** Number of grid points. */
extern unsigned int npnts;


/*
 * HEADER:100:ij:inv:2:value of field at grid(X,Y) X=1,..,nx Y=1,..,ny (WxText enabled)
 */

/**
 * Prints the value of the grid point (i,j) where i = 1..nx and j = 1..ny. 
 * 
 * Note, by default the grid is converted to a WE:SN order. This means that (1,1) is at the South-West corner.
 * 
 * ## Usage
 * -ij i j 
 * 
 * i = 1 .. nx
 * j = 1 .. ny
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_ij(ARG2) {
    struct local_struct {
        int ix, iy, iptr, last_GDS_change_no;
    };
    struct local_struct *save;

    if (mode == -1) {
        WxText = decode = 1;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_ij","");
        save->ix = atoi(arg1);
        save->iy = atoi(arg2);
        save->iptr = -1;
        save->last_GDS_change_no = 0;
    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;

    save = (struct local_struct *) *local;

//    if (new_GDS) {
    if (save->last_GDS_change_no != GDS_change_no) {
        save->last_GDS_change_no = GDS_change_no;
        if (output_order != wesn) fatal_error("ij only works in we:sn order","");
        if (GDS_Scan_staggered(scan)) fatal_error("ij does not support staggered grids","");
    
        if (save->ix <= 0 || save->ix > nx || save->iy <= 0 || save->iy > ny) {
            fatal_error("invalid i, j values","");
        }
        save->iptr = (save->ix-1) + (save->iy-1) *nx;
    }

    if (mode > 0) {
        if (WxNum > 0) sprintf(inv_out,"(%d,%d),val=\"%s\"",save->ix,save->iy,WxLabel(data[save->iptr]));
        else sprintf(inv_out,"(%d,%d),val=%lg",save->ix,save->iy,data[save->iptr]);
    }
    else {
        if (WxNum > 0) sprintf(inv_out,"val=\"%s\"",WxLabel(data[save->iptr]));
        else sprintf(inv_out,"val=%lg",data[save->iptr]);
    }

    return 0;
}


/*
 * HEADER:100:ijlat:inv:2:lat,lon and grid value at grid(X,Y) X=1,..,nx Y=1,..,ny (WxText enabled)
 */

/**
 * Prints the latitude, longitude and value of the grid point (i,j) where i = 1..nx and j = 1..ny.
 * 
 * Note, by default the grid is converted to a WE:SN order which puts (1,1) in the South-West corner. 
 * Note, multiple -ijlat can be on the command line. 
 * 
 * ## Usage
 * -ijlat i j
 * 
 * i = 1 .. nx
 * j = 1 .. ny
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_ijlat(ARG2) {

    struct local_struct {
        int ix, iy, iptr, last_GDS_change_no;
    };
    struct local_struct *save;

    if (mode == -1) {
        WxText = decode = latlon = 1;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_ijlat","");
        save->ix = atoi(arg1);
        save->iy = atoi(arg2);
        save->iptr = -1;
        save->last_GDS_change_no = 0;
    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;
    save = (struct local_struct *) *local;

//    if (new_GDS) {
    if (save->last_GDS_change_no != GDS_change_no) {
        save->last_GDS_change_no = GDS_change_no;
        if (output_order != wesn) fatal_error("ijlat only works in we:sn order","");
        if (GDS_Scan_staggered(scan)) fatal_error("ijlat does not support staggered grids","");
        if (lat == NULL || lon == NULL || data == NULL) {
            fatal_error("ijlat: found no lat/lon","");
        }

        if (save->ix <= 0 || save->ix > nx || save->iy <= 0 || save->iy > ny) {
            /* fprintf(stderr," nx=%d ny=%d ix=%d iy=%d\n",nx,ny,save->ix,save->iy); */
            fatal_error("ijlat: invalid i, j values","");
        }
        save->iptr = (save->ix-1) + (save->iy-1) * nx;
    }
//vsm_fmt    sprintf(inv_out,"(%d,%d),lon=%g,lat=%g,val=%lg",save->ix,save->iy,
    if (WxNum > 0) 
        sprintf(inv_out,"(%d,%d),lon=%lf,lat=%lf,val=\"%s\"",save->ix,save->iy,
                lon[save->iptr],lat[save->iptr],WxLabel(data[save->iptr]));
    else 
        sprintf(inv_out,"(%d,%d),lon=%lf,lat=%lf,val=%lg",save->ix,save->iy,
                lon[save->iptr],lat[save->iptr],data[save->iptr]);
    return 0;
}

/*
 * HEADER:100:ilat:inv:1:lat,lon and grid value at Xth grid point, X=1,..,npnts (WxText enabled)
 */

/**
 * Prints the latitude, longitude and value of ith grid point.
 * 
 * For example, you get a text output and you were curious about the lat-lon of the fifth grid on the 
 * list. You would use this option to get the value and its location.
 * 
 * The -ilat option uses the Fortran convention which has the index starting from one. In addition, the 
 * index is for the output grid may differ from the input grid. 
 * 
 * ## Usage
 * -ilat i
 * 
 * i = 1 .. number of grid points
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_ilat(ARG1) {

    struct local_struct {
        int last_GDS_change_no;
        unsigned int ix;
    };

    struct local_struct *save;
    unsigned int i;

    if (mode == -1) {
        WxText = decode = latlon = 1;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_ilat","");
        save->ix = atoi(arg1);
        save->last_GDS_change_no = 0;
    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;

    save = (struct local_struct *) *local;
    i = save->ix;

//    if (new_GDS) {
    if (save->last_GDS_change_no != GDS_change_no) {
        save->last_GDS_change_no = GDS_change_no;
        if (output_order != wesn) {
            fatal_error("ilat only works in we:sn order","");
        }
        if (lat == NULL || lon == NULL || data == NULL) {
            fatal_error("no lat/lon in ilat","");
        }
        if (i < 1 || i > (int) npnts) {
            fatal_error_i("ilat: invalid i = %u", i);
        }
    }
//vsm_fmt    sprintf(inv_out,"grid pt %d,lon=%g,lat=%g,val=%lg",i,
    if (WxNum > 0) 
        sprintf(inv_out,"grid pt %d,lon=%lf,lat=%lf,val=\"%s\"",i,lon[i-1],lat[i-1],WxLabel(data[i-1]));
    else 
        sprintf(inv_out,"grid pt %d,lon=%lf,lat=%lf,val=%lg",i,lon[i-1],lat[i-1],data[i-1]);
    return 0;
}

/*
 * HEADER:100:lon:inv:2:value at grid point nearest lon=X lat=Y (WxText enabled)
 */

/**
 * Prints  the value of the grid point closest to the specified longitude latitude. The latitude-longitude 
 * of the grid point are also printed. If you use the verbose mode, the grid coordinates (i,j) and the 
 * number of the element are also printed. The -lon option can be repeated to save processing time. 
 * 
 * Note, by default the grid is converted to a WE:SN order which puts (1,1) in the South-West corner. 
 * Note, multiple -ijlat can be on the command line. 
 * 
 * ## Usage
 * -lon LONGITUDE LATITUDE
 * 
 * LONGITUDE = 0 .. 360
 * LATITUDE = -90 .. 90
 * 
 * ## Old vs New
 * The original code for the -lon option used the internal geolocation package. This package could 
 * compute the lat/lon of the grid points but nothing else. To find the grid point closest to a 
 * specified lat/lon, the distance to every grid point had to be calculated. Later, a short cut was 
 * added for lat-lon grids. Finally the geolocation packages gctpc and Proj4 have inverse functions 
 * which allow you to find the closest grid point to specified point for the the supported grids. 
 * For unsupported grids like the Gaussian grid and staggered grids, the original brute force code 
 * is used. You can turn off the new code by the -gctpc 0 option. The old code did not know about 
 * grid domains and would find the closest point even if the point were outside of the grid domain. 
 * The gctpc-based closest will return a lat=lon=999 to signal an intial point outside of the domain.
 * 
 * ## Want Speed?
 * You want extract the values for a 1000 different points. So you call wgrib2 1000 times and complain 
 * that wgrib2 is slow. Well decoding a jpeg2000 compressed file 1000 times does take time. It's better 
 * to add a 1000 -lon options to the command line and only decode the file once. 
 * 
 * The number of -lon options on a command line is limited by a compile-time option. Try running wgrib2 
 * -config and look for the line "maximum number of arguments on command line:". The current value is 
 * 5000 which allows you 5000 words on the command line. Each -lon option takes 3 words, so that gives 
 * you about 1600 -lon options you can run on one line. Of course, limitations such as the maximum line 
 * length or maximum number of continuations may stop you first. 
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_lon(ARG2) {

    struct local_struct {
        double plat, plon;
        int iptr, last_GDS_change_no;
    };
    struct local_struct *save;

    if (mode == -1) {
        WxText = decode = latlon = 1;
        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_lon","");
        save->plon = atof(arg1);
        if (save->plon < 0.0) save->plon += 360.0;
        save->plat = atof(arg2);
        save->iptr = -1;
        save->last_GDS_change_no = 0;
    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;
    save = (struct local_struct *) *local;

//    if (new_GDS) {
    if (save->last_GDS_change_no != GDS_change_no) {
        save->last_GDS_change_no = GDS_change_no;
//        if (output_order != wesn) {
//            fatal_error("lon only works in we:sn order","");
//        }
        if (data == NULL) fatal_error("gridded data not decoded","");
        if (lat == NULL || lon == NULL) 
            fatal_error("lat-lon information not available","");
        closest_init(sec);
        save->iptr = closest(sec, save->plat, save->plon);
    }
//vsm_frm    sprintf(inv_out,"lon=%g,lat=%g,val=%lg",lon[save->iptr],lat[save->iptr],data[save->iptr]);
    if (save->iptr < 0)  {
        sprintf(inv_out,"lon=%lg,lat=%lg,val=%lg", UNDEFINED_ANGLE, UNDEFINED_ANGLE, UNDEFINED);
        fprintf(stderr,"-lon: grid outside of domain of data\n");
        return 0;
    }

    if (mode == 0) {
        if (WxNum > 0) sprintf(inv_out,"lon=%lf,lat=%lf,val=\"%s\"",lon[save->iptr],lat[save->iptr],
                                WxLabel(data[save->iptr]));
        else sprintf(inv_out,"lon=%lf,lat=%lf,val=%lg",lon[save->iptr],lat[save->iptr],data[save->iptr]);
    }
    else {
        sprintf(inv_out,"lon=%lf,lat=%lf,i=%d,", lon[save->iptr],lat[save->iptr],(save->iptr)+1);
        inv_out += strlen(inv_out);
        if (nx > 0 && ny > 0) {
            sprintf(inv_out,"ix=%d,iy=%d,", (save->iptr)%nx+1, (save->iptr)/nx+1);
            inv_out += strlen(inv_out);
        }
        if (WxNum > 0) sprintf(inv_out,"val=\"%s\"", WxLabel(data[save->iptr]));
        else sprintf(inv_out,"val=%lg", data[save->iptr]);
    }
    return 0;
}


/**
 * Get latitude and longitude information from the grid section.
 * 
 * @param sec Pointer to the GRIB2 section array.
 * @param lon Pointer to store the allocated longitude array.
 * @param lat Pointer to store the allocated latitude array.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int get_latlon(unsigned char **sec, double **lon, double **lat) {

    int grid_template, err;

    if (*lat != NULL) {
        free(*lat);
        free(*lon);
        *lat = *lon = NULL;
    }

    grid_template = code_table_3_1(sec);
    if (grid_template == 0) {
        err = regular2ll(sec, lat, lon);
    }
    else if (grid_template == 1) {		// rotated lat-lon 
        err = rot_regular2ll(sec, lat, lon);
    }
    else if (grid_template == 10) {
        err = mercator2ll(sec, lat, lon);
    }
    else if (grid_template == 20) {
        err = polar2ll(sec, lat, lon);
    }
    else if (grid_template == 30) {
        err = lambert2ll(sec, lat, lon);
    }
    else if (grid_template == 40) {
        err = gauss2ll(sec, lat, lon);
    }
    else if (grid_template == 90) {
        err = space_view2ll(sec, lat, lon);
    }
    else if (grid_template == 130) {
        err = irr_grid2ll(sec, lat, lon);
    }
    else {
        err= 1;
    }
    return err; 
}

