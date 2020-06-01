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

extern int decode;

/*
 * HEADER:400:checksum:inv:1:CRC checksum of section X (0..8), whole message (X = -1/message) or (X=data)
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
