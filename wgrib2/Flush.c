#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int flush_mode;


/*
 * HEADER:-1:flush:setup:0:flush output buffers after every write (interactive)
 */
int f_flush(ARG0) {
    flush_mode = 1;
    return 0;
}

/*
 * HEADER:-1:no_flush:setup:0:flush output buffers when full (default)
 */
int f_no_flush(ARG0) {
    flush_mode = 0;
    return 0;
}
