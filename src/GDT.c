/** @file
 * @brief Routines to dump the Grid Definition Template.
 * @author Public Domain: Dusan Jovic @date 06/2012
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_G2CLIB

#include <grib2.h>
g2int g2_unpack3(unsigned char *,g2int *,g2int **,g2int **,
                        g2int *,g2int **,g2int *);

/*
 * HEADER:200:gdt:inv:0:contents of Grid Definition Template (g2c)
 */

/**
 * Prints the contents of the Grid Definition Template (GDT).
 * 
 * Section 3 contains the grid definition template (GDT) and the -gdt option prints 
 * the parameters used to define the grid as used by g2clib/g2lib (NCEP libraries). 
 * This option is only useful finding the grid parameters for later use by g2clib/g2lib.
 *
 * The -gdt option will only work if g2clib is installed at compile time. 
 * 
 * ## Usage
 * -gdt
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * ## Example
 * ???
 * 
 * @author Dusan Jovic @date 06/2012
 */
int f_gdt(ARG0) {

    int n;
    unsigned char *p;

    g2int  *igdstmpl,*list_opt;
    g2int  *igds;
    g2int  iofst,igdtlen,num_opt,jerr;

    if (mode >= 0) {

        p = sec[3];

        if (p[4] != 3) {
            fatal_error("Sec3 was expected and not found", "");
        }

        igdstmpl=0;
        list_opt=0;
        igds=0;
        iofst=0;
        jerr = g2_unpack3(p,&iofst,&igds,&igdstmpl,&igdtlen,&list_opt,&num_opt);
        if (jerr == 0) {
            sprintf(inv_out,"GDT Number= %d GDT=",(int) igds[4]);
            inv_out += strlen(inv_out);
            for (n = 0; n < igdtlen; n++) {
                sprintf(inv_out," %d",(int) igdstmpl[n]);
                inv_out += strlen(inv_out);
            }
        }
        if (igds != NULL) free(igds);
        if (igdstmpl != NULL) free(igdstmpl);
        if (list_opt != NULL) free(list_opt);
    }
    return 0;
}

#else
int f_gdt(ARG0) {
    fatal_error("GDT needs g2clib which was not installed","");
    return 1;
}
#endif
