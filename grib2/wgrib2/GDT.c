#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_G2CLIB

#include "grib2.h"
g2int g2_unpack3(unsigned char *,g2int *,g2int **,g2int **,
                        g2int *,g2int **,g2int *);

/*
 * Gdt
 *
 * routines to dump the Grid Definition Template
 *
 * 06/2012 Public Domain by Dusan Jovic
 *
 */

/*
 * HEADER:200:gdt:inv:0:contents of Grid Definition Template (g2c)
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
   }
   return 0;
}

#else
int f_gdt(ARG0) {
   fatal_error("GDT needs g2clib which was not installed","");
   return 1;
}
#endif
