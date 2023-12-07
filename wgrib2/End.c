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
extern unsigned int last_message;                /* last message to process */
int f_end(ARG0) {
    // if last message is already set,  don't change error code
    if (mode >= 0 && last_message == 0) last_message = 1;
    return 0;
}

