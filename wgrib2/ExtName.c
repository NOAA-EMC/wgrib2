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
 * 2/2021 added flavors of the extended name
 *        change int use_ext_name to unsigned int type_ext_name
 *            type_ext_name == 0   ext_name turned off
 *                             &1    add misc
 *                             &2    add level
 *                             &4    add ftime
 */

extern struct codetable_4_230  codetable_4_230_table[];
char ext_name_field;
char ext_name_space;

/*
 * HEADER:100:misc:inv:0:variable name qualifiers like chemical, ensemble, probability, etc
 */
int f_misc(ARG0) {

    const char *string;
    int pdt, val, j;
    char *inv_out_init;
 
    if (mode < 0) return 0;

    pdt = GB2_ProdDefTemplateNo(sec);
    inv_out_init = inv_out += strlen(inv_out);

    f_ens(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }

    f_cluster(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }

    f_prob(call_ARG0(inv_out,NULL));
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }
    
    f_spatial_proc(call_ARG0(inv_out,NULL));
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }

    f_wave_partition(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }

    f_post_processing(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }
    inv_out += strlen(inv_out);

    f_JMA(call_ARG0(inv_out,NULL) );
    if (strlen(inv_out)) {
	strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }

    /* end of f_XXX(call_ARG0(inv_out,NULL) ); */

    val = code_table_4_3(sec);
    if (val == 5) {
	strcat(inv_out,"probability forecast:");
        inv_out += strlen(inv_out);
    }
    else if (val == 6 || val == 7) {
	strcat(inv_out,"analysis/forecast error:");
        inv_out += strlen(inv_out);
    }
    else if (val == 9) {
	strcat(inv_out,"climatological:");
        inv_out += strlen(inv_out);
    }
    else if (GB2_Center(sec) == 7 && val == 192) {
	strcat(inv_out,"Confidence Indicator:");
        inv_out += strlen(inv_out);
    }
    else if (GB2_Center(sec) == 7 && val == 194) {
	strcat(inv_out,"Neighborhood Probability:");
        inv_out += strlen(inv_out);
    }
    else if (val >= 192 && val != 255) {
	sprintf(inv_out,"process=%d:", val);
	inv_out += strlen(inv_out);
    }

    if (pdt == 7) {
	strcat(inv_out,"analysis/forecast error:");
	inv_out += strlen(inv_out);
    }
    else if (pdt == 6 || pdt == 10) {
        f_percent(call_ARG0(inv_out,NULL) );
	strcat(inv_out," level:");
	inv_out += strlen(inv_out);
    }
    else if (pdt >= 31 && pdt <= 34) {
        f_spectral_bands_extname(call_ARG0(inv_out,local));
	strcat(inv_out,":");
	inv_out += strlen(inv_out);
    }

   if ( (val = code_table_4_230(sec)) != -1) {
        strcat(inv_out,"chemical=");
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
	strcat(inv_out,":");
	inv_out += strlen(inv_out);
    }
    if ( (val = code_table_4_233(sec)) != -1) {	// use code table 4.230
        strcat(inv_out,"aerosol=");
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
        strcat(inv_out,":");
        inv_out += strlen(inv_out);
    }

    if (pdt >= 44 && pdt <= 47) {
        f_aerosol_size(call_ARG0(inv_out,NULL));
	strcat(inv_out,":");
	inv_out += strlen(inv_out);
    }
    if (pdt == 48 || pdt == 49) {	/* aerosol with optical properties */
        f_aerosol_size(call_ARG0(inv_out,NULL));
	inv_out += strlen(inv_out);
        f_aerosol_wavelength(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
	inv_out += strlen(inv_out);
    }
    if ( (val = code_table_1_2(sec)) > 1) {	// 0 and 1 are expected
	f_type_reftime(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
	inv_out += strlen(inv_out);
    }

    /* get rid of tailing : */
    if (inv_out > inv_out_init) {
        if (inv_out[-1] == ':') inv_out[-1] = 0;
    }

    return 0;
}

/*
 * HEADER:400:set_ext_name:setup:1:X=type ext_name (1*misc+2*level+4*ftime)
 */

unsigned int type_ext_name;

int f_set_ext_name(ARG1) {
    int i;
    i = atoi(arg1);
    if (i < 0) fatal_error("set_ext_name: arg < 0 (%d)",i);
    type_ext_name = i;
    return 0;
}

/*
 * HEADER:400:ext_name:inv:0:extended name, var+qualifiers
 */

int f_ext_name(ARG0) {
    int itmp;
    if (mode >= 0) {
        itmp = type_ext_name;
	/* if type_ext_name == 0 , use type_ext_name = 0, to remain compatible with old code */
        type_ext_name = (type_ext_name) ? type_ext_name : 1;
        getExtName(sec, mode, NULL, inv_out, NULL, NULL);
        type_ext_name = itmp;
    }
    return 0;
}


/* 

  getExtName : if (type_ext_name == 0) return old name
               else return extended name
   
  get extend name - need to change some characters   ... version 2 for the format
  space -> ext_name_space
  colon -> ext_name_field
 */

int getExtName(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *units) {

    char string[STRING_SIZE], *p;
    int i, len;

    if (sec == NULL) return 1;

    /* arguments for ARG0 */
    float *data = NULL;
    unsigned int ndata = 0;

    getName(sec, mode, inv_out, name, desc, units);

    if (type_ext_name == 0) return 0;

    p = name + strlen(name);

    /* misc */
    if (type_ext_name & 1) {
        string[0] = 0;
        f_misc(call_ARG0(string,NULL));
	len = strlen(string);
	if (len) {
            *p++ = ext_name_field;
	    for (i = 0; i < len; i++) {
		p[i] = string[i];
	    }
	    p += len;
	    *p = 0;
        }
    }

    /* lev */
    if (type_ext_name & 2) {
        string[0] = 0;
	f_lev(call_ARG0(string,NULL));
	len = strlen(string);
	if (len) {
            *p++ = ext_name_field;
	    for (i = 0; i < len; i++) {
		p[i] = string[i];
	    }
	    p += len;
	    *p = 0;
	}
    }

    /* ftime */
    if (type_ext_name & 4) {
        string[0] = 0;
	f_ftime(call_ARG0(string,NULL));
	len = strlen(string);
	if (len) {
            *p++ = ext_name_field;
	    for (i = 0; i < len; i++) {
		p[i] = string[i];
	    }
	    p += len;
	    *p = 0;
	}
    }
    len = strlen(name);
    for (i = 0; i < len; i++) {
	if (name[i] == ' ') name[i] = ext_name_space;
	if (name[i] == ':') name[i] = ext_name_field;
    }
    return 0;
}


/*
 * HEADER:400:set_ext_name_chars:setup:2:extended name characters X=field Y=space
 */
int f_set_ext_name_chars(ARG2) {
    if (mode == -1) {
	if (arg1[0] == 0) fatal_error("set_ext_name_chars: arg1 is empty","");
	if (arg2[0] == 0) fatal_error("set_ext_name_chars: arg2 is empty","");
        ext_name_field = arg1[0];
        ext_name_space= arg2[0];
    }
    return 0;
}

/*
 * HEADER:400:full_name:inv:0:extended name, var+misc+lev (depreciated)
 */

int f_full_name(ARG0) {
    int i;
    if (mode >= 0) {
        i = type_ext_name;
        type_ext_name = 1;
        getExtName(sec, mode, NULL, inv_out, NULL, NULL);
	type_ext_name = i;
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
