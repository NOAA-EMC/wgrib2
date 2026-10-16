/* This is a test of the grb2_wrt() function, which is part of the C API for wgrib2.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_test_util.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GRIDSIZE 36391
#define TEMPLATE_FILE "data/small_grib_file.grb2"
#define NEW_GRIB "grb2_wrt.grb2"
#define REF_GRIB "data/ref_grb2_wrt.grb2"

int wgrib2_set_reg(float *data, size_t size, int reg);
int wgrib2_set_mem_buffer(const unsigned char *my_buffer, size_t size, int n);

int
main()
{
    float data[GRIDSIZE];
    int msgno, ndata;
    int i, ret;

    printf("Testing grb2_wrt()...\n");

    msgno = 1;

    for (i = 0; i < GRIDSIZE; i++) data[i] = 10.0;

    printf("Testing with ndata = 0. Should return 1.\n");
    ndata = 0;

    if ((ret = grb2_wrt(NEW_GRIB, TEMPLATE_FILE, msgno, data, ndata)) != 1) {
        printf("ERROR: grb2_wrt() returned %d.\n", ret);
        return 1;
    }

    printf("Testing standard call of grb2_wrt(). Should return 0.\n");
    ndata = GRIDSIZE;
    ret = grb2_wrt(NEW_GRIB, TEMPLATE_FILE, msgno, data, ndata, "lev", "2 in sequence", "grib_type", "s",
            "ftime", "anl", "var", "SWELL", "meta", "1:0:d=2021113000:SWELL:2 in sequence:anl:",
            "bin_prec", 7, "percentile", 50, "set", "center", 255, "set", "model_version_date", 2021113000LL, 
            "date",2021113000LL);

    if (ret != 0) {
        printf("ERROR: grb2_wrt() returned %d.\n", ret);
        return 2;
    }

    if ((ret = compare_grib2_files(NEW_GRIB, REF_GRIB)) != 0) {
        printf("ERROR: compare_grib2_files() returned %d.\n", ret);
        return 3;
    }

    printf("Testing standard call of grb2_wrt() with invalid options. Should return 0.\n");
    ndata = GRIDSIZE;
    if ((ret = grb2_wrt("tmp.grb2", TEMPLATE_FILE, msgno, data, ndata, "invalid_option", 123)) != 0) {
        printf("ERROR: grb2_wrt() returned %d.\n", ret);
        return 4;
    }

    printf("SUCCESS!\n");
    return 0;
}