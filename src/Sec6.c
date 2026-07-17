/** @file
 * @brief Some routines that involving Section 6 (Bitmap Section).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2006 | W. Ebisuzaki | Initial
 * 1/2007 | M. Schwarb   | Cleanup
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */

#include <stdio.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:700:bitmap:inv:0:bitmap mode
 */

/**
 * Prints bitmap information.
 * 
 * GRIB2 has 3 ways to specify undefined values. If the grid point data is stored as IEEE 
 * format numbers, then the IEEE format has NaN (not a number). NaN values can considered to 
 * be a undefined value. For complex packing, values that are outside the range of normal values 
 * are undefined. Decoders can give these undefined values a special value. All the packing 
 * methods support a bitmap for specifying undefined values. 
 * 
 * The -bitmap option prints whether the record has a bitmap and the number of undefined points 
 * as specified by the bitmap. It is possible for an IEEE format packing to defined undefined by 
 * both NaN and a bitmap. So in this case the number of undefined values will be larger than 
 * specified by the bitmap. The same is true for complex packing where a bitmap could be combined 
 * with special values undefineds. Combining two methods of specifying undefineds would increase 
 * the file size, so it is not recommended. 
 * 
 * Note that using a bitmap to specify undefined values is not as efficient as using special-value 
 * undefineds. The file size is significantly bigger and decoding can be slower (wgrib2's code is 
 * parallelized for special values). 
 * 
 * Note that wgrib2 uses 9.999e20 for all undefined values including special value undefineds. (The 
 * special value is ignored.) 
 * 
 * ## Usage
 * -bitmap
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ wgrib2 grib2.polar -bitmap
 * 1.1:0:no bitmap
 * 1.2:0:no bitmap
 * @endcode

 * @author Wesley Ebisuzaki @date 2006
 */
int f_bitmap(ARG0) {

    int i;
    unsigned int nmiss;
    if (mode >= 0) {
        i = code_table_6_0(sec);
        if (i == 0) {
    //	    nmiss = GB2_Sec3_npts(sec)-uint4(sec[5]+5);
            nmiss = GB2_Sec3_npts(sec) - GB2_Sec5_nval(sec);
            sprintf(inv_out,"bitmap %d undef pts", nmiss);
            if (nmiss != missing_points(sec[6]+6, GB2_Sec3_npts(sec)))
                fatal_error("inconsistent number of undefined points","");
        }
        else if (i == 255) {
            sprintf(inv_out,"no bitmap");
        }
        else if (i == 254) {
            sprintf(inv_out,"old bitmap");
        }
        else {
            sprintf(inv_out, "predefined bitmap #%d", i);
        }
    }
    return 0;
}

/*
 * HEADER:700:Sec6:inv:0:show bit-map section
 */

/**
 * Prints a short summary of Section 6, the Bit Map Section. 
 * 
 * ## Usage
 * -Sec6
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 png.grb2 -Sec6
 * 1:4:Sec6 length 6 bitmap indicator 255
 * @endcode
 * 
 * <pre>
 * length=6                       Section 6 is 6 octets/bytes in length
 * bitmap indicator 255           Table 6.0 has a value of 255, (no bitmap)
 * </pre>
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_Sec6(ARG0) {

    if (mode >= 0) {
        sprintf(inv_out,"Sec6 length %u bitmap indicator %u", uint4(sec[6]),
            (unsigned int) sec[6][5]);
    }
    return 0;
}
