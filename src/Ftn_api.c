/** @file
 * @brief Routine called by grb2_inq in Fortran API.
 * @author Public Domain: Wesley Ebisuzaki @date 05/2015
 */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"
#include "grid_id.h"

/** Current output order type. */
extern enum output_order_type output_order;

/** Number of points in the grid. */
extern unsigned int npnts; 

/** Number of grid points in the x direction. */
extern unsigned int nx_;

/** Number of grid points in the y direction. */
extern unsigned int ny_;

/** Submessage number. */
extern int submsg;

/** Message number. */
extern int msg_no;

/** Inventory number. */
extern int inv_no;

/*
 * HEADER:100:ftn_api_fn0:inv:0:n npnts nx ny msg_no submsg i11,5(1x,i11)
 */

/**
 * Returns often needed information about a grib message.
 * 
 * This option is used by the grb2_inq function in the ftn_api for reading grib. Since
 * the ftn_api and wgrib2 are part of the same package, there is no need for compatibility 
 * between versions of wgrib2. Use of this option is not recommended. The current output is 
 * by this C statement:
 * 
 * sprintf(inv_out, "%8d %8u %8u %8u %8d %8d",inv_no,npnts,nx_,ny_,msg_no, submsg)
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 05/2015
 */
int f_ftn_api_fn0(ARG0) {
    /* ndata data may be undefined when data is not decoded */
    if (mode >= 0) sprintf(inv_out, "%11d %11u %11u %11u %11d %11d",inv_no,npnts,nx_,ny_,msg_no, submsg);
    return 0;   
}

