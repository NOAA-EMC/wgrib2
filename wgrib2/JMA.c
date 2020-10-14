#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Public Domain 2015: Wesley Ebisuzaki
 */
extern char *nl;

/*
 * HEADER:400:JMA:inv:0:inventory for JMA locally defined PDT
 */
int f_JMA(ARG0) {
    int pdt, i, nr;

    int center; 
    if (mode < 0) return 0;
    center = GB2_Center(sec);
    if (center != JMA1 && center != JMA2) return 0;
    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt < 50000) return 0;
    if (pdt == 51022) {
	sprintf(inv_out,"site=%c%c%c%c", sec[4][24], sec[4][25], sec[4][26], sec[4][27]);
	inv_out += strlen(inv_out);
	if (mode >= 1) {
	    sprintf(inv_out,":site_id#=%u", uint2(sec[4]+28));
	    inv_out += strlen(inv_out);

	    sprintf(inv_out,":site_lon=%lf:site_lat=%lf:site_elev=%.1lfm",int4(sec[4]+18)*1e-6,
		int4(sec[4]+14)*1e-6,int2(sec[4]+22)*0.1) ;
	    inv_out += strlen(inv_out);

	    sprintf(inv_out,":mag_dec=%.2fdeg", int2(sec[4]+30)*0.001);
	    inv_out += strlen(inv_out);

	    sprintf(inv_out,":freq=%ukHz", uint4(sec[4]+32));
	    inv_out += strlen(inv_out);
	    sprintf(inv_out,":pol=%d:opn_mode=%d:reflec=%d", (int) sec[4][36], (int) sec[4][37], (int) sec[4][38]);
	    inv_out += strlen(inv_out);
	    sprintf(inv_out,":qc=%d:cluster_filter=%d", (int) sec[4][39], (int) sec[4][40]);
	    inv_out += strlen(inv_out);
	    sprintf(inv_out,":angle_elev_constant=%.2f", int2(sec[4]+41)*0.01);
	    inv_out += strlen(inv_out);
	}

	nr = GB2_Sec4_size(sec);
	nr = (nr % 4) ? -1 : (nr - 60) / 4;

	if (mode >= 1) {
	    sprintf(inv_out,":Ntheta=%d", nr);
	    inv_out += strlen(inv_out);

	    i = uint_n(sec[4]+55, 3);
	    if (i != 0xffffff) {
	        sprintf(inv_out,":bin_size=%dm", i);
	        inv_out += strlen(inv_out);
	    }
	    if ((sec[4][58] != 255) || (sec[4][59] != 255)) {
	        sprintf(inv_out,":d_theta=%.1lfdeg", int2(sec[4]+58)*0.1);
	        inv_out += strlen(inv_out);
	    }
	}


	if (mode < 2) {
	    sprintf(inv_out,":ele(1)=%.2fdeg", int2(sec[4]+60)*0.01);
	    inv_out += strlen(inv_out);
	    sprintf(inv_out,":ele(%d)=%.2fdeg", nr/2,int2(sec[4]+60+((nr-1)/2)*4)*0.01);
	    inv_out += strlen(inv_out);
	}
	if (mode >= 2) {
	    sprintf(inv_out,"\n:elevation angle(1..Ntheta)=");
	    inv_out += strlen(inv_out);
	    for (i = 0; i < nr; i++) {
		sprintf(inv_out,"%.2f ", int2(sec[4]+60+i*4)*0.01);
		inv_out += strlen(inv_out);
	    }
	    sprintf(inv_out,"\n:pulse repetion freq(1..Ntheta)=");
	    inv_out += strlen(inv_out);
	    for (i = 0; i < nr; i++) {
		sprintf(inv_out,"%.2f ", int2(sec[4]+62+i*4)*0.01);
		inv_out += strlen(inv_out);
	    }
        }
    }
    return 0;
}
