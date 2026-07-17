/** @file
 * @brief Contents of GRIB section 0 (Indicator Section).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 12/2006 | W. Ebisuzaki | Initial 
 * 1/2007 | M. Schwarb | Cleanup
 * @author Public domain: Wesley Ebisuzaki @date 12/2006
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:200:Sec0:inv:0:contents of section0
 */

/**
 * Prints a short summary of Section 0, the Indicator Section. 
 * 
 * ## Usage
 * -Sec0
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
 * $ wgrib2 png.grb2 -Sec0
 * 1:4:Sec0=GRIB reserved 0x0000 Discipline=0 Grib_Edition=2 len=44444
 * @endcode
 * 
 * <pre>
 * GRIB                          octets 1-4, should be GRIB
 * reserved 0x0000               octects 5-6, 0x0000 if not used
 * Discipline=0                  Discipline, Table 0.0
 * Grib_Edition=2                Edition, 2 for grib version2
 * len=44444                     length in octets/bytes of entire grib message
 * </pre>
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int f_Sec0(ARG0) {

    unsigned char *p;
    if (mode >= 0) {
        p = sec[0];
        sprintf(inv_out,
           "Sec0=%c%c%c%c reserved 0x%.2x%.2x Discipline=%d Grib_Edition=%d len=%lu"
            , p[0], p[1], p[2], p[3], p[4], p[5], (int) GB2_Discipline(sec), (int) GB2_Edition(sec),
            (unsigned long int) GB2_MsgLen(sec));
    }
    return 0;
}

/*
 * HEADER:200:table:inv:0:parameter table
 */

/**
 * Prints the values of discipline, master table and local table. 
 * 
 * ## Usage
 * -set_table
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 f.grb -table
 * 1:0:discipline=0 master_table=2 local_table=1
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2007
 */
int f_table(ARG0) {

    if (mode >= 0) {
        sprintf(inv_out, "discipline=%d", GB2_Discipline(sec));
        inv_out += strlen(inv_out);
        sprintf(inv_out," master_table=%d", GB2_MasterTable(sec));
        inv_out += strlen(inv_out);
        sprintf(inv_out," local_table=%d", GB2_LocalTable(sec));
    }
    return 0;
}

