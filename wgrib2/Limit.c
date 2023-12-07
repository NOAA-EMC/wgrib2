#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Limit.c  12/2020 Public Domain Wesley Ebisuzaki
 * limit stops after X decodes have been processed
 * this option is for NOMADS, so processing won't take forever
 */

extern int decode;
extern unsigned int last_message;
/*
 * HEADER:100:limit:misc:1:stops after X fields decoded
 */

int f_limit(ARG1) {
    int *save;
    if (mode == -1) {
	*local = save = (int *) malloc(sizeof(int));
	*save = atoi(arg1);
    }
    else if (mode == -2) {
	save = (int *) *local;
	free(save);
    }
    else if (mode >= 0 && decode == 1) {
	save = (int *) *local;
	if (--*save == 0) last_message = 1;
    }
    return 0;
}
