#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "grib.h"
#include "pds4.h"
#include "cnames.h"

/*
 * ensemble.c   v0.1 wesley ebisuzaki
 *
 * prints ensemble meta-data
 *
 * only for NCEP and ECMWF
 *
 * output format:
 *
 *       ECMWF
 *  ens=n/N:       n:  0=ctl, +/-ve
 *                 N:  total number of members
 *
 *       NCEP
 *  ens=n/type:    n:  0=ctl, +/-ve, CLUST, PROD/
 *                 type: Mn, WtdMn, SDev, NSDev
 *
 * updated 8/06 w. ebisuzaki
 */

extern int ncep_ens;

void ensemble(unsigned char *pds, int mode) {

    int pdslen;
    unsigned char ctmp;
    char char_end;

    pdslen = PDS_LEN(pds);
    char_end = mode == 2 ? ' ' : ':';

    if ((PDS_Center(pds) == NMC || ncep_ens) && pdslen >= 45 && pds[40] == 1) {

	/* control run */

	if (pds[41] == 1) {
	    if (mode != 2) {
		printf("ens%c0:%c", pds[42] == 1 ? '+' : '-', char_end);
	    }
	    else {
		printf("%s-res_ens_control ", pds[42] == 1 ? "hi" : "low");
	    }
	}

	/* perturbation run */

	else if (pds[41] == 2 || pds[41] == 3) {
	    if (mode != 2) {
	        printf("ens%c%d%c", pds[41] == 3 ? '+' : '-', pds[42],char_end);
	    }
	    else {
		printf("ens_perturbation=%c%d ",pds[41] == 3 ? '+' : '-', 
		    pds[42]);
	    }
	}

	/* cluster mean */

	else if (pds[41] == 4) {
	    if (mode != 2) printf("cluster%c", char_end);
	    else printf("cluster(%d members) ",pds[60]);
	}


	/* ensemble mean */

	else if (pds[41] == 5) {
	    if (mode != 2) printf("ensemble%c", char_end);
	    else printf("ensemble(%d members) ",pds[60]);
	}

	/* other case .. debug code */

	else {
		printf("ens %d/%d/%d/%d%c", pds[41],pds[42],pds[43],pds[44],char_end);
	}


	if (pdslen >= 44) {
	    if (pds[43] == 1 && pds[41] >= 4) printf("mean%c", char_end);
	    else if (pds[43] == 2) printf("weighted mean%c",char_end);
	    else if (pds[43] == 3) printf("no bias%c",char_end);
	    else if (pds[43] == 4) printf("weighted mean no bias%c",char_end);
	    else if (pds[43] == 5) printf("weight%c",char_end);
	    else if (pds[43] == 6) printf("climate percentile%c",char_end);
	    else if (pds[43] == 7) printf("daily climate mean%c",char_end);
	    else if (pds[43] == 8) printf("daily climate std dev%c",char_end);
	    else if (pds[43] == 11) printf("std dev%c",char_end);
	    else if (pds[43] == 12) printf("norm std dev%c",char_end);
	    else if (pds[43] == 21) printf("max val%c",char_end);
	    else if (pds[43] == 22) printf("min val%c",char_end);
	}

	/* NCEP probability limits */

	if ((PDS_PARAM(pds) == 191 || PDS_PARAM(pds) == 192) && pdslen >= 47) {
	    ctmp = PDS_PARAM(pds);
	    PDS_PARAM(pds) = pds[45];
	    if (pds[46] == 1 && pdslen >= 51) {
		printf("prob(%s<%f)%c", k5toa(pds), ibm2flt(pds+47),char_end);
	    }
	    else if (pds[46] == 2 && pdslen >= 54) {
		printf("prob(%s>%f)%c", k5toa(pds), ibm2flt(pds+51), char_end);
	    }
	    else if (pds[46] == 3 && pdslen >= 54) {
		printf("prob(%f<%s<%f)%c", ibm2flt(pds+47), k5toa(pds), 
			ibm2flt(pds+51), char_end);
	    }
            PDS_PARAM(pds) = ctmp;
	}
    }
}
