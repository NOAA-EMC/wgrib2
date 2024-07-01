#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 2022 Public Domain Wesley Ebisuzaki
 *
 * this file contains nice to know values for JMA
 */

/*
 * HEADER:-1:JMA_Nb:inv:0:value of JMA Nb
 */

int f_JMA_Nb(ARG0) {
    int i;

    if (mode >= 0) {
        i = JMA_Nb(sec);
        sprintf(inv_out,"JMA_Nb=%d", i);
    }
    return 0;
}

int JMA_Nb(unsigned char **sec) {
    int center, grid_template;

    if (sec == NULL) return -1;
    if (sec[3] == NULL) return -1;
    center = GB2_Center(sec);
    if (center != JMA1 && center != JMA2) return -1;
    grid_template = code_table_3_1(sec);
    if (grid_template == 50120 || grid_template == 50121) return (int) uint4(sec[3]+14);
    return -1;
}

/*
 * HEADER:-1:JMA_Nr:inv:0:value of JMA Nr
 */
int f_JMA_Nr(ARG0) {
    int i;

    if (mode >= 0) {
        i = JMA_Nr(sec);
        sprintf(inv_out,"JMA_Nr=%d", i);
    }
    return 0;
}

int JMA_Nr(unsigned char **sec) {
    int center, grid_template;

    if (sec == NULL) return -1;
    if (sec[3] == NULL) return -1;
    center = GB2_Center(sec);
    if (center != JMA1 && center != JMA2) return -1;
    grid_template = code_table_3_1(sec);
    if (grid_template == 50120 || grid_template == 50121) return (int) uint4(sec[3]+18);

    return -1;
}
