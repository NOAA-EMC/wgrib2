#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 2/2008 in public domain Wesley Ebisuzaki */

/*
 * HEADER:100:end:misc:0:stop after first (sub)message (save time)
 */
extern int last_message;                /* last message to process */
int f_end(ARG0) {
    if (mode >= 0) last_message = 1;
    return 0;
}

