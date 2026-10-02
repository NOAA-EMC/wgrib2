/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the lambert2ll() routine from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include <setjmp.h>

#define TOL 1e-6

int lambert2ll(unsigned char **sec, double **lat, double **lon);

extern enum output_order_type output_order;
extern jmp_buf fatal_err;

int
main(){
    printf("Testing lambert2ll()...\n");
    printf("Test Case 1: Lambert conformal grid along the projection meridian.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[81] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 2;
        const double expected_lat[2] = {30.0, 42.1842765};
        const double expected_lon[2] = {0.0, 0.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 81;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 30;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[38] = 1;
        sec3[39] = 201;
        sec3[40] = 195;
        sec3[41] = 128;
        sec3[46] = 48;
        sec3[47] = 1;
        sec3[48] = 201;
        sec3[49] = 195;
        sec3[50] = 128;
        sec3[55] = 0;
        sec3[56] = 15;
        sec3[57] = 66;
        sec3[58] = 64;
        sec3[59] = 209;
        sec3[60] = 88;
        sec3[61] = 116;
        sec3[62] = 226;
        sec3[64] = 64;
        sec3[65] = 1;
        sec3[66] = 201;
        sec3[67] = 195;
        sec3[68] = 128;
        sec3[69] = 1;
        sec3[70] = 201;
        sec3[71] = 195;
        sec3[72] = 128;

        output_order = wesn;

        if (lambert2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("lambert2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 1;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("lambert2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 1;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 2: Lambert conformal grid with different secant latitudes.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[81] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 1;
        const double expected_lat[1] = {30.0};
        const double expected_lon[1] = {0.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 81;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 30;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[38] = 1;
        sec3[39] = 201;
        sec3[40] = 195;
        sec3[41] = 128;
        sec3[46] = 48;
        sec3[47] = 1;
        sec3[48] = 201;
        sec3[49] = 195;
        sec3[50] = 128;
        sec3[55] = 0;
        sec3[56] = 15;
        sec3[57] = 66;
        sec3[58] = 64;
        sec3[59] = 0;
        sec3[60] = 15;
        sec3[61] = 66;
        sec3[62] = 64;
        sec3[64] = 64;
        sec3[65] = 1;
        sec3[66] = 201;
        sec3[67] = 195;
        sec3[68] = 128;
        sec3[69] = 3;
        sec3[70] = 147;
        sec3[71] = 135;
        sec3[72] = 0;

        output_order = wesn;

        if (lambert2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("lambert2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 2;
        }
        
        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("lambert2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 2;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 3: Handling nnx < 1. Should return 1.\n");
    {
        int ret;
        unsigned char sec1[16] = {0};
        unsigned char sec3[65] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 65;
        sec3[4] = 3;
        sec3[13] = 30;
        sec3[37] = 1;
        sec3[64] = 64;

        ret = lambert2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("lambert2ll() returned %d for nnx < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 3;
        }
    }
    printf("Test Case 4: Handling nny < 1. Should return 1.\n");
    {
        int ret;
        unsigned char sec1[16] = {0};
        unsigned char sec3[65] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 65;
        sec3[4] = 3;
        sec3[13] = 30;
        sec3[33] = 1;
        sec3[64] = 64;

        ret = lambert2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("lambert2ll() returned %d for nny < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 4;
        }
    }
    printf("Test Case 5: Fatal error case where lon1r < 0.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[81] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 81;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 30;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[38] = 1;
        sec3[39] = 201;
        sec3[40] = 195;
        sec3[41] = 128;
        sec3[42] = 128;
        sec3[43] = 15;
        sec3[44] = 66;
        sec3[45] = 64;
        sec3[46] = 48;
        sec3[47] = 1;
        sec3[48] = 201;
        sec3[49] = 195;
        sec3[50] = 128;
        sec3[55] = 0;
        sec3[56] = 15;
        sec3[57] = 66;
        sec3[58] = 64;
        sec3[59] = 0;
        sec3[60] = 15;
        sec3[61] = 66;
        sec3[62] = 64;
        sec3[64] = 64;
        sec3[65] = 1;
        sec3[66] = 201;
        sec3[67] = 195;
        sec3[68] = 128;
        sec3[69] = 1;
        sec3[70] = 201;
        sec3[71] = 195;
        sec3[72] = 128;

        output_order = wesn;
        
        if (setjmp(fatal_err) == 0) {
            lambert2ll(sec, &lat, &lon);
            printf("lambert2ll() failed to trigger fatal_error for lon1r < 0.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 5;
        }
    }
    printf("Test Case 6: Fatal error due to stagger problem.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[81] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 81;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 30;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[38] = 1;
        sec3[39] = 201;
        sec3[40] = 195;
        sec3[41] = 128;
        sec3[46] = 48;
        sec3[47] = 1;
        sec3[48] = 201;
        sec3[49] = 195;
        sec3[50] = 128;
        sec3[55] = 0;
        sec3[56] = 15;
        sec3[57] = 66;
        sec3[58] = 64;
        sec3[59] = 0;
        sec3[60] = 15;
        sec3[61] = 66;
        sec3[62] = 64;
        sec3[64] = 64;
        sec3[65] = 1;
        sec3[66] = 201;
        sec3[67] = 195;
        sec3[68] = 128;
        sec3[69] = 1;
        sec3[70] = 201;
        sec3[71] = 195;
        sec3[72] = 128;

        output_order = raw;
        
        if (setjmp(fatal_err) == 0) {
            lambert2ll(sec, &lat, &lon);
            printf("lambert2ll() failed to trigger fatal_error for stagger problem.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 6;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}