#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Public Domain 2009: Wesley Ebisuzaki
 * Public Domain 2009: Sam Trahan
 */
extern char *nl;

/*
 * HEADER:-1:spectral_bands_extname:inv:0:spectral bands for satellite, pdt=4.31 or 4.32, concise name for ExtName
 */
int f_spectral_bands_extname(ARG0) {
    int pdtsize, pdt, nb, i, ipol;
    int code1, code2, instrument, scale_factor, scaled_val, bandstart;
    double value;
    const char *agency=NULL, *instype=NULL, *shortname=NULL, *longname=NULL;
    const char *satellite=NULL, *pol=NULL;
    const char *classification=NULL;
    
    const char *shortname1=NULL, *satellite1=NULL, *pol1=NULL;
    double c=299792458.;
    double minwave=9e19, maxwave=-9e19, sumwave=0, wl, fq;

    int multisat=0, multipol=0;

    if (mode < 0) return 0;

    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt == 31) {
        nb = sec[4][13];
        bandstart=14;
    } else if(pdt == 32) {
        nb = sec[4][22];
        bandstart=23;
    } else 
        return 0;

    // check size

    pdtsize =  prod_def_temp_size(sec);
    i = bandstart + 11*nb;
    if (pdtsize != i)
        fatal_error_i("spectral bands: section 4 is wrong size %d",pdtsize);

    // print out spectral info

    code1 = code2 = -1;
    instrument = value = 1;
    for (i = 0; i < nb; i++) {
        code1 = uint2(sec[4]+bandstart+11*i);
        code2 = uint2(sec[4]+bandstart+2+11*i);
        instrument = uint2(sec[4]+bandstart+4+11*i);
        scale_factor = int1(sec[4]+bandstart+6+11*i);
        scaled_val = uint4(sec[4]+bandstart+7+11*i);
        value = scaled2flt(scale_factor, scaled_val);
        if(value>maxwave) maxwave=value;
        if(value<minwave) minwave=value;
        sumwave+=value;

        classification=NULL;
        agency=NULL;
        instype=NULL;
        shortname=NULL;
        longname=NULL;
        satellite=NULL;
        switch(code1&511) {
#include "BUFRTable_0_02_020.dat"
        }
        switch(code2) {
#include "BUFRTable_0_01_007.dat"
        }
        switch(instrument&2047) {
#include "BUFRTable_0_02_019.dat"
        }

        if(!satellite1||!satellite1[0]) satellite1=satellite;
        if(!shortname1||!shortname1[0]) shortname1=shortname;
        if(instype) {
            ipol=instrument >> 13;
            pol=NULL;
            switch(ipol) {
            case 1: pol="unpolarized"; break;
            case 2: pol="H pol."; break;
            case 3: pol="V pol."; break;
            case 4: pol="R. circ. pol."; break;
            case 5: pol="L. circ. pol."; break;
            default: pol=NULL; break;
            }
        }
        if(!pol1||!pol1[0]) pol1=pol;

        if((satellite && satellite[0] && strcmp(satellite,satellite1)) ||
           (shortname && shortname[0] && strcmp(shortname,shortname1))) {
            multisat=1;
        }
        if(pol && pol[0] && strcmp(pol,pol1)) {
            multipol=1;
        }
    }

    if(multisat) {
        sprintf(inv_out,"Multi-sat ");
        inv_out+=strlen(inv_out);
    } else {
        if(satellite1)
            sprintf(inv_out,"%s ",satellite1);
        else
            sprintf(inv_out,"Sat %d %d ",code1,code2);
        inv_out+=strlen(inv_out);
        if(shortname1)
            sprintf(inv_out,"%s ",shortname1);
        else
            sprintf(inv_out,"Ins %d ",instrument);
        inv_out+=strlen(inv_out);
    }
        
    if(minwave*1.01<maxwave)
        sprintf(inv_out,"%d bands: %.3g to %.3g m-1 ",nb,minwave,maxwave);
    else {
        wl=1.e6/value;
        fq=c*value/1e9;
        if     (wl>0.1  && wl<100 )  sprintf(inv_out,"%.2f um ",wl);
        else if(fq>1    && fq<1000)  sprintf(inv_out,"%.2f GHz ",fq);
        else                         sprintf(inv_out,"%.3g m-1 ",minwave);

    }
    inv_out+=strlen(inv_out);

    if(multipol) 
        strcat(inv_out,"mult.pol.");
    else if(pol1 && strcmp(pol1,"unpolarized"))
        strcat(inv_out,pol);

    return 0;
}



/*
 * HEADER:400:spectral_bands:inv:0:spectral bands for satellite, pdt=4.31 or 4.32
 */
int f_spectral_bands(ARG0) {
    int pdtsize, pdt, nb, i, ipol;
    int code1, code2, instrument, scale_factor, scaled_val, bandstart;
    double value,c=299792458.,h=6.626070040e-34,J2eV=6.242e+18;
    const char *agency, *instype, *shortname, *longname, *satellite, *pol;
    const char *classification, *source;
    if (mode >= 0) {
        pdt = GB2_ProdDefTemplateNo(sec);
	if (pdt == 31) {
          nb = sec[4][13];
          bandstart=14;
        } else if(pdt == 32) {
          nb = sec[4][22];
          bandstart=23;
        } else 
          return 0;

	sprintf(inv_out,"%snumber of spectral bands=%d", nl, nb);
	inv_out += strlen(inv_out);

	// check size

	pdtsize =  prod_def_temp_size(sec);
	i = bandstart + 11*nb;
	if (pdtsize != i) fatal_error_i("spectral bands: section 4 is wrong size %d",pdtsize);

	// print out spectral info
	for (i = 0; i < nb; i++) {
	    code1 = uint2(sec[4]+bandstart+11*i);
	    code2 = uint2(sec[4]+bandstart+2+11*i);
	    instrument = uint2(sec[4]+bandstart+4+11*i);
	    scale_factor = int1(sec[4]+bandstart+6+11*i);
	    scaled_val = uint4(sec[4]+bandstart+7+11*i);
	    value = scaled2flt(scale_factor, scaled_val);

            classification=NULL;
            agency=NULL;
            instype=NULL;
            shortname=NULL;
            longname=NULL;
            satellite=NULL;
            switch(code1&511) {
#include "BUFRTable_0_02_020.dat"
            }
            switch(code2) {
#include "BUFRTable_0_01_007.dat"
            }
            switch(instrument&2047) {
#include "BUFRTable_0_02_019.dat"
            }

            if(instype)
                sprintf(inv_out,"%sband %d %d instrument %d central wave no %.3lf (m-1)",
                        nl, code1, code2, instrument&2047, value);
            else
                sprintf(inv_out,"%sband %d %d instrument %d central wave no %.3lf (m-1)",
                        nl, code1, code2, instrument, value);
            inv_out+=strlen(inv_out);

            if(mode<1)
                continue;

            source=NULL;
            switch(pdt) {
            case 31: source="Satellite product"; break;
            case 32: source="Simulated (synthetic) satellite product"; break;
            case 33: source="Ensemble member, simulated (synthetic), satellite  product"; break;
            case 34: source="Continuous, ensemble member, simulated (synthetic), satellite product"; break;
            }
            if(source) {
                sprintf(inv_out,"%s     source:               %s",nl,source);
                inv_out+=strlen(inv_out);
            }



            if(classification) {
                sprintf(inv_out,"%s     code1: classification %s",nl,satellite);
                inv_out+=strlen(inv_out);
            }
            if(satellite) {
                sprintf(inv_out,"%s     code2: satellite      %s",nl,satellite);
                inv_out+=strlen(inv_out);
            }
            if(agency) {
                sprintf(inv_out,"%s     instr: agency         %s",nl,agency);
                inv_out+=strlen(inv_out);
            }
            if(instype) {
                sprintf(inv_out,"%s     instr: instype        %s",nl,instype);
                inv_out+=strlen(inv_out);
            }
            if(shortname) {
                sprintf(inv_out,"%s     instr: shortname      %s",nl,shortname);
                inv_out+=strlen(inv_out);
            }
            if(longname) {
                sprintf(inv_out,"%s     instr: longname       %s",nl,longname);
                inv_out+=strlen(inv_out);
            }

            sprintf(inv_out,"%s     band:  wavelength     %.3lf um%s     band:  "
                    "frequency      %.3lg GHz%s     band:  energy         %.3lg eV",
                    nl, 1.e6/value, nl, c*value/1e9, nl, h*c*value * J2eV);
            inv_out+=strlen(inv_out);

            if(instype) {
                ipol=instrument >> 13;
                pol=NULL;
                switch(ipol) {
                case 1: pol="unpolarized"; break;
                case 2: pol="horizontal linear (H)"; break;
                case 3: pol="vertical linear (V)"; break;
                case 4: pol="right hand circular"; break;
                case 5: pol="left hand circular"; break;
                default: pol="unknown"; break;
                }
                if(pol)
                    sprintf(inv_out,"%s     band:  polarization   %s",nl,pol);
                inv_out+=strlen(inv_out);
            }

	}
    }
    return 0;
}
