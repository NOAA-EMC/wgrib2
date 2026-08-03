/** @file
 * @brief Options dealing with the precision/scaling of input.
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */

#include <stdio.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:510:scaling:inv:0:scaling for packing (old format)
 */

/**
 * Get scaling information for packing (old format).
 * 
 * In grib, grid values are usually stored as 
 * 
 * @code{}
 * grid_value = (ref_value + I*2**bin_scaling)) * 10**decimal_scaling
 * @endcode
 * 
 * - I is an positive integer 
 * - ref_value = 32-bit IEEE floating point number
 * - binary_scaling = -127..127
 * - decimal_scaling = -127..127
 * - number_of_bits = the smallest N such that I < 2**N
 * 
 * Note: WMO does not place a maximum value of the number of bits, however, implementations do 
 * have limits. The limits will vary by software package and procedure. 
 * 
 * This is not an absolute as the grid point values can be stored as a spectral coefficients, 
 * IEEE floating point values and other formats. The -scaling option prints the binary and scaling 
 * factors, the reference value and the number of bits used for the integer I. 
 * 
 * ## Usage
 * -scaling
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180  -scaling -for 13:14 -v -packing
 * 13:473374:scaling ref=2.32109e+06 dec_scale=-3 bin_scale=4 nbits=16:packing=grid point data - jpeg2000 compression,j val=(2.32109e+06+i*2^4)*10^-3, i=0..65535 (#bits=16)
 * 14:528630:scaling ref=2215 dec_scale=-1 bin_scale=0 nbits=10:packing=grid point data - jpeg2000 compression,j val=(2215+i*2^0)*10^-1, i=0..1023 (#bits=10)
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_scaling(ARG0) {

    int dec, bin, nbits;
    double base;
    if (mode < 0) return 0;
    if (scaling(sec, &base, &dec, &bin, &nbits) == 0) {
        sprintf(inv_out,"scaling ref=%g dec_scale=%d bin_scale=%d nbits=%d", base, dec, bin, nbits);
    }
    return 0;
}

/*
 * HEADER:510:scale:inv:0:scale for packing
 */

/**
 * Get scale information for packing.
 * 
 * In grib, grid values are usually stored as 
 * @code{}
 * grid_value = (ref_value + I*2**bin_scaling)) * 10**decimal_scaling
 * @endcode
 * 
 * - ref_value = 32-bit IEEE floating point number
 * - I is a positive integer
 * - binary_scaling = -127..127
 * - decimal_scaling = -127..127
 * 
 * This is not an absolute as the grid point values can be stored as a spectral coefficients, 
 * IEEE floating point values and other formats. The -scale option prints the binary and scaling 
 * factors. 
 * 
 * ## Usage
 * -scale
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180  -scale -for 13:14
 * 13:473374:scale=-3,4
 * 14:528630:scale=-1,0
 * $ wgrib2 gep19.t00z.pgrb2af180  -scale -for 13:14 -v -packing
 * 13:473374:scale=-3,4:packing=grid point data - jpeg2000 compression,j val=(2.32109e+06+i*2^4)*10^-3, i=0..65535 (#bits=16)
 * 14:528630:scale=-1,0:packing=grid point data - jpeg2000 compression,j val=(2215+i*2^0)*10^-1, i=0..1023 (#bits=10)
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_scale(ARG0) {

    int dec, bin, nbits;
    double base;
    if (mode < 0) return 0;
    if (scaling(sec, &base, &dec, &bin, &nbits) == 0) {
        sprintf(inv_out,"scale=%d,%d", dec, bin);
    }
    return 0;
}

