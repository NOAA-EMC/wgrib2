/** @file
 * @brief Routines to set grid point values.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2/2009 | W. Ebisuzaki | Initial
 * 5/2014 | W. Ebisuzaki | Staggered grid support, added set_ival
 * @author Public Domain: Wesley Ebisuzaki @date 2/2009
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

/** Number of grid points in the x direction. */
extern int nx;

/** Number of grid points in the y direction. */
extern int ny;

/** Number of grid points. */
extern unsigned int npnts;

/*
 * HEADER:100:set_ijval:misc:3:sets grid point value X=ix Y=iy Z=val
 */

/**
 * Sets one grid point value of the decoded grid.
 * 
 * After changing the grid value, one usually writes out the grid using -grib_out FILE. The 
 * -set_ijval option only works when the grid is a rectangular array. For example, staggered 
 * and thinned grids are not stored an an array. 
 * 
 * ## Usage
 * -set_ijval I J VAL
 * 
 * grid(I,J) = VAL
 * I = 1 to NX
 * J = 1 to NY
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 small.grb2 small.grb2 -set_ijval 1 1 91 -set_ijval 2 1 92 -set_ijval 1 2 93 -set_ijval 2 2 94 -grib_out new.grb2
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * $ wgrib2 new.grb2 -csv new.csv
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * $ cat new.csv
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",0,20,91
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",10,20,92
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",0,28,93
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",10,28,94
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2/2009
 */
int f_set_ijval(ARG3) {

    size_t i;
    long int tmp;
    struct local_struct {
        unsigned int ix, iy;
	float val;
    };
    struct local_struct *save;

    unsigned int x,y;

    if (mode == -1) {
        decode = 1;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_ijval","");

        tmp = atol(arg1);
        if (tmp < 1) fatal_error_i("ijval: ix value (%d) should be >= 1", (int) tmp);
        save->ix = atol(arg1) - 1;

        tmp = atol(arg2);
        if (tmp < 1) fatal_error_i("ijval: iy value (%d) should be >= 1", (int) tmp);
        save->iy = atol(arg2) - 1;
        save->val = atof(arg3);

    }
    else if (mode == -2) {
        free(*local);
    }
    if (mode < 0) return 0;
    save = (struct local_struct *) *local;

    x = save->ix;
    y = save->iy;

    if (GDS_Scan_staggered(scan)) fatal_error("ijval: does not support staggered grid","");
    i = x + y * (size_t) nx;
    if (i < ndata) data[i] = save->val;
    else fatal_error_uu("ijval: failed with (%ux%u)",x,y);
    return 0;
}

/*
 * HEADER:100:set_ival:misc:2:sets grid point value X=i1:i2:.. Y=va1:val2:.. grid[i1] = val1,etc  i>0
 */

/**
 * Sets one or more grid point values of the decoded grid.
 * 
 * After changing the grid value, one usually writes out the grid using -grib_out FILE.
 * 
 * ## Usage
 * -set_ival I VAL
 * -set_ival I1:I2:..:In VAL1:VAL2:..:VALn
 * 
 * I, I1, .. In is the grid point from 1 to npnts
 * VAL, VAL1, .. VALn are floating point values
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 small.grb2 -set_ival 1:2:3:4 91:92:93:94 -gribout new.grb2
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * $ wgrib2 new.grb2 -csv new.csv
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * $ cat new.csv
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",0,20,91
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",10,20,92
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",0,28,93
 * "2009-06-05 00:00:00","2009-06-12 12:00:00","HGT","200 mb",10,28,94
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2/2009
 */
int f_set_ival(ARG2) {
    int icnt, vcnt;
    int err1, err2;
    float val;
    unsigned int i;

    if (mode == -1) {
        decode = 1;
    }
    if (mode < 0) return 0;

    err1 = sscanf(arg1,"%u%n", &i, &icnt);
    err2 = sscanf(arg2,"%f%n", &val, &vcnt);
    while (err1 == 1 && err2 == 1) {
// fprintf(stderr,"set_ival i=%u v=%f\n",i,val);        
        if (i != 0 && i <= ndata) data[i-1] = val;
        else fatal_error_uu("set_ival: i=%u ndata=%u", i, ndata);
        arg1 += icnt;
        arg2 += vcnt;
        err1 = sscanf(arg1,":%u%n", &i, &icnt);
        err2 = sscanf(arg2,":%f%n", &val, &vcnt);
    }
    if (err1 != err2) fatal_error_ii("set_ival: size of list do not match %d %d",err1,err2);
    return 0;
}
