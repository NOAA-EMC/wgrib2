/** @file
 * @brief Calculate CRC32 checksum for sections and the whole message. The checksum
 * algorithm is the same as the POSIX 'cksum' utility uses.
 * @author Public Domain: R.N. Bokhorst, reinoud.bokhorst@bmtargoss.com 
 * @date 06/2009
 */

/*
 * Checksum.c
 *   Calculate CRC32 checksum for sections and the whole message. The checksum
 *   algorithm is the same as the POSIX 'cksum' utility uses.
 *
 * June 2009: R.N. Bokhorst, reinoud.bokhorst@bmtargoss.com, Public Domain
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/*
 * HEADER:400:checksum:inv:1:CRC checksum of section X (0..8), whole message (X = -1/message) or (X=data)
 */

/**
 * Writes the checksum (32 bit CRC) for the entire grib message, the decoded grid-point 
 * data or any specified section.
 * 
 * ## Usage
 * -checksum N
 * 
 * <pre>
 * Where N is:
 *   -1: whole message
 *   1-8: section number
 *   data: grid-point data
 * </pre>
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @note Two sections or messages with the same checksum are very probably the same. If the grid-point 
 * data has the same checksum, they are very probably bitwise identical. This option can be used to check 
 * the integrity of a grib message or to check for for identical sections. Note that decoded grid-point 
 * values may not be unique. Wgrib2 is compiled with the "fast" option which may sacrifice some precision 
 * for speed or uniqueness. For example, A*B*C should be calculated by (A*B)*C. However A*(B*C) will be faster 
 * if B*C was previously calculated. While mathematically the expressions are the same, the final results may 
 * be slightly different. 
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -checksum 3
 * 1:0:sec3_cksum=4006285726
 * 2:4786:sec3_cksum=4006285726
 * 3:9572:sec3_cksum=4006285726
 * 4:13335:sec3_cksum=4006285726
 * 5:17098:sec3_cksum=4006285726
 * @endcode
 * 
 * Section 3 is the Grid Definition Section (GDS). All 5 grib messages have the same GDS.
 * 
 * @code{.sh}
 * $ wgrib2 png.grb2 -checksum -1
 * 1:4:msg_cksum=827378178
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -checksum 3 | cut -f3 -d: | sort -u | wc -l
 * 1
 * @endcode
 * 
 * The above example prints out the number of grid defintion sections in the file by (1) creating 
 * the checksum for the GDS, (2) extracting the GDS checksum, (3) finding the unique checksums 
 * and finally counting them.
 * 
 * Space can be saved by putting combining like grib messages.  For example, if a 100 messages share 
 * the same bitmap and discipline, then the 100 messages could be combined into one message with a 
 * hundred submessages.  By combining the message, only one copy of the bitmap is needed.  This saves 
 * 99 copies of the bitmap.
 * 
 * @code{.sh}
 * $ wgrib2 test.grb2 -checksum 6 | sort -k3,3 -t: | wgrib2 test.grib -i -tosubmsg new.grb2
 * 1:0:d=2008120200:TMP:800 mb:anl:
 * 2:4786:d=2008120200:TMP:750 mb:anl:
 * 3:9572:d=2008120200:RH:800 mb:anl:
 * 4:13335:d=2008120200:RH:750 mb:anl:
 * 5:17098:d=2008120200:TMP:2743 m above mean sea level:anl:
 * @endcode
 * 
 * Submessage statistics:
 * - # submessages written  : 5
 * - Kbytes saved           : 0
 * - Kbytes written         : 20
 * 
 * @author R.N. Bokhorst @date 06/2009
 */
int f_checksum(ARG1) {
    unsigned int crc;
    size_t len;
    char *s;
    int i;

    if (mode == -1) {
        if (strcmp("data",arg1) == 0) decode = 1;
        return 0;
    }
    if (mode >= 0) {

        /* Message data */
        if (strcmp("data",arg1) == 0) {
            if (data) {
                crc = cksum((char unsigned *) data, sizeof(float) * (size_t) ndata);
                if (mode == 1) sprintf(inv_out, "Data checksum = 0x%08X", crc);     // hex, the default
                else if (mode == 2) sprintf(inv_out, "Data checksum = %u", crc); // decimal
                else sprintf(inv_out, "data_cksum=%u", crc); // decimal, for sorting
            }
            return 0;
        }

        /* Message checksum */

        i = (int) strtol(arg1, &s, 10);
        if (strcmp("message",arg1) == 0 || i == -1) {
            crc = cksum( sec[0], (size_t) GB2_MsgLen(sec) );
            if (mode == 1) sprintf(inv_out, "Msg checksum = 0x%08X", crc);     // hex, the default
            else if (mode == 2) sprintf(inv_out, "Msg checksum = %u", crc); // decimal
            else sprintf(inv_out, "msg_cksum=%u", crc); // decimal, for sorting
            return 0;
        }


        /* First argument is section number or -1 */

        if (*s != '\0' || i < -2  || i > 8 ) {
            fatal_error("Invalid argument '%s' for checksum", arg1);
        }

        /* Section checksum */

        if (sec[i] == NULL) {
            crc = 0;
        }
        else {
            if (i == 0) {
                len = GB2_Sec0_size;
            }
            else if (i == 8) {
                len = GB2_Sec8_size;
            }
            else {
                len = uint4(&(sec[i][0]));
            }
            crc = cksum( sec[i], len );
        }

        if (mode == 1) sprintf(inv_out, "Sec%d checksum = 0x%08X", i, crc);       // hex, the default
        else if (mode == 2) sprintf(inv_out, "Sec%d checksum = %u", i, crc);      // decimal
        else sprintf(inv_out, "sec%d_cksum=%u",i, crc);                            // decimal, for sorting
    }

    return 0;
}
