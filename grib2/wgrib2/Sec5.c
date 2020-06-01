#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 *  6/2016 Public Domain  Wesley Ebisuzaki
 *         support for AEC compression, DWD
 */

/*
 * HEADER:400:Sec5:inv:0:Sec 5 values (Data representation section)
 */
int f_Sec5(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,"Sec5 len=%u #defined data points=%u Data Repr. Template=5.%u",
          uint4(&(sec[5][0])), uint4(&(sec[5][5])), uint2(&(sec[5][9])));
    }
    return 0;
}

/*
 * HEADER:500:npts:inv:0:number of grid points
 */
int f_npts(ARG0) {
    if (mode >= 0) {
	sprintf(inv_out,"npts=%u", GB2_Sec3_npts(sec));
    }
    return 0;
}

/*
 * HEADER:510:packing:inv:0:shows the packing mode (use -v for more details)
 */

int f_packing(ARG0) {

    unsigned char *p;
    const char *string;
    int pack, i;
    float missing1, missing2;

    if (mode >= 0) {
	p = sec[5];
	pack = code_table_5_0(sec);

	if (mode >= 0) {
	    string = NULL;
	    switch(pack) {
#include "CodeTable_5.0.dat"
	    }
	    if (string == NULL) {
	        if (pack == 40000) string = "grid point data - JPEG2000";
	        if (pack == 40010) string = "grid point data - PNG";
	        if (pack == 255) string = "missing";
	    }
	    if (string == NULL) string="unknown packing";
	    sprintf(inv_out,"packing=%s", string);
	    inv_out += strlen(inv_out);

	    if (pack == 0) sprintf(inv_out,",s");
	    else if (pack == 2 || pack == 3) {
		if (pack == 2) sprintf(inv_out,",c1");
	        if (pack == 3) sprintf(inv_out,",c%d", code_table_5_6(sec)+1);
		if (GB2_Sec6_size(sec) > 6) {
		    inv_out += strlen(inv_out);
		    sprintf(inv_out,"b");
		}
	    }
	    else if (pack == 40) sprintf(inv_out,",j");
	    else if (pack == 42) sprintf(inv_out,",a");
	    else sprintf(inv_out,",_");
	    inv_out += strlen(inv_out);
	}
	if (mode > 0) {
            if (pack == 0 || pack == 1 || pack == 2 || pack == 3 || pack == 40 || pack == 40000 || 
		pack == 41 || pack == 50 || pack == 40010) {
		if (pack != 2 && pack != 3) {
                    sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d)", 
                    ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19]);
	            inv_out += strlen(inv_out);
		}
		if (pack == 2 || pack == 3) {
                    sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, ref=0..%d (#bits=%d) group width bits=%d", 
                    ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19],p[36]);
	            inv_out += strlen(inv_out);
		    sprintf(inv_out," #groups=%d", uint4(p+31));
	            inv_out += strlen(inv_out);
		    i = sub_missing_values(sec, &missing1, &missing2);
		    if (i > 0) {
		        sprintf(inv_out," missing1=%g", missing1);
	                inv_out += strlen(inv_out);
			if (i == 2) {
		            sprintf(inv_out," missing2=%g", missing2);
	                    inv_out += strlen(inv_out);
			}
		    }
		}
	    }
            else if (pack == 4) {
		sprintf(inv_out," precision code=%u", p[11]);
            }
	    else if (pack == 42) {
		sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d)",
			ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19]);
		inv_out += strlen(inv_out);
		sprintf(inv_out," compression options mask=%d samples/block=%d reference sample interval=%u",
			(int) p[21], (int) p[22], uint2(p+23) );
		inv_out += strlen(inv_out);
	    }
	    else if (pack == 51) {
                sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d)", 
                ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19]);
	        inv_out += strlen(inv_out);
		sprintf(inv_out," P-Laplacian scaling factor*10^-6=%d",int4(p+20));
	        inv_out += strlen(inv_out);
		sprintf(inv_out," Js=%u Ks=%u Ms=%u Ts=%d", uint2(p+24), uint2(p+26), uint2(p+28), 
			int4(p+30));
	        inv_out += strlen(inv_out);
		sprintf(inv_out," code_table_5.7=%d", (int) p[34]);
/*		sprintf(inv_out," mean?=%lg", ieee2flt(sec[7]+5)); */
	    }
	    else if (pack == 61) {
                sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d) pre-processing parameter=%lg", 
                ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19], ieee2flt(p+20));
	    }
        }
    }
    return 0;
}
