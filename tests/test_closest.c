/**
 * This is a test for the wgrib2 project. 
 * 
 * This test is for the closest_init() and closest() routines from geo.c.
 * Alyson Stahl, 9/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"

#define TOL 1e-6

extern enum output_order_type output_order;
extern enum geolocation_type geolocation;
extern double *lat;
extern double *lon;
extern int nx;
extern int ny;
extern int scan;

int
main(){
    printf("Testing closest()...\n");
    printf("Test Case 1: Brute-force closest-point lookup on a small valid grid.\n");
    {
        unsigned char sec3[14] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[4] = {0.0, 0.0, 10.0, 10.0};
        const double grid_lon[4] = {0.0, 10.0, 0.0, 10.0};
        double plat = 9.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 2;

        sec[3] = sec3;
        sec3[3] = 14;
        sec3[4] = 3;
        sec3[9] = 4;

        lat = (double *) malloc(4 * sizeof(double));
        lon = (double *) malloc(4 * sizeof(double));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 1;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 0;
        nx = 0;
        ny = 0;
        output_order = raw;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 1;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 1;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 2: Closest-point lookup with GCTPC routines.\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[6] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0};
        const double grid_lon[6] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0};
        double plat = 19.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 4;

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

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 2;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 64;
        nx = 3;
        ny = 2;
        output_order = wesn;
        geolocation = gctpc;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 2;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 2;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 3: Closest-point lookup for latlon grid (Grid Type 0).\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[6] = {10.0, 10.0, 10.0, 20.0, 20.0, 20.0};
        const double grid_lon[6] = {350.0, 0.0, 10.0, 350.0, 0.0, 10.0};
        double plat = 19.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 3;

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

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 3;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 64;
        nx = 3;
        ny = 2;
        output_order = wesn;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 3;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 3;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 4: Closest-point lookup for Gaussian grid (Grid Type 40).\n");
    {
        unsigned char sec3[72] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[8] = {
            -35.26438968, -35.26438968, -35.26438968, -35.26438968,
             35.26438968,  35.26438968,  35.26438968,  35.26438968
        };
        const double grid_lon[8] = {
             0.0,  90.0, 180.0, 270.0,
             0.0,  90.0, 180.0, 270.0
        };
        double plat = 30.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 4;

        sec[3] = sec3;

        sec3[3] = 72;
        sec3[4] = 3;
        sec3[9] = 8;
        sec3[13] = 40;
        sec3[14] = 6;
        sec3[33] = 4;
        sec3[37] = 2;
        sec3[70] = 1;
        sec3[71] = 64;

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 4;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 64;
        nx = 4;
        ny = 2;
        output_order = wesn;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 4;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 4;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 5: Closest-point lookup for Space view grid (Grid Type 90).\n");
    {
        unsigned char sec1[16] = {0};
        unsigned char sec3[80] = {0};
        unsigned char *sec[8] = {NULL};
        double grid_lat[5] = {-90.0, -45.0, 0.0, 45.0, 90.0};
        double grid_lon[5] = {0.0, 90.0, 180.0, 270.0, 360.0};
        double plat = 0.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 2;

        sec[1] = sec1;
        sec[3] = sec3;

        sec1[3] = 16;
        sec1[4] = 1;

        sec3[3] = 80;
        sec3[4] = 3;
        sec3[9] = 5;
        sec3[13] = 90;
        sec3[14] = 6;
        sec3[33] = 5;
        sec3[37] = 1;
        sec3[46] = 0;
        sec3[50] = 232;
        sec3[54] = 232;
        sec3[63] = 64;
        sec3[69] = 100;
        sec3[70] = 181;
        sec3[71] = 64;

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 5;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 64;
        nx = 5;
        ny = 1;
        output_order = wesn;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 5;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 5;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 6: Brute-force closest-point lookup skips invalid latitude.\n");
    {
        unsigned char sec3[14] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[4] = {999.0, 0.0, 10.0, 10.0};
        const double grid_lon[4] = {0.0, 10.0, 0.0, 10.0};
        double plat = 9.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 2;

        sec[3] = sec3;
        sec3[3] = 14;
        sec3[4] = 3;
        sec3[9] = 4;

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 6;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 0;
        nx = 0;
        ny = 0;
        output_order = raw;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 6;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 6;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 7: Brute-force closest-point lookup skips invalid longitude.\n");
    {
        unsigned char sec3[14] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[4] = {0.0, 0.0, 10.0, 10.0};
        const double grid_lon[4] = {999.0, 10.0, 0.0, 10.0};
        double plat = 9.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = 2;

        sec[3] = sec3;
        sec3[3] = 14;
        sec3[4] = 3;
        sec3[9] = 4;

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 7;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 0;
        nx = 0;
        ny = 0;
        output_order = raw;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 7;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 7;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 8: Brute-force closest-point lookup fails. Should return -1.\n");
    {
        unsigned char sec3[14] = {0};
        unsigned char *sec[8] = {NULL};
        const double grid_lat[4] = {999.0, 999.0, 999.0, 999.0};
        const double grid_lon[4] = {0.0, 10.0, 20.0, 30.0};
        double plat = 9.0;
        double plon = 1.0;
        int ret;
        long int idx;
        long int exp_idx = -1;

        sec[3] = sec3;
        sec3[3] = 14;
        sec3[4] = 3;
        sec3[9] = 4;

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 8;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 0;
        nx = 0;
        ny = 0;
        output_order = raw;
        geolocation = internal;

        ret = closest_init(sec);
        if (ret != 0) {
            printf("closest_init() returned %d for valid input.\n", ret);
            free(lat);
            free(lon);
            return 8;
        }

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            free(lat);
            free(lon);
            return 8;
        }

        free(lat);
        free(lon);
    }
    printf("Test Case 9: Calling closest() without closest_init(). Should return -1.\n");
    {
        unsigned char sec3[14] = {0};
        unsigned char *sec[8] = {NULL};
        double grid_lat[1] = {0.0};
        double grid_lon[1] = {0.0};
        double plat = 0.0;
        double plon = 0.0;
        long int idx;
        long int exp_idx = -1;

        sec[3] = sec3;
        sec3[3] = 14;
        sec3[4] = 3;
        sec3[12] = 0;
        sec3[13] = 0;

        lat = (double *) malloc(sizeof(grid_lat));
        lon = (double *) malloc(sizeof(grid_lon));
        if (lat == NULL || lon == NULL) {
            printf("Failed to allocate test coordinate arrays.\n");
            free(lat);
            free(lon);
            return 9;
        }

        memcpy(lat, grid_lat, sizeof(grid_lat));
        memcpy(lon, grid_lon, sizeof(grid_lon));
        scan = 0;
        nx = 0;
        ny = 0;
        output_order = raw;
        geolocation = internal;

        idx = closest(sec, plat, plon);
        if (idx != exp_idx) {
            printf("closest() returned %ld, expected %ld.\n", idx, exp_idx);
            return 9;
        }
        
        free(lat);
        free(lon);
    }
    printf("SUCCESS!\n");
    return 0;
}