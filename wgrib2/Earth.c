/*
 * Public Domain: Wesley Ebisuzaki 2010, 2017
 *  6/2017: print out semi major/minor axes
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:radius:inv:0:radius of Earth
 */

int f_radius(ARG0) {
    int table_3_2;
    unsigned char *p;
    double radius, major, minor;
    if (mode >= 0) {
        if ((p = code_table_3_2_location(sec)) == NULL) {
	    sprintf(inv_out,"earth radius is not available");
	    return 0;
	}
        table_3_2 = (int) *p;
	sprintf(inv_out,"code3.2=%d ", table_3_2);
	inv_out += strlen(inv_out);
	axes_earth(sec, &major, &minor);
        radius = (major+minor)*0.5;

	switch(table_3_2) {
	    case 0: 
	    case 6: 
	    case 8: 
		sprintf(inv_out,"sphere predefined radius=%.1lf m", major); break;
	    case 1: sprintf(inv_out,"sphere user defined radius=%.1lf m",major); break;
	    case 2: sprintf(inv_out, "IAU 1965 ave radius=%.1lf m major axis=%.1lf m minor axis=%.1lf m",
			radius, major,minor); break;
	    case 4: sprintf(inv_out, "IAG-GRS80 ave radius=%.1lf m major axis=%.1lf m minor axis=%.1lf m",
			radius, major,minor); break;
	    case 5: sprintf(inv_out, "WGS84 ave radius=%.1lf m major axis=%.1lf m minor axis=%.1lf m",
			radius, major,minor); break;
	    case 9: sprintf(inv_out, "OSGB-1936 Airy 1830 ave radius=%.1lf m major axis=%.1lf m minor axis=%.1lf m",
			radius, major,minor); break;
	    case 3:
	    case 7:
	        sprintf(inv_out, "oblate spheroid user defined ave radius=%.1lf m major axis=%.1lf m minor axis=%.1lf m",
		    radius, major, minor);
		break;
	    default:
		sprintf(inv_out,"unknown size/shape of earth");
		break;
	}
    }
    return 0;
}

/*
 * HEADER:100:set_radius:misc:1:set radius of Earth  X= 0,2,4,5,6,8,9 (Code Table 3.2), 1:radius , 7:major:minor
 */
int f_set_radius(ARG1) {
    unsigned char *p;
    int i, v3_2, scale, val;
    double major, minor;
    if (mode >= 0) {
        p = code_table_3_2_location(sec);
	if (p == NULL) {
	    sprintf(inv_out,"earth radius can not be set");
	    return 0;
	}
	if (strcmp(arg1,"0") == 0) { *p  = 0; return 0; }
	if (strcmp(arg1,"2") == 0) { *p  = 2; return 0; }
	if (strcmp(arg1,"4") == 0) { *p  = 4; return 0; }
	if (strcmp(arg1,"5") == 0) { *p  = 5; return 0; }
	if (strcmp(arg1,"6") == 0) { *p  = 6; return 0; }
	if (strcmp(arg1,"8") == 0) { *p  = 8; return 0; }
	if (strcmp(arg1,"9") == 0) { *p  = 9; return 0; }
	i = sscanf(arg1,"%d:%lf:%lf", &v3_2, &major, &minor);
	if (i == 2 && v3_2 == 1) {
	    /* sphere with user define radius */
	    if (GB2_Sec3_gdef(sec) == 101) fatal_error("Grid defintion 101 does not support user-defined radius","");
	    *p = 1;
	    if (major > 6400000.0 || major < 6300000.0) fatal_error("set_radius: radius must be in meters","");
	    best_scaled_value(major,&scale,&val);
	    p[1] = scale & 255;
	    uint_char(val, p+2);
	    return 0;
	}
	if (i == 3 && (v3_2 == 3 || v3_2 == 7)) {
	    if (GB2_Sec3_gdef(sec) == 101) fatal_error("Grid defintion 101 does not support user-defined major/minor axes","");
	    if (v3_2 == 7) {
	       if (major > 6400000.0 || major < 6300000.0) fatal_error("set_radius: major must be in meters","");
	       if (minor > 6400000.0 || minor < 6300000.0) fatal_error("set_radius: minor must be in meters","");
	       if (major < minor) fatal_error("set_radius: minor must be less than major","");
	       *p = 7;
            }
            else {
	       if (major > 6400.0 || major < 6300.0) fatal_error("set_radius: major must be in km","");
	       if (minor > 6400.0 || minor < 6300.0) fatal_error("set_radius: minor must be in km","");
	       if (major < minor) fatal_error("set_radius: minor must be less than major","");
	       *p = 3;
            }

	    best_scaled_value(major,&scale,&val);
	    p[6] = scale & 255;
	    uint_char(val, p+7);
	    best_scaled_value(minor,&scale,&val);
	    p[11] = scale & 255;
	    uint_char(val, p+12);
	    return 0;
	}
	fatal_error("set_radius: unknown argument %s",arg1);
	return 1;
    }
    return 0;
}


/* routine returns length of major and minor axis  */

int axes_earth(unsigned char **sec, double *major , double *minor) {
    int table_3_2;
    unsigned char *p;
    int factor, value;
    p = code_table_3_2_location(sec);

    if (p == NULL) fatal_error("radius_earth: code_table 3.2 is unknown","");
    table_3_2 = *p;

    /* set a default value .. not sure what to do with most values */
    *major = *minor = 6367.47 * 1000.0;

    if (table_3_2 == 0) *major = *minor = 6367.47 * 1000.0;
    else if (table_3_2 == 1)  {
	if (GB2_Sec3_gdef(sec) == 101) fatal_error("Grid defintion 101 does not support user-defined radius","");
        factor = INT1(p[1]);
        value = int4(p+2);
        *major = *minor = scaled2dbl(factor, value);
    }
    else if (table_3_2 == 2)  {
	*major = 6378.160 * 1000.0;
	*minor = 6356.775* 1000.0;
    }
    else if (table_3_2 == 3 || table_3_2 == 7) {
	if (GB2_Sec3_gdef(sec) == 101) fatal_error("Grid defintion 101 does not support user-defined major/minor axes","");
        /* get major axis */
        factor = INT1(p[6]);
        value = int4(p+7);
        *major = scaled2dbl(factor, value);
        /* get minor axis */
        factor = INT1(p[11]);
        value = int4(p+12);
        *minor =  scaled2dbl(factor, value);

        /* radius in km, convert to m */
        if (table_3_2 == 3) {
	    *major *= 1000.0;
	    *minor *= 1000.0;
	}
    }
    else if (table_3_2 == 4) {
	*major = 6378.137 * 1000.0;
	*minor = 6356.752140 * 1000.0;
    }
    else if (table_3_2 == 5) {
	*major = 6378.137 * 1000.0;
	*minor = 6356.752245 * 1000.0;
    }
    else if (table_3_2 == 6)  *major = *minor = 6371.2290 * 1000.0;
    else if (table_3_2 == 8)  *major = *minor = 6371.200 * 1000.0;
    else if (table_3_2 == 9) {
	*major = 6377563.396;
	*minor = 6356256.909;
    }

    return 0;
}



/* function to return the size of the earth */

double radius_earth(unsigned char **sec) {
    int table_3_2;
    double radius;
    unsigned char *p;
    int factor, value;

    p = code_table_3_2_location(sec);
    if (p == NULL) fatal_error("radius_earth: code_table 3.2 is unknown","");

    table_3_2 = (int) *p;

    /* set a default value .. not sure what to do with most values */
    radius = 6367.47 *1000.0;
    if (table_3_2 == 0) radius = 6367.47 * 1000.0;
    else if (table_3_2 == 1)  {
	factor = INT1(p[1]);
	value = int4(p+2);
	radius = scaled2dbl(factor, value);
	if (radius < 6300000.0 || radius > 6400000.0) 
	   fatal_error_i("radius of earth is %d m", (int) radius);
    }
    else if (table_3_2 == 2)  radius = (6378.160 + 6356.775)*0.5 * 1000.0;
    else if (table_3_2 == 3 || table_3_2 == 7) {
	if (GB2_Sec3_gdef(sec) == 101) fatal_error("Grid defintion 101 does not support user-defined major/minor axes","");
	/* get major axis */
	factor = INT1(p[6]);
	value = int4(p+7);
	radius = scaled2dbl(factor, value);
	/* get minor axis */
	factor = INT1(p[11]);
	value = int4(p+12);
	radius = (radius + scaled2dbl(factor, value)) * 0.5;

	/* radius in km, convert to m */
	if (table_3_2 == 3) radius *=  1000.0;

        if (radius < 6300000.0 || radius > 6400000.0) 
	   fatal_error_i("radius of earth is %d m", (int) radius);
    }
    else if (table_3_2 == 4)  radius = (6378.137 + 6356.752314)*0.5 * 1000.0;
    else if (table_3_2 == 5)  radius = (6378.137 + 6356.752245)*0.5 * 1000.0;
    else if (table_3_2 == 6)  radius = 6371.2290 * 1000.0;
    else if (table_3_2 == 8)  radius = 6371.200 * 1000.0;
    else if (table_3_2 == 9)  radius = (6377563.396 + 6356256.909)*0.5;

    return radius;
}

