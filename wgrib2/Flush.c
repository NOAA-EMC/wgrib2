#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Flush.c          10/2024   Public Domain    Wesley Ebisuzaki
 *
 * Most C programs flush the output when the buffers are full.
 * This is usually the efficient method.
 *
 * However, life is different when you start writting to pipe instead of disk files.
 * In this case, you want to flush the output buffers at the end of the write.
 * If you don't flush after every write, the pipe line can stall and never complete.
 * Wgrib2 gets around the stalling pipeline by checking the output files to see
 * whether is a pipe.  If so, it turns on the flush mode.  This causes a flush
 * after all writes.
 *
 * The automatic detection for flush mode works well. However, it is stll
 *  possible to turn on the flush mode manually which was done in the old
 *  days.
 */

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
