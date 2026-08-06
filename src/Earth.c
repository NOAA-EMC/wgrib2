/** @file
 * @brief Set or print the shape and radius of the Earth.
 * @author Public Domain: Wesley Ebisuzaki @date 2010
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2010 | W. Ebisuzaki | Initial
 * 06/2017 | W. Ebisuzaki | Print semi-major/minor axes
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:radius:inv:0:radius of Earth
 */

/**
 * Prints the shape and size of Earth according to Code Table 3.2.
 * 
 * Such information is necessary for finding the latitude and longitude of the grid 
 * points in various projections.
 * 
 * ## Usage:
 * -radius
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 -radius png.grb2
 * 1:4:code3.2=6 sphere predefined radius=6371229.0 m
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2010
 */
int f_radius(ARG0) {
    int table_3_2, is_spherical;
    double major, minor;

    if (mode < 0) return 0;

    table_3_2 = code_table_3_2(sec);

    if (table_3_2 < 0) {
        sprintf(inv_out,"earth radius is not available");
       	return 0;
    }
    axes_earth(sec, &major, &minor, &is_spherical);
    sprintf(inv_out,"code3.2=%d radius_maj=%lf radiusmin=%lf is_spherical=%d", 
            table_3_2,major,minor,is_spherical);
    return 0;
}

/*
 * HEADER:100:set_radius:misc:1:set radius of Earth  X= 0,2,4,5,6,8,9 (Code Table 3.2), X=1:radius , X=7:major:minor
 */

/**
 * Sets the shape and size of Earth according to Code Table 3.2.
 * 
 * The geolocation of the grid points is done prior to the execution of the run-time 
 * options. So the -set_radius will not affect calculations of the lat-lon of the grid 
 * points. To get the correction locations after using the -set_radius option, you must 
 * write the file with the new shape of the Earth. Then you can use this new file. 
 * 
 * ## Usage
 * -set_radius N 
 *      N=0,2,4,5,6,8,9
 *      Code Table 3.2 is set to N
 * 
 * -set_radius 1:R
 *      R=radius in meters (spherical)
 *      Code Table 3.2 is set to 1
 * 
 * -set_radius 3:X:Y
 *      X=major axis Y=minor axis (oblate spheroid), X, Y in km
 *      Code Table 3.2 is set to 3
 * 
 * -set_radius 7:X:Y 
 *      X=major axis Y=minor axis (oblate spheroid), X, Y in m
 *      Code Table 3.2 is set to 7
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * @code{.sh}
 * $ wgrib2 small.grb2 -set_radius 0 -radius
 * 1:0:code3.2=0 sphere predefined radius=6367470.0 m
 * $ wgrib2 small.grb2 -set_radius 1:6300000 -radius
 * 1:0:code3.2=1 sphere user defined radius=6300000.0 m
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2010
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

/**
 * Returns the length of the major and minor axes.
 * 
 * @param sec Pointer to the section array.
 * @param major Pointer to store the length of the major axis in meters.
 * @param minor Pointer to store the length of the minor axis in meters.
 * @param is_spherical Pointer to store whether the Earth is spherical (1) or not (0).
 * 
 * @return 0 on success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 2010
 */
int axes_earth(unsigned char **sec, double *major, double *minor, int *is_spherical) {
    int table_3_2;
    unsigned char *p;
    int factor, value;
    p = code_table_3_2_location(sec);

    if (p == NULL) fatal_error("radius_earth: code_table 3.2 is unknown","");
    table_3_2 = *p;

    /* set a default value .. not sure what to do with most values */
    *major = *minor = 6367.47 * 1000.0;
    if (is_spherical) *is_spherical = 1;

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
        if (is_spherical) *is_spherical = 0;
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
        if (is_spherical) *is_spherical = 0;

        /* radius in km, convert to m */
        if (table_3_2 == 3) {
            *major *= 1000.0;
            *minor *= 1000.0;
        }
    }
    else if (table_3_2 == 4) {
        *major = 6378.137 * 1000.0;
        *minor = 6356.752140 * 1000.0;
        if (is_spherical) *is_spherical = 0;
    }
    else if (table_3_2 == 5) {
        *major = 6378.137 * 1000.0;
        *minor = 6356.752245 * 1000.0;
        if (is_spherical) *is_spherical = 0;
    }
    else if (table_3_2 == 6)  *major = *minor = 6371.2290 * 1000.0;
    else if (table_3_2 == 8)  *major = *minor = 6371.200 * 1000.0;
    else if (table_3_2 == 9) {
        *major = 6377563.396;
        *minor = 6356256.909;
        if (is_spherical) *is_spherical = 0;
    }
    else {
        fprintf(stderr,"WARNING: axes_earth: unknown code table 3.2 using radias=%lf m\n", *major);
    }

    return 0;
}

/** 
 * Returns the radius of the Earth in meters.
 * 
 * @param sec Pointer to the section array.
 * 
 * @return The average radius of the Earth in meters.
 * 
 * @author Wesley Ebisuzaki @date 2010
*/
double radius_earth(unsigned char **sec) {
    double radius_major, radius_minor, radius;
    int is_spherical;

    axes_earth(sec, &radius_major, &radius_minor, &is_spherical);
    radius = 0.5 * (radius_major + radius_minor);

    if (radius < 6300000.0 || radius > 6400000.0) 
        fatal_error_i("radius of earth is %d m", (int) radius);

    return radius;
}
