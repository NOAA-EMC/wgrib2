#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:200:d:setup:1:dump message X (n or n.m), only 1 -d allowed
 */

extern int dump_msg, dump_submsg;
extern enum input_type input;

int f_d(ARG1) {
    const char *s;

    if (mode == -1) {
	if (input != all_mode) fprintf(stderr,"*** Warning -d %s overrides earlier -i, -d options\n", arg1);
        input = dump_mode;
        s = arg1;

	while (isspace( (unsigned char) *s)) { s++; }	/* get rid of leading blanks */

        dump_msg = 0;
        while (isdigit((unsigned char) *s)) {
            dump_msg = 10*dump_msg + *s++ - '0';
        }
        if (*s == '.') {
            dump_submsg = 0;
            s++;
            while (isdigit((unsigned char) *s)) {
                dump_submsg = 10*dump_submsg + *s++ - '0';
            }
        }
        else {
            dump_submsg = 1;
        }
        return (dump_msg <= 0);
    }
    return 0;
}
