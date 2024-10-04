#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 
 * Check_pdt_size.c     10/2024 Public Domain Wesley Ebisuzaki
 *
 * check_pdt_size(..) checks the size of the pdt
 *   sees whether the actual size of the pdt is the expected value
 *
 * This check can be enabled or disabled by -check_pdt 1/0  (enable/disable)
 *
 * in theory, the pdt can be bigger than expected with no ill consequences
 *  howerver, if the pdt is smaller than expectations, then any routine that
 *  uses the pdt could be reading outside of the pdt.
 */

/*
 * HEADER:100:check_pdt_size:misc:1:check pdt size X=1 enable/default, X=0 disable
 */

int check_pdt_size_flag = 1;
int warn_check_pdt = 1;

int f_check_pdt_size(ARG1) {
   check_pdt_size_flag = atoi(arg1);
   return 0;
}

int check_pdt_size(unsigned char **sec) {
    int calc_pdt_size, pdt_size;

    if (check_pdt_size_flag == 0) return 1;

    calc_pdt_size = pdt_len(sec, -1);
    pdt_size =  GB2_Sec4_size(sec);
    // fprintf(stderr, "pdt_size: %d %d\n",  pdt_size, calc_pdt_size);

    if (calc_pdt_size == -1) {
	fprintf(stderr,"check_pdt_size: pdt=%d needs to be added to pdt_len(..)\n", code_table_4_0(sec));
	return 1;
    }
    if (pdt_size == calc_pdt_size) return 1;
    if (warn_check_pdt++ < 4) fprintf(stderr,"*** check_pdt: pdt size %d expected %d ***\n",
	pdt_size, calc_pdt_size);
    return 0;
}
