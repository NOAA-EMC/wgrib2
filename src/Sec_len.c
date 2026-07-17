/** @file
 * @brief Length of various GRIB sections.
 * @author Public Domain: Wesley Ebisuzaki @date 2007
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Message number. */
extern int msg_no;

/** Sub-message number. */
extern int submsg;

/*
 * HEADER:700:Sec_len:inv:0:length of various grib sections
 */

/**
 * Function to get the length of various GRIB sections.
 * 
 * Grib2 messages (records) are comprised of 9 sections (0-8). Sections 0 and 8 are 16 and 4 
 * bytes long, respectively. When the message contains submessages, each submessage contains 9 
 * sections but some of the sections can be shared with the other submessages. The -Set_late 
 * option shows the length of each section except for sections 0 and 8. 
 * 
 * ## Usage
 * -Sec_len
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 small.grb2 -Sec_len
 * 1:0:Sec size msg=188 id(1)=21 local(2)=0 grid(3)=72 product(4)=37 data-rep(5)=21 bitmap(6)=6 data(7)=11
 * @endcode
 * 
 * The output shows us that:
 * - The size of the grib message is 188 bytes.
 * - Section 1 is 21
 * - Section 2 is missing and not used
 * - Section 3 is 72
 * - Section 4 is 37
 * - Section 5 is 21
 * - Section 6 is 6
 * - Section 7 is 11
 * 
 * @author Wesley Ebisuzaki @date 2007
 */
int f_Sec_len(ARG0) {
    const char *new_sec2, *new_sec3;
    if (mode >= 0) {
        sprintf(inv_out,"Sec size msg=%lu", (unsigned long int) GB2_MsgLen(sec));
        inv_out += strlen(inv_out);

        new_sec2=new_sec3="";
        if (submsg != 1) {
            if (sec[3] + uint4(sec[3]) != sec[4]) {
                new_sec2 = new_sec3 = "*";
            }
            else {
                if ((sec[2] != NULL) && (sec[2] + uint4(sec[2]) == sec[3])) {
                    new_sec2 = "*";
                }
            }
        }

        sprintf(inv_out," id(1)=%u", uint4(sec[1]));
        inv_out += strlen(inv_out);

        if (sec[2] != NULL) 
           sprintf(inv_out," local(2)=%u%s", uint4(sec[2]),new_sec2);
        else
           sprintf(inv_out," local(2)=0");
        inv_out += strlen(inv_out);

        sprintf(inv_out," grid(3)=%u%s", uint4(sec[3]),new_sec3);
        inv_out += strlen(inv_out);

        sprintf(inv_out," product(4)=%u", uint4(sec[4]));
        inv_out += strlen(inv_out);

        sprintf(inv_out," data-rep(5)=%u", uint4(sec[5]));
        inv_out += strlen(inv_out);

        if (sec[5] != NULL) 
            sprintf(inv_out," bitmap(6)=%u", uint4(sec[6]));
        else
            sprintf(inv_out," bitmap(6)=0");
        inv_out += strlen(inv_out);

        sprintf(inv_out," data(7)=%u", uint4(sec[7]));
        inv_out += strlen(inv_out);
    }
    return 0;
}
