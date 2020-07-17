#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * ext_name: extended variable names
 *
 * A one time, the variable name was sufficient to identify the field
 *  along came, probabilities (50% precent chance was different from a 10% chance)
 *              ensembles
 *              statistical processing
 *              mass_density and chemical type
 *
 * Now we have "compound" variables - ensembles of chemical-types
 * Sooner or later .. 30% chance, ensemble member, daily mean, O3
 *
 * To handle the current and future extentions
 *
 *  Part A:  -f_misc
 *           inventory to print out the extensions  
 *           format :A=value:B=value:C=value:
 *  Part B   getExtName
 *           like getName but returns extended name
 *
 * public domain 10/2010: Wesley Ebisuzaki
 */

extern struct codetable_4_230  codetable_4_230_table[];

/*
 * HEADER:100:misc:inv:0:variable name qualifiers like chemical, ensemble, probability, etc
 */
int f_misc(ARG0) {

    const char *string;
    int need_space = 0;
    int pdt, val, j;
    static int error_count = 0;

    if (mode < 0) return 0;

    pdt = GB2_ProdDefTemplateNo(sec);
    inv_out += strlen(inv_out);

    f_ens(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	if (need_space) strcat(inv_out,":");
	need_space = 1;
        inv_out += strlen(inv_out);
    }

    f_prob(call_ARG0(inv_out,NULL));
    if (strlen(inv_out)) {
	if (need_space) strcat(inv_out,":");
	need_space = 1;
        inv_out += strlen(inv_out);
    }
    
    f_spatial_proc(call_ARG0(inv_out,NULL));
    if (strlen(inv_out)) {
	if (need_space) strcat(inv_out,":");
	need_space = 1;
        inv_out += strlen(inv_out);
    }

    f_wave_partition(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	if (need_space) strcat(inv_out,":");
	need_space = 1;
        inv_out += strlen(inv_out);
    }
    inv_out += strlen(inv_out);

    f_JMA(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	if (need_space) strcat(inv_out,":");
	need_space = 1;
        inv_out += strlen(inv_out);
    }


    /* end of f_XXX(call_ARG0(inv_out,NULL) ); */


    val = code_table_4_3(sec);
    if (val == 5) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"probability forecast");
    }
    else if (val == 6 || val == 7) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"analysis/forecast error");
	need_space = 1;
    }
    else if (val == 9) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"climatological");
	need_space = 1;
    }
    else if (GB2_Center(sec) == 7 && val == 192) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"Confidence Indicator");
	need_space = 1;
    }
    else if (GB2_Center(sec) == 7 && val == 194) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"Neighborhood Probability");
	need_space = 1;
    }
    else if (val >= 192 && val != 255) {
	if (need_space) strcat(inv_out,":");
	inv_out += strlen(inv_out);
	sprintf(inv_out,"process=%d", val);
	need_space = 1;
    }

    if (pdt == 7) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"analysis/forecast error");
	need_space = 1;
    }
    else if (pdt == 6 || pdt == 10) {
	if (need_space) strcat(inv_out,":");
	inv_out += strlen(inv_out);
        f_percent(call_ARG0(inv_out,NULL) );
	strcat(inv_out," level");
	need_space = 1;
    }
    else if (pdt >= 31 && pdt <= 34) {
	if (need_space) strcat(inv_out,":");
	inv_out += strlen(inv_out);
        f_spectral_bands_extname(call_ARG0(inv_out,local));
	need_space = 1;
    }

   if ( (val = code_table_4_230(sec)) != -1) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"chemical=");
        val = code_table_4_230(sec);
        if (val >= 0) {
            if (GB2_MasterTable(sec) <= 4) {
                if (error_count++ <= 10) {
		    if (GB2_Center(sec) == ECMWF) 
			fprintf(stderr, "Warning: possible incompatible chemistry table .. table turned off.\n");
		    else if (GB2_Center(sec) != NCEP)
			fprintf(stderr,
                    "Warning: if file made with ECMWF API, possible incompatible chemistry table\n");
		}
            }

	    j = 0;
	    string=NULL;
	    while (codetable_4_230_table[j].no != 65365) {
		if (codetable_4_230_table[j].no == val) {
		    string = codetable_4_230_table[j].name;
		    break;
		}
		j++;
	    }

            if (GB2_MasterTable(sec) <= 4 && GB2_Center(sec) == ECMWF) {
                string = NULL;
            }
            if (string != NULL)  strcat(inv_out,string);
            else {
		inv_out += strlen(inv_out);
		sprintf(inv_out,"%d",val);
	    }
	}
	need_space = 1;
    }

    if ( (val = code_table_4_233(sec)) != -1) {
	if (need_space) strcat(inv_out,":");
	strcat(inv_out,"aerosol=");
        val = code_table_4_233(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.233.dat"
            }
            if (string != NULL)  strcat(inv_out,string);
            else {
                inv_out += strlen(inv_out);
                sprintf(inv_out,"chemical_%d",val);
            }
        }
        need_space = 1;
    }

    if (pdt >= 44 && pdt <= 47) {
	if (need_space) strcat(inv_out,":");
	inv_out += strlen(inv_out);
        f_aerosol_size(call_ARG0(inv_out,NULL));
	need_space = 1;
    }
    if (pdt == 48) {
	if (need_space) strcat(inv_out,":");
	inv_out += strlen(inv_out);
        f_aerosol_size(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
	inv_out += strlen(inv_out);
        f_aerosol_wavelength(call_ARG0(inv_out,NULL));
	need_space = 1;
    }

    return 0;
}

/*
 * HEADER:400:set_ext_name:setup:1:X=0/1 extended name on/off
 */

int use_ext_name;

int f_set_ext_name(ARG1) {
    use_ext_name = atoi(arg1);
    return 0;
}

/*
 * HEADER:400:ext_name:inv:0:extended name, var+qualifiers
 */

int f_ext_name(ARG0) {
    int itmp;
    if (mode >= 0) {
        itmp = use_ext_name;
        use_ext_name = 1;
        getExtName(sec, mode, NULL, inv_out, NULL, NULL,".","_");
        use_ext_name = itmp;
    }
    return 0;
}


/* 

  getExtName : if (use_ext_name == 0) return old name
               else return extended name
   
  get extend name - need to change some characters   ... version 2 for the format
  space -> *space
  colon -> *space
  delim -> *delin
 */

int getExtName(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *units,
   const char *delim, const char *space) {

    char misc_string[STRING_SIZE], *p;
    int i;

    if (sec == NULL) return 1;

    /* arguments for ARG0 */
    float *data = NULL;
    unsigned int ndata = 0;

    getName(sec, mode, inv_out, name, desc, units);

    if (use_ext_name == 0) return 0;

    misc_string[0] = 0;
    f_misc(call_ARG0(misc_string,NULL));

    p = name + strlen(name);
    if (strlen(misc_string) > 0) *p++ = '.';

    for (i = 0; i < strlen(misc_string) ; i++) {
        if (misc_string[i] == ':') *p++ = '.';
        else if (misc_string[i] == ' ') *p++ = '_';
        else *p++ = misc_string[i];
    }
    *p = 0;
    return 0;
}

/*
 * HEADER:400:full_name:inv:0:extended name, var+qualifiers
 */

int f_full_name(ARG0) {
    int i;
    if (mode >= 0) {
        i = use_ext_name;
        use_ext_name = 1;
        getExtName(sec, mode, NULL, inv_out, NULL, NULL,".","_");
	use_ext_name = i;
	inv_out += strlen(inv_out);
	*inv_out++ = '.';
	*inv_out = 0;
	f_lev(call_ARG0(inv_out,NULL));
	for (i = 0; inv_out[i]; i++) {
	    if (inv_out[i] == ' ') inv_out[i] = '_';
	}
    }
    return 0;
}
