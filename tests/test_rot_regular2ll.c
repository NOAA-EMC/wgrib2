/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the rot_regular2ll() routine from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"

#define TOL 1e-6

int rot_regular2ll(unsigned char **sec, double **lat, double **lon);

extern enum output_order_type output_order;

int
main(){
    printf("Testing rot_regular2ll()...\n");
    printf("Test Case 1: Typical case of rotated lat-lon grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[84] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {
            -22.316648447173367,
            -22.517632000000130,
            -22.316648447173375,
            -32.307631905681440,
            -32.517632000000106,
            -32.307631905681440
        };
        const double expected_lon[6] = {
            -169.347269731166648,
            180.000000000000000,
            169.347269731166676,
            -168.868348359614060,
            180.000000000000000,
            168.868348359614060
        };

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 84;
        sec3[4] = 3;
        sec3[9] = 6;
        sec3[13] = 1;
        sec3[33] = 3;
        sec3[37] = 2;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[50] = 20;
        sec3[51] = 220;
        sec3[52] = 147;
        sec3[53] = 128;
        sec3[54] = 48;
        sec3[55] = 1;
        sec3[56] = 49;
        sec3[57] = 45;
        sec3[59] = 0;
        sec3[60] = 152;
        sec3[61] = 150;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[67] = 0;
        sec3[68] = 152;
        sec3[69] = 150;
        sec3[70] = 128;
        sec3[71] = 64;
        sec3[72] = 250;
        sec3[73] = 162;
        sec3[74] = 176;
        sec3[75] = 128;
        sec3[83] = 0;

        output_order = wesn;


        if (rot_regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("rot_regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 1;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("rot_regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 1;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 2: Non-zero basic angle with rotated lat-lon grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[84] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {
            -22.316648447173367,
            -22.517632000000130,
            -22.316648447173375,
            -32.307631905681440,
            -32.517632000000106,
            -32.307631905681440
        };
        const double expected_lon[6] = {
            -169.347269731166648,
            180.000000000000000,
            169.347269731166676,
            -168.868348359614060,
            180.000000000000000,
            168.868348359614060
        };

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 84;
        sec3[4] = 3;
        sec3[9] = 6;
        sec3[13] = 1;
        sec3[41] = 1;
        sec3[43] = 15;
        sec3[44] = 66;
        sec3[45] = 64;
        sec3[33] = 3;
        sec3[37] = 2;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[50] = 20;
        sec3[51] = 220;
        sec3[52] = 147;
        sec3[53] = 128;
        sec3[54] = 48;
        sec3[55] = 1;
        sec3[56] = 49;
        sec3[57] = 45;
        sec3[59] = 0;
        sec3[60] = 152;
        sec3[61] = 150;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[67] = 0;
        sec3[68] = 152;
        sec3[69] = 150;
        sec3[70] = 128;
        sec3[71] = 64;
        sec3[72] = 250;
        sec3[73] = 162;
        sec3[74] = 176;
        sec3[75] = 128;
        sec3[83] = 0;

        output_order = wesn;

        if (rot_regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("rot_regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 2;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("rot_regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 2;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 3: Handling nny < 1. Should return 1.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[84] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        int ret;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 0;

        ret = rot_regular2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("rot_regular2ll() returned %d for nny < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 3;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}