/**
 * This is a test for the wgrib2 project. This test is for grid_ident.c.
 * 
 * Alyson Stahl, 3/2026
 */

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include "grid_id.h"
#include "wgrib2.h"

#define ERR_TOL 1e-6
#define NUM_TEMPLATES 6

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing grid_id().\n");
    printf("Testing standard calls of grid_id() with different grid definition templates...\n");
    {
        /* All implemented grid definition templates + miscellaneous */
        int gdt_num[NUM_TEMPLATES] = {
            0, 1, 10, 20, 30, 4
        };

        /* Expected outputs for each template in gdt_num. */
        enum projection_type exp_proj_id[NUM_TEMPLATES] = {
            p_latlon, p_rotated_latlon, p_mercator, p_polar_stereographic, 
            p_lambert_conic, p_unknown
        };
        double exp_radius = 6367.47 * 1000.0;
        int exp_nx = 10;
        int exp_ny = 20;
        int exp_n = exp_nx * exp_ny;

        /* Sections 0, 1, and 3 (grid definition). */
        unsigned char sec0[16] = {0};
        unsigned char sec1[16] = {0};
        unsigned char sec3[80] = {0};
        unsigned char *sec[10] = {sec0, sec1, NULL, sec3};

        /* Minimal Section 1: length and section number. Center left as 0. */
        sec1[0] = 0;   /* length MSB */
        sec1[1] = 0;
        sec1[2] = 0;
        sec1[3] = 16;  /* section length = 16 bytes */
        sec1[4] = 1;   /* section number = 1 */

        /* Section 3 header. */
        sec3[0] = 0;
        sec3[1] = 0;
        sec3[2] = 0;
        sec3[3] = 80;   /* section length = 80 bytes */
        sec3[4] = 3;    /* section number = 3 */
        sec3[5] = 0;    /* source of grid definition (specified in template) */

        /* Number of data points (GB2_Sec3_npts) = nx * ny = 10 * 20 = 200. */
        sec3[6]  = 0;
        sec3[7]  = 0;
        sec3[8]  = 0;
        sec3[9]  = 200;

        /* No optional list of numbers. */
        sec3[10] = 0;   /* number of octets for optional list */
        sec3[11] = 0;   /* interpretation of list */

        /* Earth shape (Code Table 3.2) = 0: spherical Earth, radius 6367.47 km. */
        sec3[14] = 0;

        /* nx and ny at offsets 30 and 34 (big-endian 32-bit). */
        sec3[30] = 0;
        sec3[31] = 0;
        sec3[32] = 0;
        sec3[33] = 10;  /* nx = 10 */

        sec3[34] = 0;
        sec3[35] = 0;
        sec3[36] = 0;
        sec3[37] = 20;  /* ny = 20 */

        for (int i = 0; i < NUM_TEMPLATES; i++) {
            /* grid_id() outputs. */
            double r_major = 0.0;
            double r_minor = 0.0;
            enum projection_type proj_id = p_unknown;
            double proj_args[N_proj_args];
            struct grid_type grid_defn = {0};

            /* Set the grid definition template number in Section 3 at offsets 12 and 13. */
            sec3[12] = (unsigned char) (gdt_num[i] >> 8);
            sec3[13] = (unsigned char) (gdt_num[i] & 0xff);

            if ((grid_id(sec, &r_major, &r_minor, &proj_id,
                                proj_args, N_proj_args, &grid_defn) != 0)) {
                printf("grid_id() returned an error for template %d.\n", gdt_num[i]);
                return 10;
            }

            if (proj_id != exp_proj_id[i]) {
                printf("Unexpected projection type for template %d: got %d expected %d.\n", 
                        gdt_num[i], proj_id, exp_proj_id[i]);
                return 11;
            }

            if (fabs(r_major - exp_radius) > ERR_TOL || fabs(r_minor - exp_radius) > ERR_TOL) {
                printf("Unexpected Earth radii for template %d: r_major=%g r_minor=%g, expected=%g.\n",
                        gdt_num[i], r_major, r_minor, exp_radius);
                return 12;
            }

            /* Grid dimensions and total number of points must match Section 3. */
            if (grid_defn.nx != exp_nx || grid_defn.ny != exp_ny) {
                printf("Unexpected grid dimensions for template %d: nx=%d ny=%d, expected nx=%d ny=%d.\n",
                        gdt_num[i], grid_defn.nx, grid_defn.ny, exp_nx, exp_ny);
                return 13;
            }

            if (grid_defn.n != exp_n) {
                printf("Unexpected total grid points for template %d: n=%d, expected %d.\n", 
                        gdt_num[i], grid_defn.n, exp_n);
                return 14;
            }
        }

    }
    printf("Testing with undefined axes. Should throw a fatal_error()...\n");
    {
        unsigned char sec0[16] = {0};
        unsigned char sec1[16] = {0};
        unsigned char sec3[32] = {0};
        unsigned char *sec[10] = {sec0, sec1, NULL, sec3};

        /*
         * Set a grid definition template (gdt) whose Code Table 3.2 location
         * is undefined. For gdt = 50 and center != NCEP/JMA, code_table_3_2_location()
         * returns NULL.
         */
        sec3[12] = 0;   
        sec3[13] = 50; 

        if (setjmp(fatal_err) == 0) {
            double r_major, r_minor;
            enum projection_type proj_id;
            double proj_args[N_proj_args];
            struct grid_type grid_defn;

            grid_id(sec, &r_major, &r_minor, &proj_id,
                proj_args, N_proj_args, &grid_defn);

            printf("grid_id() did not trigger fatal_error() for undefined axes.\n");
            return 15;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}