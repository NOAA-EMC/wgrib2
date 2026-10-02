/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the polar2ll() routine from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"

#define TOL 1e-6

int polar2ll(unsigned char **sec, double **lat, double **lon);

int
main(){
    printf("Testing polar2ll()...\n");
    printf("Test Case 1: North polar stereographic grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[65] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 2;
        const double expected_lat[2] = {60.0, 90.0};
        const double expected_lon[2] = {0.0, 0.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 65;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 20;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 48;
        sec3[38] = 3;
        sec3[39] = 147;
        sec3[40] = 135;
        sec3[41] = 0;
        sec3[47] = 3;
        sec3[48] = 147;
        sec3[49] = 135;
        sec3[50] = 0;
        sec3[55] = 189;
        sec3[56] = 195;
        sec3[57] = 240;
        sec3[58] = 216;
        sec3[59] = 189;
        sec3[60] = 195;
        sec3[61] = 240;
        sec3[62] = 216;
        sec3[64] = 64;

        if (polar2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("polar2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 1;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("polar2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 1;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 2: South polar stereographic grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[65] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 2;
        const double expected_lat[2] = {-60.0, -90.0};
        const double expected_lon[2] = {0.0, 0.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 65;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 20;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[38] = 131;
        sec3[39] = 147;
        sec3[40] = 135;
        sec3[41] = 0;
        sec3[46] = 48;
        sec3[47] = 3;
        sec3[48] = 147;
        sec3[49] = 135;
        sec3[50] = 0;
        sec3[51] = 10;
        sec3[52] = 186;
        sec3[53] = 149;
        sec3[54] = 0;
        sec3[55] = 189;
        sec3[56] = 195;
        sec3[57] = 240;
        sec3[58] = 216;
        sec3[59] = 189;
        sec3[60] = 195;
        sec3[61] = 240;
        sec3[62] = 216;
        sec3[63] = 128;
        sec3[64] = 64;

        if (polar2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("polar2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 2;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("polar2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 2;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 3: North polar stereographic grid with negative y scanning.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[65] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 2;
        const double expected_lat[2] = {90.0, 60.0};
        const double expected_lon[2] = {0.0, 180.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 65;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 20;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[38] = 3;
        sec3[39] = 147;
        sec3[40] = 135;
        sec3[41] = 0;
        sec3[46] = 48;
        sec3[47] = 3;
        sec3[48] = 147;
        sec3[49] = 135;
        sec3[50] = 0;
        sec3[55] = 189;
        sec3[56] = 195;
        sec3[57] = 240;
        sec3[58] = 216;
        sec3[59] = 189;
        sec3[60] = 195;
        sec3[61] = 240;
        sec3[62] = 216;
        sec3[64] = 0;

        if (polar2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("polar2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 3;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("polar2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 3;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 4: North polar stereographic grid with negative x scanning.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[65] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 2;
        const double expected_lat[2] = {60.0, 90.0};
        const double expected_lon[2] = {0.0, 0.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 65;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 20;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[38] = 3;
        sec3[39] = 147;
        sec3[40] = 135;
        sec3[41] = 0;
        sec3[46] = 48;
        sec3[47] = 3;
        sec3[48] = 147;
        sec3[49] = 135;
        sec3[50] = 0;
        sec3[55] = 189;
        sec3[56] = 195;
        sec3[57] = 240;
        sec3[58] = 216;
        sec3[59] = 189;
        sec3[60] = 195;
        sec3[61] = 240;
        sec3[62] = 216;
        sec3[64] = 192;

        if (polar2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("polar2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 4;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("polar2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 4;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 5: Handling nnx < 1. Should return 1.\n");
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
        sec3[13] = 20;
        sec3[37] = 1;
        sec3[46] = 48;
        sec3[64] = 64;

        ret = polar2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("polar2ll() returned %d for nnx < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 5;
        }
    }
    printf("Test Case 6: Handling nny < 1. Should return 1.\n");
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
        sec3[13] = 20;
        sec3[33] = 1;
        sec3[46] = 48;
        sec3[64] = 64;

        ret = polar2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("polar2ll() returned %d for nny < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 6;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}

