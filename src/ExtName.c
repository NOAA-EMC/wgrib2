/** @file
 * @brief Extended variable names handling.
 * @author Public Domain: Wesley Ebisuzaki @date 10/2010
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 10/2010 | W. Ebisuzaki | Initial
 * 02/2021| W. Ebisuzaki | Added flavors of the extended name
 *                         change int use_ext_name to unsigned int type_ext_name
 *                          type_ext_name == 0   ext_name turned off
 *                              &1    add misc
 *                              &2    add level
 *                              &4    add ftime
 */
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

/** Code Table 4.230 - Atmospheric Chemical or Physical Type */
extern struct codetable_4_230  codetable_4_230_table[];

/** Extended variable name field. */
char ext_name_field;

/** Extended variable name space. */
char ext_name_space;

/*
 * HEADER:100:misc:inv:0:variable name qualifiers like chemical, ensemble, probability, etc
 */

/**
 * Prints out extended variable names qualifiers like chemical, ensemble, probability, etc.
 * 
 * This option is called by the -s option and many other similar options
 * 
 * ## Usage
 * -misc
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 percentile_precip.grib2 -stats
 * 1:0:75% level
 * 2:315649:90% level
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 percentile_precip.grib2 -s
 * 1:0:d=2014101012:TPRATE:surface:2@1 hour max(13-14 hour acc fcst)++,missing=0:75% level
 * 2:315649:d=2014101012:TPRATE:surface:2@1 hour max(13-14 hour acc fcst)++,missing=0:90% level
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 10/2010
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

/** Type of extension name. */
unsigned int type_ext_name;

/**
 * Select between the regular and extended name.
 * 
 * Why would you want use extended names? Sometimes you want the names to be unique. 
 * For example, when creating NetCDF file, you want the TMP from ensemble member 1 to 
 * has a different name from ensemble member 2. In another example, you may want to 
 * search for a unique field. With the extended name, you only need simple name comparison. 
 * 
 * ## Extended Names
 * The extended name includes modifiers like the ensemble information, dust information 
 * and probabilities. Optionally you can include the level and forecast information 
 * (wgrib2 v3.0.2+). For more information, see the documentation for -ext_name.
 * 
 * ## Extended Extended Names
 * The extended name facility was extended with wgrib2 v3.0.2 in order to fix a problem 
 * with converting grib to netcdf with some of the newer NCEP files. When wgrib2 writes 
 * netcdf files, the -netcdf concatinates the level information to the extended name to 
 * produce the netcdf name for the field. For some of the newer NCEP forecast files, this 
 * name wasn't unique. So the extended name needs to optionally include the forecast time 
 * information. With this extension, the level information was added as another option 
 * field to the extended name.
 *
 * $ wgrib2 FCST.grb -set_ext_num 1 -netcdf FCST.nc
 *   lost fields because the field names were not unique
 * $ wgrib2 FCST.grb -set_ext_num 5 -netcdf FCST.nc
 *   extended name include misc-info and forecast-time-info.
 * 
 * The final modification to the extended name, was to make the field and space character 
 * a run-time parameter by the option, -set_ext_name_chars. The default values are '.' and 
 * '_' for backwards compatibilty. The modification was needed we now are seeing level-info 
 * and misc-info with periods in them like "0.5 mb". 
 * 
 * ## -match, -if, and other string matches 
 * The extended names also applies the match inventory. So string matches will have to be 
 * rewritten. Fortunately the process is mechanical. 
 *      -match ":RH:"         ->     -match ":RH(:|.)"
 *      -match ":RH:500 mb:"  ->     -match ":RH(:|.)" -match ":500 mb:"
 * 
 * Many options use the extended name if enabled by -set_ext_name. For example, the -netcdf 
 * uses the extended name if enabled. 
 * 
 * ## Usage
 * To select between the regular and extended name, you use:
 * 
 * -set_ext_name N
 * 
 * N | Value
 * --|------
 * 0 | default
 * 1 | add misc terms like ensemble or probability
 * 2 | add level information
 * 3 | misc + level information
 * 4 | add forecast time information
 * 5 | misc + forecast information
 * 6 | level information + forecast information
 * 7 | misc + level information + forecast information
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ ./wgrib2 chem.grb2 -var
 * 1:0:MASSDEN
 * $ ./wgrib2 chem.grb2 -ext_name
 * 1:0:MASSDEN.hi-res_ctl.Water_Vapour
 * $ ./wgrib2 chem.grb2 -set_ext_name 1 -ext_name
 * 1:0:MASSDEN.hi-res_ctl.Water_Vapour
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 10/2010
 */
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

/**
 * Prints the extended name.
 * 
 * ## Extended Name for wgrib2 3.0.2+
 * Wgrib2 can write netcdf files, and -netcdf concatinates the level information to 
 * the extended name to produce the netcdf name for the field. For some of the newer 
 * NCEP forecast files, this name wasn't unique. So the extended name was expanded to 
 * be name.misc-info.level-info.forecast-time.info where the last 3 field were optional. 
 * 
 * $ wgrib2 FCST.grb -set_ext_num 1 -netcdf FCST.nc
 *   lost fields because the field names were not unique
 * $ wgrib2 FCST.grb -set_ext_num 5 -netcdf FCST.nc
 *   extended name include misc-info and forecast-time-info.
 * 
 * The final modification to the extended name, was to make the field and space character 
 * a run-time parameter by the option, -set_ext_name_chars. The default values are '.' and 
 * '_' for backwards compatibilty. The modification was needed we now are seeing level-info 
 * and misc-info with periods in them like "0.5 mb". The -var option prints the VARIABLE name 
 * of the grib message. Common names would be HGT and TMP for the geopotential height and the 
 * temperature. For most knowing the variable name, the level and the timimg information is all 
 * you need. Then things became more complicated. Eventually a file came along which had only one 
 * variable type (MASSDEN, mass density) but had a couple of important qualifier chemical type 
 * (H2O/O3/N02) and ensemble member ID. The -AAIG output was useless because its output used the 
 * variable name.
 * 
 * To fix the -AAIG output, an extended name was introduced. You can see the extended name by the 
 * -ext_name option. 
 * 
 * The extended name takes the output of -misc, changes the colons to periods, spaces to underscores 
 * and removes the text up to the equal size and appends it to the variable name. As of wgrib2 v1.9.0, 
 * the extended name is used with -AAIGc, -csv, and -netcdf. To stop using the extended name in -AAIG, 
 * -csv and -netcdf, use the option -set_ext_name 0.
 * 
 * The -ext_name prints the extended name when the extended name type is not zero. (The -set_ext_name 
 * option sets the extended name type.) 
 * 
 * ## Extended Name when the extended name type is 0
 * The -ext_name always prints an extended name. If the ext_name type is 0 which is the default value, 
 * -ext_name prints out an extended name with the misc information. One sets the extended name type 
 * using -set_ext_name. 
 * 
 * ## Usage
 * 
 * -ext_name
 * Prints extended name type N if N > 0. 
 * Prints extended name type 1 if N == 0.
 * 
 * Several options such as -netcdf use the extended name type differently. If the extend name type is 0, 
 * they use the regular name (-var). 
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ ./wgrib2 chem.grb2 -var
 * 1:0:MASSDEN
 * $ ./wgrib2 chem.grb2 -ext_name
 * 1:0:MASSDEN.hi-res_ctl.Water_Vapour
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 10/2010
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

/**
 * Get the extended name.
 * 
 * @param sec Pointer to the GRIB section.
 * @param mode Mode of operation (0 for normal, -1 for initialization).
 * @param inv_out Inventory output.
 * @param name Pointer to name of the variable.
 * @param desc Pointer to description of the variable.
 * @param units Pointer to units of the variable.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 10/2010
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

/**
 * Set the field separator and space replacement characters for extended names.
 * 
 * ## Usage
 * -set_ext_name_chars 'X' 'Y'
 * 
 * X and Y are characters
 * The default field separator '.' is replaced by 'X'.
 * The default space separator '_' is replaced by 'Y'.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ wgrib2 gep19.aec -set_ext_name 3 -ext_name  -set_ext_name_chars '=' '~' -var -misc -lev
 * 1:0:HGT=ENS=+19=200~mb:HGT:ENS=+19:200 mb
 * 2:70707:TMP=ENS=+19=200~mb:TMP:ENS=+19:200 mb
 * 3:96843:RH=ENS=+19=200~mb:RH:ENS=+19:200 mb
 * @endcode
 * 
 * <pre>
 *  The -var -misc -lev fields:  HGT:ENS=+19:200 mb
 *  are converted to an extended name: HGT=ENS=+19=200~mb
 *  using the field separater '=' and space replacement of '~'
 * </pre>
 * 
 * @author Wesley Ebisuzaki @date 10/2010
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

/**
 * (Deprecated) With the expansion of "Extended Names", the full name is now a subset of the new 
 * Extended Names. Please use the -ext_name option instead.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 10/2010
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
