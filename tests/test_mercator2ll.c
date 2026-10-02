/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the mercator2ll() routine from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include <setjmp.h>

#define TOL 1e-6

int mercator2ll(unsigned char **sec, double **lat, double **lon);

extern jmp_buf fatal_err;

int
main(){
    printf("Testing mercator2ll()...\n");
    printf("Test Case 1: Mercator grid with reverse x and y scanning.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 9;
        const double expected_lat[9] = {
            -45.0, -45.0, -45.0,
            0.0, 0.0, 0.0,
            45.0, 45.0, 45.0
        };
        const double expected_lon[9] = {
            0.0, 10.0, 20.0,
            0.0, 10.0, 20.0,
            0.0, 10.0, 20.0
        };

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 9;
        sec3[13] = 10;
        sec3[33] = 3;
        sec3[37] = 3;
        sec3[38] = 2;
        sec3[39] = 174;
        sec3[40] = 165;
        sec3[41] = 64;
        sec3[46] = 48;
        sec3[51] = 130;
        sec3[52] = 174;
        sec3[53] = 165;
        sec3[54] = 64;
        sec3[55] = 0;
        sec3[56] = 0;
        sec3[57] = 0;
        sec3[58] = 0;
        sec3[59] = 128;
        sec3[64] = 66;
        sec3[65] = 61;
        sec3[66] = 153;
        sec3[67] = 45;
        sec3[68] = 66;
        sec3[69] = 61;
        sec3[70] = 153;
        sec3[71] = 45;
        sec3[42] = 1;
        sec3[43] = 49;
        sec3[44] = 45;
        sec3[45] = 0;

        if (mercator2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("mercator2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 1;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("mercator2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 1;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 2: Mercator grid with positive y scanning.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 9;
        const double expected_lat[9] = {
            -45.0, -45.0, -45.0,
            0.0, 0.0, 0.0,
            45.0, 45.0, 45.0
        };
        const double expected_lon[9] = {
            0.0, 10.0, 20.0,
            0.0, 10.0, 20.0,
            0.0, 10.0, 20.0
        };

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 9;
        sec3[13] = 10;
        sec3[33] = 3;
        sec3[37] = 3;
        sec3[38] = 130;
        sec3[39] = 174;
        sec3[40] = 165;
        sec3[41] = 64;
        sec3[42] = 0;
        sec3[43] = 0;
        sec3[44] = 0;
        sec3[45] = 0;
        sec3[46] = 48;
        sec3[51] = 2;
        sec3[52] = 174;
        sec3[53] = 165;
        sec3[54] = 64;
        sec3[55] = 1;
        sec3[56] = 49;
        sec3[57] = 45;
        sec3[58] = 0;
        sec3[59] = 64;
        sec3[64] = 66;
        sec3[65] = 61;
        sec3[66] = 153;
        sec3[67] = 45;
        sec3[68] = 66;
        sec3[69] = 61;
        sec3[70] = 153;
        sec3[71] = 45;

        if (mercator2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("mercator2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 2;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("mercator2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 2;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 3: Mercator grid with positive x scanning.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 9;
        const double expected_lat[9] = {
            -45.0, -45.0, -45.0,
            0.0, 0.0, 0.0,
            45.0, 45.0, 45.0
        };
        const double expected_lon[9] = {
            0.0, 10.0, 20.0,
            0.0, 10.0, 20.0,
            0.0, 10.0, 20.0
        };

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 9;
        sec3[13] = 10;
        sec3[33] = 3;
        sec3[37] = 3;
        sec3[38] = 2;
        sec3[39] = 174;
        sec3[40] = 165;
        sec3[41] = 64;
        sec3[42] = 0;
        sec3[43] = 0;
        sec3[44] = 0;
        sec3[45] = 0;
        sec3[46] = 48;
        sec3[51] = 130;
        sec3[52] = 174;
        sec3[53] = 165;
        sec3[54] = 64;
        sec3[55] = 1;
        sec3[56] = 49;
        sec3[57] = 45;
        sec3[58] = 0;
        sec3[59] = 0;
        sec3[64] = 66;
        sec3[65] = 61;
        sec3[66] = 153;
        sec3[67] = 45;
        sec3[68] = 66;
        sec3[69] = 61;
        sec3[70] = 153;
        sec3[71] = 45;
      
        if (mercator2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("mercator2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 3;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("mercator2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 3;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 4: Handling nny < 1. Should return 1.\n");
    {
        int ret;
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[13] = 10;
        sec3[33] = 1;

        ret = mercator2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("mercator2ll() returned %d for nny < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 4;
        }

    }
    printf("Test Case 5: Handling nnx < 1. Should return 1.\n");
    {
        int ret;
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[13] = 10;
        sec3[37] = 1;

        ret = mercator2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("mercator2ll() returned %d for nnx < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 5;
        }

    }
    printf("Test Case 6: Handling non-zero mercator orientation angle. Should return 2.\n");
    {
        int ret;
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[63] = 1;

        ret = mercator2ll(sec, &lat, &lon);
        if (ret != 2) {
            printf("mercator2ll() returned %d for non-zero orientation angle. Expected 2.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 6;
        }

    }
    printf("Test Case 7: Fatal error case where lon1 > 360.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[42] = 21;
        sec3[43] = 117;
        sec3[44] = 42;
        sec3[45] = 1;
        
        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for lon > 360.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 7;
        }
    }
    printf("Test Case 8: Fatal error case where lon2 > 360.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[55] = 21;
        sec3[56] = 117;
        sec3[57] = 42;
        sec3[58] = 1;

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for lon > 360.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 8;
        }
    }
    printf("Test Case 9: Fatal error case where lat1 < -90.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[38] = 133;
        sec3[39] = 108;
        sec3[40] = 140;
        sec3[41] = 192;

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for lat < -90.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 9;
        }
    }
    printf("Test Case 10: Fatal error case where lat2 < -90.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[51] = 133;
        sec3[52] = 108;
        sec3[53] = 140;
        sec3[54] = 192;

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for lat < -90.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 10;
        }
    }
    printf("Test Case 11: Fatal error case where lat1 > 90.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[38] = 5;
        sec3[39] = 108;
        sec3[40] = 140;
        sec3[41] = 192;

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for lat > 90.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 11;
        }
    }
    printf("Test Case 12: Fatal error case where lat2 > 90.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[51] = 5;
        sec3[52] = 108;
        sec3[53] = 140;
        sec3[54] = 192;


        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for lat > 90.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 12;
        }
    }
    printf("Test Case 13: Fatal error due to inconsistent scan order.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[38] = 2;
        sec3[39] = 174;
        sec3[40] = 165;
        sec3[41] = 64;
        sec3[51] = 130;
        sec3[52] = 174;
        sec3[53] = 165;
        sec3[54] = 64;
        sec3[59] = 64;
        

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for inconsistent scan order.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 13;
        }
    }
    printf("Test Case 14: Fatal error due to ambiguous grid definition.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[59] = 16;

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for ambiguous grid definition.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 14;
        }
    }
    printf("Test Case 15: Fatal error due to grid definition decoding failure.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[6] = 0;
        sec3[7] = 0;
        sec3[8] = 0;
        sec3[9] = 1;
        sec3[13] = 10;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 32;
        sec3[59] = 16;

        if (setjmp(fatal_err) == 0) {
            mercator2ll(sec, &lat, &lon);
            printf("mercator2ll() failed to trigger fatal_error for grid definition decoding failure.\n");
            if (lat) free(lat);
            if (lon) free(lon);
            return 15;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}