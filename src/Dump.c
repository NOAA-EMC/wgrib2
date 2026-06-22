/** @file
 * @brief Specifies the grib message number to process.
 * @author Public Domain: Wesley Ebisuzaki @date 2004
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2004 | W. Ebisuzaki | Initial
 * 3/2020 | G. Trojan | Added optional dump offset
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>

#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:200:d:setup:1:dump message X = n, n.m, n:offset, n.m:offset, only 1 -d allowed
 */

 /** Pointer to the grib message number to process. */
extern int dump_msg;

/** Pointer to the sub-message number to process. */
extern int dump_submsg;

/** Offset in bytes to skip before processing the dump. */
extern size_t dump_offset;

/** Input type. */
extern enum input_type input;

/** 
 * Specifies the grib message number (N) to process. 
 * 
 * The -d N option comes from wgrib where "d" stood fom "dump". 
 * 
 * If Nth message has submessage, then the first submessage is chosen (N.1). For messages 
 * with submessages, use -d N.M to select the Mth submessage of the Nth message. 
 * If M is missing, only the first submessage is selected. 
 * 
 * You can also add an optional offset. For example, -d N:OFFSET will skip OFFSET bytes 
 * (must be positive), find the next grib message, give it a label N, and dump it. The 
 * other form, -d N.M:OFFSET will skip OFFSET bytes (must be positive), find the next grib 
 * message and dump the Mth submessage of that grib message. Using the offset option can be 
 * faster for large files, and can be used to skip sections of a grib file that stop wgrib2 
 * processing. The -d option works for files and pipes.
 * 
 * If there are more than one -d on a command line, only the last one is used. 
 * 
 * The form, -d all, is not valid in wgrib2. This is the default action. 
 * 
 * The -d should be used with caution. The order of messages records within grib files can 
 * change without notice (ex. today's forecast may have a different order from tomorrow's forecast). 
 * I use -d interactively but this option should not be used in scripts unless you are 100% 
 * certain of the order of the records. 
 * 
 * ## Usage
 * -d N
 * -d N.M
 * -d N:OFFSET
 * -d N.M:OFFSET
 * 
 * N is an integer larger than 0, M is an positive integer, OFFSET is a positive integer
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * ## Example:
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2004
*/
int f_d(ARG1) {
    const char *s;

    if (mode == -1) {
        if (input != all_mode) fprintf(stderr,"*** Warning -d %s overrides earlier -i, -d options\n", arg1);
        input = dump_mode;
        s = arg1;

        while (isspace( (unsigned char) *s)) { s++; }	/* get rid of leading blanks */

        dump_msg = 0;
        dump_submsg = 1;
        dump_offset = 0;

        /* get d or d.n */
        while (isdigit((unsigned char) *s)) {
            dump_msg = 10*dump_msg + *s++ - '0';
        }
        if (*s == '.') {
            s++;
            dump_submsg = 0;
            while (isdigit((unsigned char) *s)) {
                dump_submsg = 10*dump_submsg + *s++ - '0';
            }
        }

        /*
        * dump_offset:  0       not being used
        *               n > 0   skip (n-1) bytes
        * note: offset starts at 0
        */

        /* get :offset */

        if (*s == ':') {
            s++;
            while (isdigit((unsigned char) *s)) {
                dump_offset = 10*dump_offset + *s++ - '0';
            }
            dump_offset++;
        }
    }
    return 0;
}
