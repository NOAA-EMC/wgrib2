#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * if_delayed_error
 *
 * 12/2020 in public domain Wesley Ebisuzaki
 */

/*
 * HEADER:100:if_delayed_error:If:0:if delayed error
 */

extern int run_flag;
extern unsigned int last_message;

int f_if_delayed_error(ARG0)  {
    if (mode < 0) return 0;
    /* if delayed_error .. run_flag = 1 otherwise 0 */
    run_flag = (last_message >> 1) ? 1 : 0;
    return 0;
}

