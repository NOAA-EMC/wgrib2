/******************************************************************************************
 Copyright (C) 2005-2006  Karl Pfeiffer
 This file is part of wgrib2 and is distributed under terms of the GNU General Public License 
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA  02110-1301  USA
  mods made Wesley Ebisuzaki
  4/2006 Bug found by Naoya Suda (lambert2ll) Thanks
  10/2009 Bug found by Jerry Stueve (mercator) Thanks
*/

/*
 *  kdp 2005-08-22
 *
 *  Routines supporting the -geo option
 *
 * 1/2007 some cleanup M. Schwarb
 * 1/2008 lat and lon changed from float to double, polar can use LatD != 60 SV
 * 2/2008 vsm added LatD for lambert conformal
 * 2/2008 lambertll uses earth_radius as specified in code table 3.2
 * 2/2008 vsm fix lambert conformal
 * 2/2009 wne fix mercator
 * 6/2010 wne sec3_grid creates new sec3 and lat-lon values,
 *	changed (projection)2ll so that it reads gdt, not external nx, ny, npnts
 * need to check if all scan modes are handled
 * 10/2010 rotated lat-lon (experimental)
 * 1/2012 regional Gaussian grid
 * 12/2013 added staggering to regular2ll .. adds to rotated lat-lon grids
 * 02/2014 added staggering to lambert2ll 
 * 04/2014 add the new args to stagger()
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* M_PI, M_PI_2, M_PI_4, and M_SQRT2 are not ANSI C but are commonly defined */
/* values from GNU C library version of math.h copyright Free Software Foundation, Inc. */

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

extern double *lat, *lon;
extern int  scan, nx, ny;
extern int *variable_dim;
extern enum output_order_type output_order;

// static double toradians(double x) { return x * (M_PI/180.0); }
static double todegrees(double x) { return x * (180.0/M_PI); }

int regular2ll(unsigned char **sec, double **lat, double **lon) {
 
    int basic_ang, sub_ang;
    double units, dlat, dlon, lat1, lat2, lon1, lon2;
    double e, w, n, s, dx, dy;
 
    unsigned int i, j;
    double *llat, *llon;
    unsigned char *gds;
    unsigned int nnx, nny;
    int nres, nscan;
    unsigned int nnpnts;

    get_nxny_(sec, &nnx, &nny, &nnpnts, &nres, &nscan);
    gds = sec[3];

    if (nny < 1) {
        fprintf(stderr,"Sorry code does not handle variable ny yet\n");
        return 0;
    }

    if ((*lat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("regular2ll memory allocation failed","");
    }
    if ((*lon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("regular2ll memory allocation failed","");
    }

    /* now figure out the grid coordinates mucho silly grib specification */

    basic_ang = GDS_LatLon_basic_ang(gds);
    sub_ang = GDS_LatLon_sub_ang(gds);
    if (basic_ang != 0) {
        units = (double) basic_ang / (double) sub_ang;
    }
    else {
        units = 0.000001;
    }

    dlat = GDS_LatLon_dlat(gds) * units;
    dlon = GDS_LatLon_dlon(gds) * units;
    lat1 = GDS_LatLon_lat1(gds) * units;
    lat2 = GDS_LatLon_lat2(gds) * units;
    lon1 = GDS_LatLon_lon1(gds) * units;
    lon2 = GDS_LatLon_lon2(gds) * units;

    if (lon1 < 0.0 || lon2 < 0.0) fatal_error("BAD grid definition lon < zero","");
    if (lon1 > 360.0 || lon2 > 360.0) fatal_error("BAD grid definition lon >= 360","");
    if (lat1 < -90.0 || lat2 < -90.0 || lat1 > 90.0 || lat2 > 90.0) fatal_error("BAD grid definition lat","");

    /* find S latitude and dy */
    if (GDS_Scan_y(nscan)) {
        s = lat1;
        n = lat2;
    }
    else {
        s = lat2;
        n = lat1;
    }
    if (s > n) fatal_error("lat-lon grid: lat1 and lat2 inconsistent with scan order, bad grid definition","");

    if (nny > 1) {
        dy = (n - s) / (nny - 1);
        if (nres & 16) { /* lat increment is valid */
            if (fabs(dy - dlat) > 0.001) fatal_error("lat-lon grid: dlat is inconsistent, bad grid definition","");
        }
    }
    else { 
        dy = 0.0;
    }
// fprintf(stderr,">>> geo:  dy %lf dlat %lf nres %d has dy %d has dx %d\n", dy, dlat, nres, nres & 16, nres & 32);

    /* find W latitude and dx */

    if ( GDS_Scan_row_rev(nscan) && (nny % 2 == 0) && ((nres & 32) == 0) ) {
         fatal_error("grib grid definition ambiguity","");
    }

    if (GDS_Scan_x(nscan)) {
        w = lon1;
        e = lon2;
        if (GDS_Scan_row_rev(nscan) && ((nres & 32) == 0)) {
            e = lon1 + (nnx-1) * dlon;
        }
    } else {
        w = lon2;
        e = lon1;
        if (GDS_Scan_row_rev(nscan) && ((nres & 32) == 0)) {
            w = lon1 - (nnx-1) * dlon;
        }
    }

    if (e <= w) e += 360.0;
    if (e-w > 360.0) e -= 360.0;
    if (w < 0) {
        w += 360.0;
        e += 360.0;
    }

    /* lat-lon should be in a WE:SN order */

    if (nnx > 0 && nny > 0) {			/* non-thinned, potentially staggered grid */
	/* put x[] and y[] values in lon[] and lat[] */
        llat = *lat;
        llon = *lon;
	if (stagger(sec, nnpnts,llon,llat)) fatal_error("geo: stagger problem","");

        if (nnx > 1) {
	    dx = (e-w) / (nnx - 1);
	    dx = fabs(dx);
            if (nres & 32) { /* lon increment is valid */
                if (fabs(dx - fabs(dlon)) > 0.001) fatal_error("lat-lon grid: dlon is inconsistent, bad grid definition","");
	    }
        }
        else {
	    dx = 0.0;
	}
	dy = fabs(dy);

#pragma omp parallel for private(j)
	for (j = 0; j < nnpnts; j++) {
            llon[j] = lon1 + llon[j]*dx;
	    llon[j] = llon[j] >= 360.0 ? llon[j] - 360.0 : llon[j];
	    llon[j] = llon[j] < 0.0 ? llon[j] + 360.0 : llon[j];
	    llat[j] = lat1 + llat[j]*dy;
	}
	return 0;
    }

    /* must be thinned grid */

    llat = *lat;
        /* quasi-regular grid */
        for (j = 0; j < nny; j++) {
            for (i = 0; i < variable_dim[j];  i++) {
                *llat++ = s + j*dy;
            }
        }

    llon = *lon;
        /* quasi-regular grid */
        for (j = 0; j < nny; j++) {
            dx = (e-w) / (variable_dim[j]-1);
            for (i = 0; i < variable_dim[j]; i++) {
                *llon++ = w + i*dx >= 360.0 ? w + i*dx - 360.0: w + i*dx;
            }
        }
    return 0;
} /* end regular2ll() */ 

/* adapted from grib2ctl.pl */

int rot_regular2ll(unsigned char **sec, double **lat, double **lon) {

    unsigned char *gds;
    double units, *tlon, *tlat;
    double sp_lat, sp_lon, angle_rot;
    double sin_a, cos_a;
    int basic_ang, sub_ang;
    unsigned int i, npnts;
    double a, b, r, pr, gr, pm, gm, glat, glon;

    /* get the lat-lon coordinates in rotated frame of referencee */
    i = regular2ll(sec, lat, lon);
    if (i != 0) return i;

    gds = sec[3];
    npnts = GB2_Sec3_npts(sec);

    basic_ang = GDS_LatLon_basic_ang(gds);
    sub_ang = GDS_LatLon_sub_ang(gds);
    if (basic_ang != 0) {
	units = (double) basic_ang / (double) sub_ang;
    }
    else {
	units = 0.000001;
    }

    sp_lat = GDS_RotLatLon_sp_lat(gds) * units;
    sp_lon = GDS_RotLatLon_sp_lon(gds) * units;
    angle_rot = GDS_RotLatLon_rotation(gds) * units;

    a = (M_PI/180.0) * (90.0+sp_lat);
    b = (M_PI/180.0) * sp_lon;
    r = (M_PI/180.0) * angle_rot;

    sin_a = sin(a);
    cos_a = cos(a);

    tlat = *lat;
    tlon = *lon;

/* old code
    for (i = 0; i < npnts; i++) {
	pr = (M_PI/180.0) * *tlat;
	gr = -(M_PI/180.0) * *tlon;
        pm = asin(cos(pr)*cos(gr));
        gm = atan2(cos(pr)*sin(gr),-sin(pr));
        glat = (180.0/M_PI)*(asin(sin_a*sin(pm)-cos_a*cos(pm)*cos(gm-r)));
        glon = -(180.0/M_PI)*(-b+atan2(cos(pm)*sin(gm-r),sin_a*cos(pm)*cos(gm-r)+cos_a*sin(pm)) );
	*tlat++ = glat;
	*tlon++ = glon;
    }
*/

#pragma omp parallel for private(i,pr,gr,pm,gm,glat,glon)
    for (i = 0; i < npnts; i++) {
	pr = (M_PI/180.0) * tlat[i];
	gr = -(M_PI/180.0) * tlon[i];
        pm = asin(cos(pr)*cos(gr));
        gm = atan2(cos(pr)*sin(gr),-sin(pr));
        glat = (180.0/M_PI)*(asin(sin_a*sin(pm)-cos_a*cos(pm)*cos(gm-r)));
        glon = -(180.0/M_PI)*(-b+atan2(cos(pm)*sin(gm-r),sin_a*cos(pm)*cos(gm-r)+cos_a*sin(pm)) );
	tlat[i] = glat;
	tlon[i] = glon;
   }

   return 0;
}

/* adapted from iplib */

int polar2ll(unsigned char **sec, double **llat, double **llon) {
    
    double *lat, *lon;
    unsigned char *gds;

    double dx, dy, orient, de, de2, dr, tmp, xp, yp, h, lat1, lon1, dr2;
    double di, dj, LatD;
    unsigned int ix, iy;
    unsigned int nnx, nny;
    int nres, nscan;
    unsigned int nnpnts;
    size_t nx;    

    get_nxny_(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

    gds = sec[3];

    if (nnx < 1 || nny < 1) {
        fprintf(stderr,"Sorry code does not handle variable nx/ny yet\n");
        return 0;
    }
    nx = nnx;		/* size_t, multiplications will not overflow */

    if ((*llat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("polar2ll memory allocation failed","");
    }
    if ((*llon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("polar2ll memory allocation failed","");
    }
    lat = *llat;
    lon = *llon;


    /* based on iplib */

    lat1 = GDS_Polar_lat1(gds) * (M_PI/180);
    lon1 = GDS_Polar_lon1(gds);
    orient = GDS_Polar_lov(gds);
    LatD = GDS_Polar_lad(gds);

    lon1 *= (M_PI/180.0);
    orient *= (M_PI/180.0);

    dy  = GDS_Polar_dy(gds);
    dx  = GDS_Polar_dx(gds);

    h = 1.0;
    if (GDS_Polar_sps(gds)) {
	h = -1.0;
	/* added 12/19/2008 WNE sps checkout */
	orient -= M_PI;
    }

// removed 12/11    if (! (GDS_Scan_x(nscan))) dx = -dx;
// removed 12/11    if (! (GDS_Scan_y(nscan))) dy = -dy;

    /* 60 probably becomes something else in grib2 */
    /* vsm: from comment to grib2 polar template:
    "Grid length is in units of 10-3 m at the latitude specified by LaD"
     do use GDS_Polar_lad(gds) instead of 60?
     Do use fabs for southern hemisphere?
    */ 

    de = (1.0 + sin(fabs(LatD)*(M_PI/180.0))) * radius_earth(sec);
    dr = de * cos(lat1) / (1 + h*sin(lat1));

    xp=-h*sin(lon1-orient)*dr/dx;
    yp= cos(lon1-orient)*dr/dy;

// added 12/11
    if (! (GDS_Scan_y(nscan))) {
	yp = yp - nny + 1;
    }
    if (! (GDS_Scan_x(nscan))) {
	xp = xp - nnx + 1;
    }

    de2 = de*de;
#pragma omp parallel for private(iy,ix,di,dj,dr2,tmp)
    for (iy = 0; iy < nny; iy++) {
        for (ix = 0; ix < nnx; ix++) {
            di = (ix - xp) * dx;
            dj = (iy - yp) * dy;
            dr2 = di*di + dj*dj;
            if (dr2 < de2*1e-6) {
                lon[ix+iy*nx] = 0.0;
                lat[ix+iy*nx] = h*90.0;
            } else {
                tmp = (orient+h*atan2(di,-dj))*(180.0/M_PI);
                if (tmp < 0.0) tmp += 360.0;
                if (tmp > 360.0) tmp -= 360.0;
                lon[ix+iy*nx] = tmp;
                lat[ix+iy*nx] = h*asin((de2-dr2)/(de2+dr2))*(180.0/M_PI);
            }
        }
    }
    return 0;
}


int lambert2ll(unsigned char **sec, double **llat, double **llon) {

    double n;
    double *lat, *lon;

    double dx, dy, lat1r, lon1r, lon2d, lon2r, latin1r, latin2r;
    double lond, latd, d_lon;
    double f, rho, rhoref, theta, startx, starty;
    unsigned int nnx, nny;
    int nres, nscan;
    double x, y, tmp;
    unsigned char *gds;
    double latDr;
    double earth_radius;
    unsigned int j, nnpnts;

    get_nxny_(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

    if (nnx < 1 || nny < 1) {
        fprintf(stderr,"Sorry code does not handle variable nx/ny yet\n");
        return 0;
    }

    earth_radius = radius_earth(sec);
    gds = sec[3];
    dy      = GDS_Lambert_dy(gds);
    dx      = GDS_Lambert_dx(gds);
    lat1r   = GDS_Lambert_La1(gds) * (M_PI / 180.0);
    lon1r   = GDS_Lambert_Lo1(gds) * (M_PI / 180.0);
    lon2d   = GDS_Lambert_Lov(gds);
    lon2r   = lon2d * (M_PI / 180.0);
    latin1r = GDS_Lambert_Latin1(gds) * (M_PI/180.0);
    latin2r = GDS_Lambert_Latin2(gds) * (M_PI/180.0);

//  fix for theta start value crossing 0 longitude
//    if ((lon1r - lon2r) > 0) lon2r = lon2r + 2*M_PI;


//
// Latitude of "false origin" where scales are defined.
// It is used to estimate "reference_R", rhoref.
// Often latDr == latin1r == latin2r and non-modified code is true and works fine.
// But could be different if intersection latitudes latin1r and latin2r are different.
// Usually latDr must be latin1r <=  latDr <= latin2r, other could be strange.
//
    latDr = GDS_Lambert_LatD(gds) * (M_PI/180.0);


    if (lon1r < 0) fatal_error("bad grid definition, lon1r < 0.0","");

    if ( fabs(latin1r - latin2r) < 1E-09 ) {
        n = sin(latin1r);
    }
    else {
        n = log(cos(latin1r)/cos(latin2r)) / 
        log(tan(M_PI_4 + latin2r/2.0) / tan(M_PI_4 + latin1r/2.0));
    }
  
    f = (cos(latin1r) * pow(tan(M_PI_4 + latin1r/2.0), n)) / n;
  
    rho = earth_radius * f * pow(tan(M_PI_4 + lat1r/2.0),-n);
    // old rhoref = earth_radius * f * pow(tan(M_PI_4 + latin1r/2.0),-n);
    rhoref = earth_radius * f * pow(tan(M_PI_4 + latDr/2.0),-n);

    // 2/2009 .. new code
    d_lon = lon1r - lon2r;
    if (d_lon > M_PI) d_lon -= 2*M_PI;
    if (d_lon < -M_PI) d_lon += 2*M_PI;
    theta = n * d_lon; 
    // 2/2009 theta = n * (lon1r - lon2r); 

    startx = rho * sin(theta);
    starty = rhoref - rho * cos(theta);

    if ((*llat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("lambert2ll memory allocation failed","");
    }
    if ((*llon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("lambert2ll memory allocation failed","");
    }
    lat = *llat;
    lon = *llon;

    /* put x[] and y[] values in lon[] and lat[] */
    if (stagger(sec, nnpnts, lon, lat)) fatal_error("geo: stagger problem","");

    dx = fabs(dx);
    dy = fabs(dy);

#pragma omp parallel for private(j,x,y,tmp,theta,rho,lond,latd)
    for (j = 0; j < nnpnts; j++) {
	y = starty + lat[j]*dy;
        x = startx + lon[j]*dx;
	tmp = rhoref - y;
	theta = atan(x / tmp);
        rho = sqrt(x * x + tmp*tmp);
        rho = n > 0 ? rho : -rho;
        lond = lon2d + todegrees(theta/n);
        latd = todegrees(2.0 * atan(pow(earth_radius * f/rho,1.0/n)) - M_PI_2);
	lond = lond >= 360.0 ? lond - 360.0 : lond;
	lond = lond < 0.0 ? lond + 360.0 : lond;
        lon[j] = lond;
        lat[j] = latd;
    }
    return 0;
} /* end lambert2ll() */

int mercator2ll(unsigned char **sec, double **lat, double **lon) {

    double dx, dy, lat1, lat2, lon1, lon2;
    double *llat, *llon;
    unsigned int i, j;
    double dlon, circum;

    double n,s,e,w,tmp,error;
    unsigned char *gds;

    unsigned int nnx, nny;
    int nres, nscan;
    unsigned int nnpnts;

    get_nxny_(sec, &nnx, &nny, &nnpnts, &nres, &nscan);
    gds = sec[3];

    dy     = GDS_Mercator_dy(gds);
    dx     = GDS_Mercator_dx(gds);
    lat1 = GDS_Mercator_lat1(gds);
    lat2 = GDS_Mercator_lat2(gds);
    lon1 = GDS_Mercator_lon1(gds);
    lon2 = GDS_Mercator_lon2(gds);

    if (lon1 < 0.0 || lon2 < 0.0 || lon1 > 360.0 || lon2 > 360.0) fatal_error("BAD grid definition lon","");
    if (lat1 < -90.0 || lat2 < -90.0 || lat1 > 90.0 || lat2 > 90.0) fatal_error("BAD grid definition lat","");

    if (GDS_Mercator_ori_angle(gds) != 0.0) {
        fprintf(stderr,"cannot handle non-zero mercator orientation angle %f\n",
                GDS_Mercator_ori_angle(gds));
        return 0;
    }

    if (nnx < 1 || nny < 1) {
        fprintf(stderr,"Sorry geo/mercator code does not handle variable nx/ny yet\n");
        return 0;
    }

    if ((*lat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("mercator2ll memory allocation failed","");
    }
    if ((*lon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("mercator2ll memory allocation failed","");
    }

    /* now figure out the grid coordinates mucho silly grib specification */

    /* find S and N latitude */
    if (GDS_Scan_y(nscan)) {
        s = lat1;
        n = lat2;
    }
    else {
        s = lat2;
        n = lat1;
    }
    if (s > n) fatal_error("Mercator grid: lat1 and lat2, bad grid definition","");

    /* find W and E longitude */

    if ( ((nscan & 16) == 16) && (nny % 2 == 0) && ((nres & 32) == 0) ) {
         fatal_error("Mercator grid: grid definition ambiguity, bad grid definition","");
    }

    if ( ((nscan & 16) == 16) && (nny % 2 == 0) ) {
         fatal_error("Mercator: more code needed to decode grid definition","");
    }

    if (GDS_Scan_x(nscan)) {
        w = lon1;
        e = lon2;
    } else {
        w = lon2;
        e = lon1;
    }
    if (e <= w) e += 360.0;


    llat = *lat;
    llon = *lon;

    dlon = (e-w) / (nnx-1);
    circum = 2.0 * M_PI * radius_earth(sec) * cos(GDS_Mercator_latD(gds) * (M_PI/180.0));
    dx = dx * 360.0 / circum;

    // dlon should be almost == to dx
    // replace dx by dlon to get end points to match

    if (dx != 0.0) {
	error = fabs(dx-dlon) / fabs(dx);
	if (error >= 0.001) { fprintf(stderr,
           "\n*** Mercator grid error: inconsistent d-longitude, radius and grid domain\n"
		"*** d-longitude from grid domain %lf (used), d-longitude from dx %lf (not used)\n",
		dlon, dx);
	}
        dx = dlon;
    }

    s = log(tan((45+s/2)*M_PI/180));
    n = log(tan((45+n/2)*M_PI/180));
    dy = (n - s) / (nny - 1);

#pragma omp parallel for private(j,tmp,i)
    for (j = 0; j < nny; j++) {
        tmp = (atan(exp(s+j*dy))*180/M_PI-45)*2;
        for (i = 0; i < nnx; i++) {
            llat[i+j*nnx] = tmp;
        }
    }

    for (j = 0; j < nnx; j++) {
        llon[j] = w + j*dx >= 360.0 ?  w + j*dx - 360.0 : w + j*dx;
    }
    for (j = nnx; j < nnpnts; j++) {
        llon[j] = llon[j-nnx];
    }
    return 0;
} /* end mercator2ll() */




/*  kdp 2005-08-22
 *  
 *  Code for computing Gaussian latitudes was adapted from
 *  the wonderful gauss2lats.m Matlab program from Tom Holt.
 *  The code gauss2lats.m also works quite well with Octave.
 *
 *  Note that the algorithms used here require a 1-based
 *  array vice the typical 0-based array.  The points are
 *  mapped correctly to the (lat,lon) arrays as zero-based.
 *
 * Note: adapted from an NCAR fortran program by Tom Holt
 */
double gord(int n, double x) {
  
  double colat = acos(x);
  double c1 = M_SQRT2;
  int i;
  
  double fn = (double) n;
  double ang = fn * colat;
  double s1 =  0.0;
  double c4 =  1.0;
  double a  = -1.0;
  double b  =  0.0;
  double fi;

  for (i=1; i <= n; i++) {
    c1 = c1 * sqrt(1.0 - 1.0/(4.0*i*i));
  } 
  
  for (i = 0; i <= n; i = i + 2) {
    if ( i == n ) { c4 = 0.5 * c4; }
    s1  = s1 + c4*cos(ang);
    a   = a + 2.0;
    b   = b + 1.0;
    fi = (double) i;
    ang = colat*(fn - fi - 2.0); 
    c4 = (a*(fn-b+1.0)/(b*(fn+fn-a)))*c4;
  }
  
  return ( s1 * c1 );
  
} /* end gord() */


double *gauss2lats(int nlat, double *ylat) {
  
  const double xlim = 1.0E-7;
  
  double *cosc  = (double *) malloc(sizeof(double) * (nlat + 1));
  double *sinc  = (double *) malloc(sizeof(double) * (nlat + 1));
  double *colat = (double *) malloc(sizeof(double) * (nlat + 1));
  
  int nzero = (nlat / 2);
  
  int i;
  double fi = nlat;
  double fi1 = fi + 1.0;
  double a = fi * fi1/sqrt(4.0*fi1*fi1 - 1.0);
  double b = fi1 * fi/sqrt(4.0*fi*fi - 1.0);

    double g, gm, gp, gt, delta, d;

  for (i = 1; i <= nzero; i++) {
    cosc[i] = sin((i - 0.5)*M_PI/nlat + M_PI*0.5);
  }
  
  for (i = 1; i <= nzero; i++) {
    g = gord(nlat, cosc[i]);
    gm = gord(nlat - 1, cosc[i]);
    gp = gord(nlat + 1, cosc[i]);
    gt = (cosc[i]*cosc[i] - 1.0)/(a * gp - b * gm);
    delta = g*gt;
    cosc[i] = cosc[i] - delta;
    
    while ( fabs(delta) > xlim ) {
      g = gord(nlat,cosc[i]);
      gm = gord(nlat - 1, cosc[i]);
      gp = gord(nlat + 1, cosc[i]);
      gt = (cosc[i]*cosc[i] - 1.0)/(a * gp - b * gm);
      delta = g*gt;
      cosc[i] = cosc[i] - delta;
      
    } /* end while */
    
  } /* end for */
  
  for (i = 1; i <= nzero; i++) {
    colat[i] = acos(cosc[i]);
    sinc[i] = sin(colat[i]);
  }
  
  /*
   * ... deal with equator if odd number of points
   */
  if ( ( nlat % 2) != 0 ) {
    i = nzero + 1;
    cosc[i] = 0.0;
    d = gord(nlat - 1, cosc[i]);
    d = d*d*fi*fi;
    colat[i] = M_PI * 0.5;
    sinc[i] = 1.0;
  } /* end if() */
  
  /*
   *  ... deal with southern hemisphere by symmetry
   */
  for (i = nlat - nzero + 1; i <= nlat; i++) {
    cosc[i]  = -cosc[nlat + 1 - i];
    colat[i] = M_PI - colat[nlat + 1 - i];
    sinc[i]  = sinc[nlat + 1 - i];
  } /* end for(i) */
  
  for (i = 1; i <= nlat; i++) {
    ylat[i-1] = todegrees(acos(sinc[i]));
    if ( i > (nlat / 2) ) ylat[i-1] = -ylat[i-1];
    /* change from N-S to S-N */
    ylat[i-1] = -ylat[i-1];
  }

  free(cosc);
  free(sinc);
  free(colat);
  
  return ylat;
  
} /* end gauss2lats() */

#define LATERR		(0.01 * 180.0 / (double) nlat)


int gauss2ll(unsigned char **sec, double **llat, double **llon) {
 
 
    int nlat; /* in grib, number of latitudes must be even! */
  
    double dx, e, w, south, north, lat1, lon1, lat2, lon2, *ylat;
    int isouth, inorth;
    double units;
    double *lat, *lon;
    int basic_ang, sub_ang;
    int i,j, n;
    unsigned int ii, jj;
    unsigned char *gds;

    int nnx, nny, nres, nscan;
    unsigned int nnx_, nny_;
    unsigned int nnpnts;

    get_nxny_(sec, &nnx_, &nny_, &nnpnts, &nres, &nscan);
    get_nxny(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

    gds = sec[3];
    nlat = 2 * GDS_Gaussian_nlat(gds);

    if (nny_ == 0) fatal_error("gauss2ll: code does not handle variable ny","");

    /* figure out angle units */

    basic_ang = GDS_Gaussian_basic_ang(gds);
    sub_ang = GDS_Gaussian_sub_ang(gds);
    units = basic_ang == 0 ? 0.000001 : (double) basic_ang / (double) sub_ang;

    lat1 = GDS_Gaussian_lat1(gds) * units;
    lat2 = GDS_Gaussian_lat2(gds) * units;
    lon1 = GDS_Gaussian_lon1(gds) * units;
    lon2 = GDS_Gaussian_lon2(gds) * units;

    if (lon1 < 0.0 || lon2 < 0.0 || lon1 > 360.0 || lon2 > 360.0) fatal_error("gausian2ll: bad grid defintion lon","");
    if (lat1 < -90.0 || lat2 < -90.0 || lat1 > 90.0 || lat2 > 90.0) fatal_error("gaussian2ll: bad grid definition lat","");

    /* find S latitude and dy */
    if (GDS_Scan_y(nscan)) {
        south = lat1;
        north = lat2;
    }
    else {
        south = lat2;
        north = lat1;
    }
    if (south > north) fatal_error("gaussian grid: lat1 and lat2 inconsistent with scan order, bad grid definition","");

    if (nny > nlat || nny < 0) fatal_error_i("gauss2ll: bad grid definition, ny %d",nny);

    if ((*llat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("gauss2ll memory allocation failed","");
    }
    if ((*llon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("gauss2ll memory allocation failed","");
    }
    lat = *llat;
    lon = *llon;

    /* do latitudes first */
 
    ylat = (double *) malloc(sizeof(double) * nlat);

    /* calculate Gaussian latitudes */
    gauss2lats(nlat, ylat);

    /* find index of south and north */
    isouth = inorth = -1;
    for (i = 0; i < nlat; i++) {
	if (fabs(south - ylat[i]) < LATERR) {
	    isouth = i;
	    break;
	}
    }
    for (i = 0; i < nlat; i++) {
	if (fabs(north - ylat[i]) < LATERR) {
	    inorth = i;
	    break;
	}
    }

    if (isouth < 0 || inorth < 0) fatal_error("gauss2ll: bad grid definition, lat1/lat2 not a Gaussian latitude","");
    if (inorth - isouth + 1 != nny) fatal_error("gauss2ll: bad grid definition, lat1/lat2 not consistent with ny","");

    n = 0;
    if (nnx_ > 0) {        /* regular grid */

#pragma omp parallel for private(ii,jj)
	for (jj = 0; jj < nny_; jj++) {
            for (ii = 0; ii < nnx_; ii++) {
                lat[ii+jj*nnx_] = ylat[jj+isouth];
            }
        }
    }
    else {                /* quasi regular grid */
        for (jj = 0; jj < nny_; jj++) {
            for (ii = 0; ii < variable_dim[jj]; ii++) {
                lat[n++] = ylat[jj+isouth];
            }
        }
    }

    free(ylat); 

    /* now for the longitudes */
  
    if (GDS_Scan_x(nscan)) {
        e = lon1;
        w = lon2;
    }
    else {
        e = lon2;
        w = lon1;
    }
    if (e > w) w += 360.0;
    if (e < 0.0) {
        e += 360.0;
        w += 360.0;
    }
    if (e >= 360.0) {
        e -= 360.0;
        w -= 360.0;
    }

    if (nnx >= 0) {
        dx = (w-e) / (nnx-1);

#pragma omp parallel
{
#pragma omp for private(i)
	for (i = 0; i < nnx; i++) {
            lon[i] = e + (dx * i) >= 360.0 ?  e + (dx * i) - 360.0 : e + (dx * i);  
	}
#pragma omp for private(i,j)
	for (j = 1; j < nny; j++) {
	    for (i = 0; i < nnx; i++) {
		lon[i+j*nnx] = lon[i];
	    }
	}
}
    }
    else {
        n = 0;
        for (j = 0; j < nny; j++) {
            dx = (w-e) / (variable_dim[j]-1);
            for (i = 0; i < variable_dim[j]; i++) {
                lon[n++] = e + (dx * i) >= 360.0 ?  e + (dx * i) - 360.0 : e + (dx * i);
            }
        }
    }
    return 0;
} /* end gauss2ll() */


/* find the closest grid point to (plat, plon) */

/* this code needs to be rewritten. too slow */



/* closest_init:  location of grid point in x-y-z space, assume r=1 */ 

static double *x = NULL, *y = NULL, *z = NULL;
extern int use_gctpc;

int closest_init(unsigned char **sec) {

    unsigned int i, nnpts;
    double s, c;
    int grid_type;


    if (use_gctpc && output_order == wesn && nx > 0 && ny > 0) {
       if (gctpc_ll2xy_init(sec, lon, lat) == 0) return 0;
    }

    grid_type = code_table_3_1(sec);

    if  (!GDS_Scan_staggered(scan) && nx > 0 && ny > 0) {
        /* if grids with (lat,lon) -> (i,j) insert code here */
        if (grid_type == 0 && output_order == wesn) return latlon_init(sec, nx, ny);
        if (grid_type == 90 && output_order == wesn) return space_view_init(sec);
    }

    nnpts = GB2_Sec3_npts(sec);
    if (x) {
        free(x);
        free(y);
        free(z);
        x = y = z = NULL;
    }
    if (lat && lon) {
        x = (double *) malloc(((size_t) nnpts) * sizeof(double));
        y = (double *) malloc(((size_t) nnpts) * sizeof(double));
        z = (double *) malloc(((size_t) nnpts) * sizeof(double));
        if (x == NULL || y == NULL || z == NULL) fatal_error("memory allocation closest_init","");

#pragma omp parallel for private(i,s,c)
        for (i = 0; i < nnpts; i++) {
	    if (lat[i] >= 999.0 || lon[i] >= 999.0) {
		/* x[i] = sin() .. cannot be bigger than 1 */
		x[i] = y[i] = z[i] = 999.0;
	    }
            else {
                s = sin(lat[i] * (M_PI / 180.0));
                c = sqrt(1.0 - s * s);
                z[i] = s;
                x[i] = c * cos(lon[i] * (M_PI / 180.0));
                y[i] = c * sin(lon[i] * (M_PI / 180.0));
	    }
        }

    }
    return 0;
}

/*
 * closest: 
 *    * 6/2018 2G+ rewrote OpenMP
 */
long int closest(unsigned char **sec, double plat, double plon) {

    unsigned int i, i0, nnpts;
    long int j;
    int grid_type;
    double t, xx, yy, zz, small;
    unsigned int k;

    if (use_gctpc && output_order == wesn && nx > 0 && ny > 0) {
	/* will fix it so that everything is 0 for out of bounds */
	if (gctpc_ll2i(1, &plon, &plat, &k) == 0) return ((long int) k) - 1;
    }

    grid_type = code_table_3_1(sec);

    // if grid with (lat,lon) -> (i,j) /l.. insert code here
    if (grid_type == 0 && nx > 0 && ny > 0 && output_order == wesn) return latlon_closest(sec, plat, plon);
    if (grid_type == 90 && nx > 0 && ny > 0 && output_order == wesn) return space_view_closest(sec, plat, plon);

    nnpts = GB2_Sec3_npts(sec);
    if (x == NULL) return -1l;

    zz = sin(plat * (M_PI / 180.0));
    t = sqrt(1.0 - zz*zz);
    xx = t * cos(plon * (M_PI / 180.0));
    yy = t * sin(plon * (M_PI / 180.0));

    small = 0.0;
    j = -1;

    for (i = 0; i < nnpts; i++) {
	if (x[i] >= 999.0) continue;
        small = (x[i]-xx)*(x[i]-xx)+(y[i]-yy)*(y[i]-yy)+(z[i]-zz)*(z[i]-zz);
        j = i;
	break;
    }
    if (j == -1) return j;
    i0 = j;


#pragma omp parallel private(i)
{
    double s, small_thread;
    long int j_thread;

    small_thread = small;
    j_thread = j;

#pragma omp for nowait
    for (i = i0; i < nnpts; i++) {
	if (x[i] >= 999.0) continue;
        s = (x[i]-xx)*(x[i]-xx)+(y[i]-yy)*(y[i]-yy)+(z[i]-zz)*(z[i]-zz);
        if (s < small_thread) {
	    small_thread = s;
            j_thread = i;
	}
    }
#pragma omp critical
    {
	if (small_thread < small) {
            small = small_thread;
            j = j_thread;
        }
    }
}
    return j;
}
