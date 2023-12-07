#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "config.h"

#ifdef WMO_VALIDATION

/*
 *2/2019: public domain W. Ebisuzaki
 * 
 *
 */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif
#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif

extern enum output_order_type output_order;

int cubed_sphere2ll(unsigned char **sec, double **llat, double **llon) {
    double *x, *y;
    int nres, nscan;
    unsigned int nnx_, nny_, nnpnts, npnts_tile;
    int gds_tile, tile, tile_start, tile_end;
    unsigned int ncell, i, i_offset, j_offset;
    double b, sb, asb;
    double stretch, sin_lat, oneps2, onems2;
    unsigned char *gds;
    double sp_lat, sp_lon, angle_rot, a, c, r, sin_a, cos_a;
    double xn, yn, zn, dist;
    double pr,gr,pm,gm;


    if (code_table_3_1(sec) != 60) fatal_error("cubed_sphere2ll: not cubed sphere","");
    if (output_order != wesn) fatal_error("cubed_sphere2ll grid must in we:sn order","");
    
    get_nxny_(sec, &nnx_, &nny_, &nnpnts, &nres, &nscan);

    if (nnx_ == 0 || nny_ == 0) {
        fprintf(stderr,"Sorry code does not handle variable nx/ny yet\n");
        return 0;
    }

    gds = sec[3];
    gds_tile = GDS_Gnom_tile(gds);
    if (gds_tile < 0 || gds_tile > 6) fatal_error_i("cubed_sphere2ll: invalid tile=%d", gds_tile);
    ncell = GDS_Gnom_face_size(gds);

    i_offset = GDS_Gnom_i_offset(gds);
    if (i_offset+nnx_ > ncell+1) 
	fatal_error_i("cubed_sphere2ll: bad value of i_offset %d", i_offset);

    j_offset = GDS_Gnom_j_offset(gds);
    if (j_offset+nny_ > ncell+1)
        fatal_error_i("cubed_sphere2ll: bad value of j_offset %d", j_offset);
     
    b = GDS_Gnom_B(gds);
    if (b <= -1.0) fatal_error("cubed_sphere2ll: bad value of b","");

    stretch = GDS_Gnom_Stretch(gds);
    if (stretch <= 0.0) fatal_error("cubed_sphere2ll: stretch <= 0","");
    oneps2 = 1 + stretch*stretch;
    onems2 = 1 - stretch*stretch;

    if ((*llat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("cubed_sphere2ll memory allocation failed","");
    }
    if ((*llon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("cubed_sphere2ll memory allocation failed","");
    }
    x = *llon;
    y = *llat; 

    if (b >= 0.0) {
	sb = sqrt(b);
	asb = atan(sb);
    }
    else {
        sb = sqrt(-b);
        asb = atanh(sb);
    }

    // parameters for rotation
    sp_lat = GDS_Gnom_SP_Lat(gds);
    sp_lon = GDS_Gnom_SP_Lon(gds);
    angle_rot = GDS_Gnom_SP_Rot(gds);
    a = (M_PI/180.0) * (90.0+sp_lat);
    c = (M_PI/180.0) * sp_lon;
    r = (M_PI/180.0) * angle_rot;
    sin_a = sin(a);
    cos_a = cos(a);
    if (gds_tile == 0) {
	tile_start = 1; tile_end=6;
        npnts_tile = nnpnts / 6;
    }
    else {
	tile_start = gds_tile; tile_end=gds_tile;
        npnts_tile = nnpnts;
        /* x[] ranges from 0..nnx_-1,    y[] ranges from 0..nny_-1 for non staggered */ 
    }
    if (stagger(sec, nnpnts, x, y)) fatal_error("cubed_sphere2ll: stagger problem","");

#pragma omp parallel for private(i)
    for (i = 0; i < npnts_tile; i++) {
        /* want x[] to range from -1..1  y[] to range from -1..1 */
	x[i] = (x[i] + i_offset) * 2.0 / ncell - 1.0;
	y[i] = (y[i] + j_offset) * 2.0 / ncell - 1.0;
	/* apply b factor */
	if (b > 0.0) {
	    x[i] = tan(asb*x[i]) / sb;
	    y[i] = tan(asb*y[i]) / sb;
	}
	else if (b < 0.0) {
	    x[i] = tanh(asb*x[i]) / sb;
	    y[i] = tanh(asb*y[i]) / sb;
	}
    }

    // if gds_tile == 0, global, copy x[], y[] to other tiles
    if (gds_tile == 0) {
        for (i = 0; i < npnts_tile; i++) {
	    x[i+npnts_tile] = x[i+2*npnts_tile] = x[i+3*npnts_tile] = 
		x[i+4*npnts_tile] = x[i+5*npnts_tile] = x[i];
	    y[i+npnts_tile] = y[i+2*npnts_tile] = y[i+3*npnts_tile] = 
		y[i+4*npnts_tile] = y[i+5*npnts_tile] = y[i];
	}
    } 


#pragma omp parallel for private(i, dist, xn, yn, zn, pr, gr, pm, gm, sin_lat, tile)
    for (i = 0; i < nnpnts; i++) {
	if (nnpnts == npnts_tile) {
	    tile = gds_tile;
	}
	else {
	    tile = i / npnts_tile + 1;
	}

	/* xn, yn, zn = 3d coordinate on 2x2x2 cube */
	if (tile == 1) {
		xn = y[i];
		yn = x[i];
		zn = -1.0;
	} else if (tile == 2) {
		xn = 1.0;
		yn = x[i];
		zn = y[i];
	} else if (tile == 3) {
		xn = -x[i];
		yn = 1.0;
		zn = y[i];
	} else if (tile == 4) {
		xn = -1.0;
		yn = -x[i];
		zn = y[i];
	} else if (tile == 5) {
		xn = x[i];
		yn = -1.0;
		zn = y[i];
	} else {	// tile 6
		xn = -y[i];
		yn = x[i];
		zn = 1.0;
	}

        // x, y, z ranges from -1 to 1
        dist = sqrt(x[i]*x[i] + y[i]*y[i] + 1.0);

        // Cartesian to spherical coordinates (radians)
        x[i] = atan2(yn, xn);
        if (fabs(x[i]) < 1e-10) x[i] = 0.0;
        y[i] = asin(zn/dist);

        /* x[] and y[] are lon/lat in radians */

	/* stretching */
	if (stretch != 1.0) {
	    sin_lat = sin(y[i]);
	    y[i] = asin( (onems2 + oneps2 * sin_lat)  / (oneps2 + onems2*sin_lat) );
	}

	/* do the rotation note: rotation b term -> c */
        pr = y[i];
        gr = -x[i];
        pm = asin(cos(pr)*cos(gr));
        gm = atan2(cos(pr)*sin(gr),-sin(pr));
        y[i] = (180.0/M_PI)*(asin(sin_a*sin(pm)-cos_a*cos(pm)*cos(gm-r)));
        x[i] = -(180.0/M_PI)*(-c+atan2(cos(pm)*sin(gm-r),sin_a*cos(pm)*cos(gm-r)+cos_a*sin(pm)) );
	if (x[i] < 0.0) x[i] += 360.0;
	if (x[i] > 360.0) x[i] -= 360.0;
    }
    return 0;
}

int cubed_spherell2xy(unsigned char **sec, int n, double *lon, double *lat, double *xx, double *yy, int *face) {
    unsigned int i, j;
    double sp_lat, sp_lon, angle_rot, a, c, r, sin_a, cos_a;
    unsigned char *gds;
    double x, y, z, xprime, yprime, zprime;
  
    gds = sec[3];
    // parameters for rotation
    sp_lat = GDS_Gnom_SP_Lat(gds);
    sp_lon = GDS_Gnom_SP_Lon(gds);
    angle_rot = GDS_Gnom_SP_Rot(gds);
    a = (M_PI/180.0) * (90.0+sp_lat);
    c = (M_PI/180.0) * sp_lon;
    r = (M_PI/180.0) * angle_rot;
    sin_a = sin(a);
    cos_a = cos(a);

#pragma omp parallel for private(i,x,y,z, xprime, yprime, zprime)
    for (i = 0; i < n; i++) {
	x = cos(lat[i]*M_PI/180.0) * cos(lon[i]*M_PI/180.0 - c);
	y = cos(lat[i]*M_PI/180.0) * sin(lon[i]*M_PI/180 - c);
	z = sin(lat[i]*M_PI/180.0);

	xprime = cos(-c) * x - sin(-c) * z;
	zprime = sin(-c) * x + cos(-c) * z;

	x = xprime;
	z = zprime;
// more stuff
    }
    return 0;
}
#endif
