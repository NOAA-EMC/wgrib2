/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the regular2ll() routine from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include <setjmp.h>

#define TOL 1e-6

int regular2ll(unsigned char **sec, double **lat, double **lon);

extern enum output_order_type output_order;
extern jmp_buf fatal_err;

int
main(){

    printf("Testing regular2ll()...\n");
    printf("Test Case 1: Typical case of regular lat-lon grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0};
        const double expected_lon[6] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 6;
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

        output_order = wesn;

        if (regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 1;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 1;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 2: Thinned quasi-regular grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[74] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 5;
        const double expected_lat[5] = {10.0, 10.0, 20.0, 20.0, 20.0};
        const double expected_lon[5] = {350.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 74;
        sec3[4] = 3;
        sec3[9] = 5;
        sec3[10] = 1;
        sec3[30] = 255;
        sec3[31] = 255;
        sec3[32] = 255;
        sec3[33] = 255;
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
        sec3[72] = 2;
        sec3[73] = 3;

        output_order = wesn;

        if (regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 2;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 2;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 3: Grid Template 0 with negative y scanning order.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0};
        const double expected_lon[6] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 6;
        sec3[33] = 3;
        sec3[37] = 2;
        sec3[46] = 1;
        sec3[47] = 49;
        sec3[48] = 45;
        sec3[50] = 20;
        sec3[51] = 220;
        sec3[52] = 147;
        sec3[53] = 128;
        sec3[54] = 48;
        sec3[55] = 0;
        sec3[56] = 152;
        sec3[57] = 150;
        sec3[58] = 128;
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
        sec3[71] = 0;

        output_order = wesn;
      
        if (regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 3;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 3;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 4: Positive x scan with row-reversed rows and implied longitude increment.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 9;
        const double expected_lat[9] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0, 30.0, 30.0, 30.0};
        const double expected_lon[9] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 9;
        sec3[33] = 3;
        sec3[37] = 3;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[50] = 20;
        sec3[51] = 220;
        sec3[52] = 147;
        sec3[53] = 128;
        sec3[54] = 16;
        sec3[55] = 1;
        sec3[56] = 201;
        sec3[57] = 195;
        sec3[58] = 128;
        sec3[59] = 0;
        sec3[60] = 76;
        sec3[61] = 75;
        sec3[62] = 64;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[67] = 0;
        sec3[68] = 152;
        sec3[69] = 150;
        sec3[70] = 128;
        sec3[71] = 80;

        output_order = wesn;

        if (regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 4;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 4;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 5: Negative x scan with row-reversed rows and implied longitude increment.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 9;
        const double expected_lat[9] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0, 30.0, 30.0, 30.0};
        const double expected_lon[9] = {0.0, 10.0, 20.0, 0.0, 10.0, 20.0, 0.0, 10.0, 20.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 9;
        sec3[33] = 3;
        sec3[37] = 3;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[50] = 1;
        sec3[51] = 49;
        sec3[52] = 45;
        sec3[53] = 0;
        sec3[54] = 16;
        sec3[55] = 1;
        sec3[56] = 201;
        sec3[57] = 195;
        sec3[58] = 128;
        sec3[59] = 2;
        sec3[60] = 98;
        sec3[61] = 90;
        sec3[62] = 0;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[67] = 0;
        sec3[68] = 152;
        sec3[69] = 150;
        sec3[70] = 128;
        sec3[71] = 208;

        output_order = wesn;

        if (regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 5;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 5;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 6: Non-zero basic angle with negative w normalization.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *lat = NULL;
        double *lon = NULL;
        unsigned int exp_size = 9;
        const double expected_lat[9] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0, 30.0, 30.0, 30.0};
        const double expected_lon[9] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 9;
        sec3[33] = 3;
        sec3[37] = 3;
        sec3[41] = 1;
        sec3[43] = 15;
        sec3[44] = 66;
        sec3[45] = 64;
        sec3[46] = 0;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[50] = 0;
        sec3[51] = 152;
        sec3[52] = 150;
        sec3[53] = 128;
        sec3[54] = 0;
        sec3[55] = 1;
        sec3[56] = 201;
        sec3[57] = 195;
        sec3[58] = 128;
        sec3[59] = 1;
        sec3[60] = 201;
        sec3[61] = 195;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[67] = 0;
        sec3[68] = 152;
        sec3[69] = 150;
        sec3[70] = 128;
        sec3[71] = 208;

        output_order = wesn;

        if (regular2ll(sec, &lat, &lon) != 0 || lat == NULL || lon == NULL) {
            printf("regular2ll() failed on valid input.\n");
            free(lat);
            free(lon);
            return 6;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(lat[i] - expected_lat[i]) > TOL ||
                fabs(lon[i] - expected_lon[i]) > TOL) {
                printf("regular2ll() produced an unexpected coordinate at index %u.\n", i);
                free(lat);
                free(lon);
                return 6;
            }
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 7: Handling nny < 1. Should return 1.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
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

        ret = regular2ll(sec, &lat, &lon);
        if (ret != 1) {
            printf("regular2ll() returned %d for nny < 1. Expected 1.\n", ret);
            if (lat) free(lat);
            if (lon) free(lon);
            return 7;
        }

    }
    printf("Test Case 8: Fatal error case where lon1 < 0.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 255;
        sec3[42] = 255;
        sec3[43] = 255;
        sec3[44] = 255;
        sec3[46] = 0;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[50] = 0;
        sec3[51] = 15;
        sec3[52] = 66;
        sec3[53] = 64;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lon < 0.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 8;
        }
    }
    printf("Test Case 9: Fatal error case where lon2 < 0.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 255;
        sec3[42] = 255;
        sec3[43] = 255;
        sec3[44] = 255;
        sec3[59] = 0;
        sec3[60] = 0;
        sec3[61] = 0;
        sec3[62] = 1;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lon < 0.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 9;
        }
    }
    printf("Test Case 10: Fatal error case where lon1 >= 360.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 0;
        sec3[44] = 1;
        sec3[50] = 21;
        sec3[51] = 117;
        sec3[52] = 42;
        sec3[53] = 1;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lon >= 360.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 10;
        }
    }
    printf("Test Case 11: Fatal error case where lon2 >= 360.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 0;
        sec3[44] = 1;
        sec3[59] = 21;
        sec3[60] = 117;
        sec3[61] = 42;
        sec3[62] = 1;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lon >= 360.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 11;
        }
    }
    printf("Test Case 12: Fatal error case where lat1 < -90.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 0;
        sec3[44] = 1;
        sec3[46] = 250;
        sec3[47] = 162;
        sec3[48] = 181;
        sec3[49] = 127;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lat < -90.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 12;
        }
    }
    printf("Test Case 13: Fatal error case where lat2 < -90.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 0;
        sec3[44] = 1;
        sec3[55] = 250;
        sec3[56] = 162;
        sec3[57] = 181;
        sec3[58] = 127;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lat < -90.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 13;
        }
    }
    printf("Test Case 14: Fatal error case where lat1 > 90.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 0;
        sec3[44] = 1;
        sec3[46] = 5;
        sec3[47] = 93;
        sec3[48] = 74;
        sec3[49] = 129;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lat > 90.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 14;
        }
    }
    printf("Test Case 15: Fatal error case where lat2 > 90.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[41] = 0;
        sec3[44] = 1;
        sec3[55] = 5;
        sec3[56] = 93;
        sec3[57] = 74;
        sec3[58] = 129;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for lat > 90.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 15;
        }
    }
    printf("Test Case 16: Fatal error due to inconsistent scan order.\n");
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
        sec3[9] = 2;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 1;
        sec3[47] = 49;
        sec3[48] = 45;
        sec3[49] = 0;
        sec3[55] = 0;
        sec3[56] = 152;
        sec3[57] = 150;
        sec3[58] = 128;
        sec3[71] = 64;

        output_order = wesn;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for inconsistent scan order.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 16;
        }
    }
    printf("Test Case 17: Fatal error due to inconsistent dlat.\n");
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
        sec3[9] = 2;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 0;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[54] = 16;
        sec3[55] = 1;
        sec3[56] = 49;
        sec3[57] = 45;
        sec3[58] = 0;
        sec3[67] = 0;
        sec3[68] = 76;
        sec3[69] = 75;
        sec3[70] = 64;
        sec3[71] = 64;

        output_order = wesn;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for inconsistent dlat.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 17;
        }
    }
    printf("Test Case 18: Fatal error due to ambiguous grid definition.\n");
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
        sec3[9] = 2;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 0;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[54] = 16;
        sec3[55] = 1;
        sec3[56] = 49;
        sec3[57] = 45;
        sec3[58] = 0;
        sec3[67] = 0;
        sec3[68] = 152;
        sec3[69] = 150;
        sec3[70] = 128;
        sec3[71] = 80;

        output_order = wesn;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for ambiguous grid definition.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 18;
        }
    }
    printf("Test Case 19: Fatal error due to stagger problem.\n");
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
        sec3[9] = 1;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[55] = 0;
        sec3[56] = 152;
        sec3[57] = 150;
        sec3[58] = 128;
        sec3[71] = 64;

        output_order = raw;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for stagger problem.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 19;
        }
    }
    printf("Test Case 20: Fatal error due to inconsistent dlon.\n");
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
        sec3[9] = 2;
        sec3[33] = 2;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[47] = 152;
        sec3[48] = 150;
        sec3[49] = 128;
        sec3[54] = 32;
        sec3[55] = 0;
        sec3[56] = 152;
        sec3[57] = 150;
        sec3[58] = 128;
        sec3[59] = 0;
        sec3[60] = 152;
        sec3[61] = 150;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 76;
        sec3[65] = 75;
        sec3[66] = 64;
        sec3[71] = 64;

        output_order = wesn;

        if (setjmp(fatal_err) == 0) {
            regular2ll(sec, &lat, &lon);
            printf("regular2ll() failed to trigger fatal_error for inconsistent dlon.\n");
            if (lat) free(lat);
            if (lon) free(lon);         
            return 20;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}

