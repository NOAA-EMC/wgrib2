#include <stdio.h>
#include <stdlib.h>
#ifdef CALLABLE_WGRIB2
#include <setjmp.h>
#endif
#include "wgrib2.h"

/*
 * write fatal error message .. so to have common format 
 */
#ifdef CALLABLE_WGRIB2
extern jmp_buf fatal_err;
#endif

void fatal_error(const char *fmt, const char *string)
{
    fprintf(stderr, "\n*** FATAL ERROR: ");
    fprintf(stderr, fmt, string);
    fprintf(stderr," ***\n\n");
#ifndef SIMPLE_FATAL
    err_bin(1); err_string(1);
#endif
#ifdef CALLABLE_WGRIB2
    longjmp(fatal_err,1);
#endif
    exit(8);
    return;
}

void fatal_error_ss(const char *fmt, const char *string1,const char *string2)
{
    fprintf(stderr, "\n*** FATAL ERROR: ");
    fprintf(stderr, fmt, string1,string2);
    fprintf(stderr," ***\n\n");
#ifndef SIMPLE_FATAL
    err_bin(1); err_string(1);
#endif
#ifdef CALLABLE_WGRIB2
    longjmp(fatal_err,1);
#endif
    exit(8);
    return;
}

void fatal_error_i(const char *fmt, const int i)
{
    fprintf(stderr, "\n*** FATAL ERROR: ");
    fprintf(stderr, fmt, i);
    fprintf(stderr," ***\n\n");
#ifndef SIMPLE_FATAL
    err_bin(1); err_string(1);
#endif
#ifdef CALLABLE_WGRIB2
    longjmp(fatal_err,1);
#endif
    exit(8);
    return;
}
void fatal_error_u(const char *fmt, const unsigned int i)
{
    fprintf(stderr, "\n*** FATAL ERROR: ");
    fprintf(stderr, fmt, i);
    fprintf(stderr," ***\n\n");
#ifndef SIMPLE_FATAL
    err_bin(1); err_string(1);
#endif
#ifdef CALLABLE_WGRIB2
    longjmp(fatal_err,1);
#endif
    exit(8);
    return;
}

void fatal_error_ii(const char *fmt, const int i, const int j)
{
    fprintf(stderr, "\n*** FATAL ERROR: ");
    fprintf(stderr, fmt, i, j);
    fprintf(stderr," ***\n\n");
#ifndef SIMPLE_FATAL
    err_bin(1); err_string(1);
#endif
#ifdef CALLABLE_WGRIB2
    longjmp(fatal_err,1);
#endif
    exit(8);
    return;
}

void fatal_error_uu(const char *fmt, const unsigned int i, const unsigned int j)
{
    fprintf(stderr, "\n*** FATAL ERROR: ");
    fprintf(stderr, fmt, i, j);
    fprintf(stderr," ***\n\n");
#ifndef SIMPLE_FATAL
    err_bin(1); err_string(1);
#endif
#ifdef CALLABLE_WGRIB2
    longjmp(fatal_err,1);
#endif
    exit(8);
    return;
}
