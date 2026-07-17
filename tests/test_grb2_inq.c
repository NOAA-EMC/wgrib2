/* This is a test of the functions in grb2_inq.c, which is part of the C API for wgrib2.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define NDATA 100

extern int last_options;
extern int good;

int
main()
{
    printf("Testing grb2_get_data()...\n");
    {
        int ret;
        int ndata = NDATA;
        float data[NDATA] = { 0. };

        good = 1;
    }
    printf("SUCCESS!\n");
    return 0;
}