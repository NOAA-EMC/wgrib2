/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the gauss2ll() routine from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include <setjmp.h>

#define TOL 1e-6

int gauss2ll(unsigned char **sec, double **llat, double **llon);

extern jmp_buf fatal_err;

int
main(){
    printf("Testing gauss2ll()...\n");
    printf("Test Case 1: Regular Gaussian lat-lon grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {
            19.875719147440904, 19.875719147440904, 19.875719147440904,
            59.444408289166775, 59.444408289166775, 59.444408289166775
        };
        const double expected_lon[6] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 6;
        sec3[13] = 40;
        sec3[33] = 3;
        sec3[37] = 2;
        sec3[46] = 1;
        sec3[47] = 47;
        sec3[48] = 71;
        sec3[49] = 135;
        sec3[50] = 20;
        sec3[51] = 220;
        sec3[52] = 147;
        sec3[53] = 128;
        sec3[55] = 3;
        sec3[56] = 139;
        sec3[57] = 12;
        sec3[58] = 184;
        sec3[59] = 0;
        sec3[60] = 152;
        sec3[61] = 150;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[70] = 2;
        sec3[71] = 64;

        if (gauss2ll(sec, &llat, &llon) != 0 || llat == NULL || llon == NULL) {
            printf("gauss2ll() failed on valid input.\n");
            free(llat);
            free(llon);
            return 1;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(llat[i] - expected_lat[i]) > TOL ||
                fabs(llon[i] - expected_lon[i]) > TOL) {
                printf("gauss2ll() produced an unexpected coordinate at index %u.\n", i);
                free(llat);
                free(llon);
                return 1;
            }
        }

        free(llat);
        free(llon);
    }
    printf("Test Case 2: Quasi-regular Gaussian lat-lon grid.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[74] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;
        unsigned int exp_size = 5;
        const double expected_lat[5] = {
            19.875719147440904, 19.875719147440904,
            59.444408289166775, 59.444408289166775, 59.444408289166775
        };
        const double expected_lon[5] = {350.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 74;
        sec3[4] = 3;
        sec3[9] = 5;
        sec3[10] = 1;
        sec3[13] = 40;
        sec3[30] = 255;
        sec3[31] = 255;
        sec3[32] = 255;
        sec3[33] = 255;
        sec3[37] = 2;
        sec3[46] = 1;
        sec3[47] = 47;
        sec3[48] = 71;
        sec3[49] = 135;
        sec3[50] = 20;
        sec3[51] = 220;
        sec3[52] = 147;
        sec3[53] = 128;
        sec3[55] = 3;
        sec3[56] = 139;
        sec3[57] = 12;
        sec3[58] = 184;
        sec3[59] = 0;
        sec3[60] = 152;
        sec3[61] = 150;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[70] = 2;
        sec3[71] = 64;
        sec3[72] = 2;
        sec3[73] = 3;

        if (gauss2ll(sec, &llat, &llon) != 0 || llat == NULL || llon == NULL) {
            printf("gauss2ll() failed on valid input.\n");
            free(llat);
            free(llon);
            return 2;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(llat[i] - expected_lat[i]) > TOL ||
                fabs(llon[i] - expected_lon[i]) > TOL) {
                printf("gauss2ll() produced an unexpected coordinate at index %u.\n", i);
                free(llat);
                free(llon);
                return 2;
            }
        }

        free(llat);
        free(llon);
    }
    printf("Test Case 3: Regular Gaussian lat-lon grid with negative x and y scanning order.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {
            19.875719147440904, 19.875719147440904, 19.875719147440904,
            59.444408289166775, 59.444408289166775, 59.444408289166775
        };
        const double expected_lon[6] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 6;
        sec3[13] = 40;
        sec3[33] = 3;
        sec3[37] = 2;
        sec3[46] = 3;
        sec3[47] = 139;
        sec3[48] = 12;
        sec3[49] = 184;
        sec3[50] = 0;
        sec3[51] = 152;
        sec3[52] = 150;
        sec3[53] = 128;
        sec3[55] = 1;
        sec3[56] = 47;
        sec3[57] = 71;
        sec3[58] = 135;
        sec3[59] = 20;
        sec3[60] = 220;
        sec3[61] = 147;
        sec3[62] = 128;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[70] = 2;
        sec3[71] = 128;

        if (gauss2ll(sec, &llat, &llon) != 0 || llat == NULL || llon == NULL) {
            printf("gauss2ll() failed on valid input.\n");
            free(llat);
            free(llon);
            return 3;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(llat[i] - expected_lat[i]) > TOL ||
                fabs(llon[i] - expected_lon[i]) > TOL) {
                printf("gauss2ll() produced an unexpected coordinate at index %u.\n", i);
                free(llat);
                free(llon);
                return 3;
            }
        }

        free(llat);
        free(llon);
    }
    printf("Test Case 4: Regular Gaussian lat-lon grid with e normalized from 360 degrees.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;
        unsigned int exp_size = 6;
        const double expected_lat[6] = {
            19.875719147440904, 19.875719147440904, 19.875719147440904,
            59.444408289166775, 59.444408289166775, 59.444408289166775
        };
        const double expected_lon[6] = {0.0, 10.0, 20.0, 0.0, 10.0, 20.0};

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 6;
        sec3[13] = 40;
        sec3[33] = 3;
        sec3[37] = 2;
        sec3[46] = 1;
        sec3[47] = 47;
        sec3[48] = 71;
        sec3[49] = 135;
        sec3[50] = 21;
        sec3[51] = 117;
        sec3[52] = 42;
        sec3[53] = 0;
        sec3[55] = 3;
        sec3[56] = 139;
        sec3[57] = 12;
        sec3[58] = 184;
        sec3[59] = 1;
        sec3[60] = 49;
        sec3[61] = 45;
        sec3[62] = 0;
        sec3[63] = 0;
        sec3[64] = 152;
        sec3[65] = 150;
        sec3[66] = 128;
        sec3[70] = 2;
        sec3[71] = 64;

        if (gauss2ll(sec, &llat, &llon) != 0 || llat == NULL || llon == NULL) {
            printf("gauss2ll() failed on valid input.\n");
            free(llat);
            free(llon);
            return 4;
        }

        for (unsigned int i = 0; i < exp_size; i++) {
            if (fabs(llat[i] - expected_lat[i]) > TOL ||
                fabs(llon[i] - expected_lon[i]) > TOL) {
                printf("gauss2ll() produced an unexpected coordinate at index %u.\n", i);
                free(llat);
                free(llon);
                return 4;
            }
        }

        free(llat);
        free(llon);
    }
    printf("Test Case 5: Fatal error due to lon1 > 360.0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[49] = 1;
        sec3[50] = 21;
        sec3[51] = 117;
        sec3[52] = 42;
        sec3[53] = 1;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for lon > 360.0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 5;
        }
    }
    printf("Test Case 6: Fatal error due to lon2 > 360.0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[49] = 1;
        sec3[59] = 21;
        sec3[60] = 117;
        sec3[61] = 42;
        sec3[62] = 1;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for lon > 360.0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 6;
        }
    }
    printf("Test Case 7: Fatal error due to lat1 < -90.0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 250;
        sec3[47] = 162;
        sec3[48] = 181;
        sec3[49] = 127;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for lat < -90.0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 7;
        }
    }
    printf("Test Case 8: Fatal error due to lat2 < -90.0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[49] = 1;
        sec3[55] = 250;
        sec3[56] = 162;
        sec3[57] = 181;
        sec3[58] = 127;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for lat < -90.0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 8;
        }
    }
    printf("Test Case 9: Fatal error due to lat1 > 90.0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 5;
        sec3[47] = 93;
        sec3[48] = 74;
        sec3[49] = 129;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for lat > 90.0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 9;
        }
    }
    printf("Test Case 10: Fatal error due to lat2 > 90.0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[49] = 1;
        sec3[55] = 5;
        sec3[56] = 93;
        sec3[57] = 74;
        sec3[58] = 129;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for lat > 90.0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 10;
        }
    }
    printf("Test Case 11: Fatal error due to nny == 0. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[46] = 0;
        sec3[49] = 1;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for nny == 0.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 11;
        }
    }
    printf("Test Case 12: Fatal error due to inconsistent scan order. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 0;
        sec3[49] = 10;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[58] = 20;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for inconsistent scan order.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 12;
        }
    }
    printf("Test Case 13: Fatal error due to bad grid definition (nny > nlat). \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 3;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 3;
        sec3[46] = 0;
        sec3[49] = 10;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[58] = 20;
        sec3[70] = 1;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for bad grid definition (nny > nlat).\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 13;
        }
    }
    printf("Test Case 14: Fatal error due to non-Gaussian latitude (isouth < 0). \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 0;
        sec3[49] = 10;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[55] = 3;
        sec3[56] = 139;
        sec3[57] = 12;
        sec3[58] = 184;
        sec3[70] = 2;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for non-Gaussian latitude.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 14;
        }
    }
    printf("Test Case 15: Fatal error due to non-Gaussian latitude (inorth < 0). \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 2;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 2;
        sec3[46] = 1;
        sec3[47] = 47;
        sec3[48] = 71;
        sec3[49] = 135;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[55] = 0;
        sec3[58] = 30;
        sec3[70] = 2;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for non-Gaussian latitude.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 15;
        }
    }
    printf("Test Case 16: Fatal error due to latitude not consistent with ny. \n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        double *llat = NULL;
        double *llon = NULL;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 1;
        sec3[13] = 40;
        sec3[33] = 1;
        sec3[37] = 1;
        sec3[46] = 1;
        sec3[47] = 47;
        sec3[48] = 71;
        sec3[49] = 135;
        sec3[50] = 0;
        sec3[53] = 1;
        sec3[55] = 3;
        sec3[56] = 139;
        sec3[57] = 12;
        sec3[58] = 184;
        sec3[70] = 2;
        sec3[71] = 64;

        if (setjmp(fatal_err) == 0) {
            gauss2ll(sec, &llat, &llon);
            printf("gauss2ll() failed to trigger fatal_error for non-Gaussian latitude.\n");
            if (llat) free(llat);
            if (llon) free(llon);
            return 16;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}

