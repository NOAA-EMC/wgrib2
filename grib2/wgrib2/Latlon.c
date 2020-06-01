#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 12/2006 Public Domain Wesley Ebisuzaki
 * 1/2007 Cleanup M. Schwarb
 * 1/2008 lat and lon changed from float to double 
 * 1/2011 replace new_GDS by GDS_change_no, WNE
 */
   
extern int decode;
extern int need_output_file;
extern enum output_order_type output_order;

extern double *lat, *lon;
extern int latlon;
extern int WxNum, WxText;

extern int scan, nx, ny, GDS_change_no;
extern unsigned int npnts;


/*
 * HEADER:100:ij:inv:2:value of field at grid(X,Y) X=1,..,nx Y=1,..,ny (WxText enabled)
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

int f_ilat(ARG1) {

    struct local_struct {
        int ix, last_GDS_change_no;
    };
    struct local_struct *save;
    int i;

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
            fatal_error_i("ilat: invalid i = %d", i);
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

