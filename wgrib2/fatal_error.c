#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <setjmp.h>

#include "wgrib2.h"

/*
 * write fatal error message .. so to have common format 
 * Public Domain 2004 Wesley Ebisuzaki, George Trojan (2021)
 * v1.0   original release (2004)
 * v1.X   added fatal_error_XY, code for callable wgrib2, calls to err_bin, err_string
 * v2.0   replace various fatal_error*(..) by one routine using vfprintf(..)  George Trojan
 *
 *   fatal_error(ARGS) is replacement for
 *         fprintf(ARGS)
 *         do_fatal_error_processing
 */

extern jmp_buf fatal_err;



void fatal_error(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    fprintf(stderr, "\n*** FATAL ERROR: ");
    vfprintf(stderr, fmt, arg);
    fprintf(stderr," ***\n\n");
    va_end(arg);

    err_bin(1); err_string(1);

    longjmp(fatal_err,1);

    exit(8);
    return;
}
