#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"


/* 
 *  Dump.c
 *     2004        Public Domain Wesley Ebisuzaki
 *     3/2020      George Trojan: add optional dump offset
 */



/*
 * HEADER:200:d:setup:1:dump message X = n, n.m, n:offset, n.m:offset, only 1 -d allowed
 */

extern int dump_msg, dump_submsg;
extern size_t dump_offset;

extern enum input_type input;

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
