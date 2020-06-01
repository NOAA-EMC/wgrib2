#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * WxText.c
 *
 * 6/2011 Public Domain Wesley Ebisuzaki
 *
 * NDFD uses section 2 (local) to keep weather information keys
 *   these keys are text strings with weather info.
 *
 * mk_WxKeys decodes and makes a copy of the keys
 *
 * v1.1 modification for MDL
 */


char *WxTable, **WxKeys;
int WxNum;

const char *WxLabel(float f) {
    int j;
    if (UNDEFINED_VAL(f)) return "Undefined";
    j = (int) floor(f + 0.5);
    if (f < 0 || WxNum == 0 || j >= WxNum) fatal_error("WxLabel: program error","");
    return WxKeys[j];
}

int mk_WxKeys(unsigned char **sec) {

    int template, n_bits, ok;
    unsigned int i, j, n;
    double ref_val, dec_scale, bin_scale;
    float *dat;

    if (WxKeys) { free(WxKeys); WxKeys = NULL; }
    if (WxTable) { free(WxTable); WxTable = NULL; }
    WxNum = 0;

    /* must have local section  */
    if (GB2_Sec2_size(sec) == 0) return 0;

    /* PWTHER "Predominant Weather" uses the extension */

    ok = 0;
    if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 1
                && (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 226)) ok = 1;
    /* NDFD uses the extension */
    if (GB2_Center(sec) == 8) ok = 1;

    if (ok == 0) return 0;

    template = int2(sec[2]+6);
    if (template != 1) return 0;
    n = uint4(sec[2]+8);
    ref_val = ieee2flt(sec[2]+12);
    dec_scale = Int_Power(10.0, -int2(sec[2]+14));
    bin_scale = Int_Power(2.0, int2(sec[2]+16));

    n_bits = (int) sec[2][18];

    dat = (float *) malloc(sizeof(float) * (size_t) n);
    WxTable = (char *) malloc(sizeof(char) * ((size_t) n + 1));
    if (dat == NULL || WxTable == NULL) fatal_error("mk_WxKeys: memory allocation","");

    unpk_0(dat, sec[2] + 20, NULL, n_bits, n, ref_val, bin_scale, dec_scale);

    for (j = i = 0; i < n; i++) {
        WxTable[i] = (int) dat[i];
        if (WxTable[i] == 0) j++;
    }
    free(dat);
    WxTable[n] = 0;
    j++;
    WxKeys = (char **) malloc(sizeof(char *) * (size_t) j);
    if (WxKeys == NULL) fatal_error("mk_WxKeys: memory allocation","");

    WxKeys[0] = WxTable;
    j = 1;
    for (i = 0; i < n; i++) {
        if (WxTable[i] == 0) WxKeys[j++] = WxTable+i+1;
    }

    if (WxTable[n-1] == 0) j--;
    WxNum = j;
//    print out table
//    for (i = 0; i < WxNum; i++) {
//       fprintf(stderr, "%d %s\n", i, WxKeys[i]);
//    }
    return 0;
}
