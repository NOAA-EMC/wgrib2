/* This is a test of the grb2_mk_inv() function, which is part of the C API for wgrib2.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_test_util.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "junk_c_api.inv"
#define EXP_GRB_INV "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"

int
main()
{
    printf("Testing grb2_mk_inv()...\n");
    {
        int ret;

        if ((ret = grb2_mk_inv(GRB_FILE, GRB_INV))) {
            printf("grb2_mk_inv() failed with return code %d\n", ret);
            return 2;
        }

        if ((ret = compare_files(GRB_INV, EXP_GRB_INV))) {
            printf("Inventory files differ.\n");
            return 3;
        }
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}