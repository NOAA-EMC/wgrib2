/** @file
 * @brief Routines to modify GRIB fields.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** File header flag. */
extern int header;

/** Decode grib file flag. */
extern int decode;

/** Level table (Code Table 4.5) */
extern const char *level_table[192];

/** NCEP level table (Code Table 4.5) */
extern const char *ncep_level_table[64];

/** Use scaling flag. */
extern int use_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/*
 * this takes a string and returns a fixed surface
 */

/**
 * Takes a string and returns a fixed surface.
 *
 * @param sec Pointer to the GRIB sections.
 * @param string Input string to parse.
 * @param table_4_5 Pointer to the table 4.5 value.
 * @param scale_factor Pointer to the scale factor.
 * @param scale_value Pointer to the scale value.
 * 
 * @return 0 on success, 1 on failure.
 */
int parse_level1(unsigned char **sec, const char *string, int *table_4_5, int *scale_factor, int *scale_value) {

    int i, n_percent, slen, n, ival, len_string, center;
    const char *t;
    char tmp_string[100];
    float val;
    double dval;

    len_string = strlen(string);
    for (i = 1; i < 192; i++) {		// can skip i==0 because it is not allowed resevered
        if (strcmp("reserved", level_table[i]) == 0) continue;
        // count the number of % characters in the level table
        n_percent = 0;
        t = level_table[i];
	/* %% -> quoted %, otherwise a read */

        while (*t) { 
            if (*t++ == '%') {
                if (*t == 0) break;
                if (*t++ != '%') n_percent++; 
            }
        }
        if (n_percent == 0) {
            if (strcmp(string, level_table[i]) == 0) {
                *table_4_5 = i;
                *scale_factor = 255;
                return 0;
            }
        }
        else if (n_percent == 1) {
            slen=strlen(level_table[i]);
            if (slen > sizeof(tmp_string) - 3) fatal_error("parse_level1: string overflow","");
            strncpy(tmp_string,level_table[i],slen);
            strncpy(tmp_string+slen,"%n",3);

            if (n = -1, sscanf(string,tmp_string,&val,&n), n == len_string) {
                dval = (double) val;
                *table_4_5 = i;
                if (i == 100 || i == 108) dval = dval * 100; // convert mb to Pa

                best_scaled_value(dval, &n, &ival);
                *scale_factor = n;
                *scale_value = ival;
                return 0;
            }
        }

    }

    /* check local level tables (see Level.c) */
    center = GB2_Center(sec);
    if (center == NCEP) {
        for (i = 0; i < 64; i++) {
            if (strcmp("reserved", ncep_level_table[i]) == 0) continue;
            n_percent = 0;
            t = ncep_level_table[i];
            /* %% -> quoted %, otherwise a read */

            while (*t) { 
                if (*t++ == '%') {
                    if (*t == 0) break;
                    if (*t++ != '%') n_percent++; 
                }
            }
            if (n_percent == 0) {
                if (strcmp(string, ncep_level_table[i]) == 0) {
                    *table_4_5 = i + 192;
                    *scale_factor = 255;
                    return 0;
                }
            }
            else if (n_percent == 1) {
                slen=strlen(ncep_level_table[i]);
                if (slen > sizeof(tmp_string) - 3) fatal_error("parse_level1: string overflow","");
                strncpy(tmp_string,ncep_level_table[i],slen);
                strncpy(tmp_string+slen,"%n",3);

                if (n = -1, sscanf(string,tmp_string,&val,&n), n == len_string) {
                    dval = (double) val;
                    *table_4_5 = i + 192;
                    if (i + 192 == 235) dval = dval * 10; // convert C to 1/10C

                    best_scaled_value(dval, &n, &ival);
                    *scale_factor = n;
                    *scale_value = ival;
                    return 0;
                }
            }
        }
    }
    /* check for local level type 200 0 */
    if (n = -1, sscanf(string,"local level type %d %lf%n",&i, &dval, &n), n == len_string) {
        *table_4_5 = i;
        best_scaled_value(dval, &n, &ival);
        *scale_factor = n;
        *scale_value = ival;
        return 0;
    }
    /* check for local level type 200 */
    if (n = -1, sscanf(string,"local level type %d%n",&i, &n), n == len_string) {
        *table_4_5 = i;
        *scale_factor = 255;
        return 0;
    }

    return 1;
}

/*
 * HEADER:100:set_lev:misc:1:changes level code .. not complete
 */

/**
 * Changes the level/layer of the field. By design, the level/layer used by -set_lev 
 * option is the same level/layer as used in the inventory. 
 * 
 * The code used to set the level/layer are used by -set_lev, -set_metadata, and 
 * -set_metadata_str. So if you are unsure of the format of the level description, 
 * you first look for a file with the same or similar level. Failing to find such a 
 * file, you can try the -set_lev option or look at the tables in level.c source code. 
 * 
 * ## Usage
 * -set_lev LEVEL
 * 
 * LEVEL is either a level or layer description. (NCEP specific special levels are not listed)
 * 
 *  "surface",
 * "cloud base",
 * "cloud top",
 * "0C isotherm",
 * "level of adiabatic condensation from sfc",
 * "max wind",
 * "tropopause",
 * "top of atmosphere",
 * "sea bottom",
 * "entire atmosphere",
 * "cumulonimbus base",
 * "cumulonimbus top",
 * "%g K level",
 * "%g mb",
 * "mean sea level",
 * "%g m above mean sea level",
 * "%g m above ground",
 * "%g sigma level",
 * "%g hybrid level",
 * "%g m underground",
 * "%g K isentropic level",
 * "%g mb above ground",
 * "PV=%g (Km^2/kg/s) surface",
 * "%g Eta level",
 * "logarithmic hybrid level",
 * "snow level",
 * "mixed layer depth",
 * "hybrid height level",
 * "hybrid pressure level",
 * "%g generalized vertical height coordinate",
 * "%g m below sea level",
 * "%g m below water surface",
 * "lake or river bottom",
 * "bottom of sediment layer",
 * "bottom of thermally active sediment layer",
 * "bottom of sediment layer penetrated by thermal wave",
 * "maxing layer",
 * "bottom of root zone",
 * "top surface of ice on sea, lake or river",
 * "top surface of ice, und snow on sea, lake or river",
 * "bottom surface ice on sea, lake or river",
 * "deep soil",
 * "top surface of glacier ice and inland ice",
 * "deep inland or glacier ice",
 * "grid tile land fraction as a model surface",
 * "grid tile water fraction as a model surface",
 * "grid tile ice fraction on sea, lake or river as a model surface",
 * "grid tile glacier ice and inland ice fraction as a model surface",
 * "%g-%g mb above ground",
 * "%g-%g mb"
 * "%g-%g m below ground"
 * "%g-%g m above ground"
 * "%g-%g sigma layer",
 * "%g-%g m below sea level",
 * "%gC ocean isotherm%n", NCEP only
 * "atmos col"
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set_lev(ARG1) {
    unsigned char *p1, *p2;
    float val1, val2;
    int i, n, len_arg1, ival, dash;
    char string[STRING_SIZE];
    char layer_type1[20], layer_type2[20];

    int table_4_5, scale_factor, scale_value;
 
    if (mode < 0) return 0;

    len_arg1 = strlen(arg1);
    if (len_arg1 == 0) return 0;	// if empty string, NOP
    if (len_arg1 > STRING_SIZE+1) fatal_error("set_lev: arg1 too long","");

    if (mode == 99) fprintf(stderr,"set_lev: arg=%s\n", arg1);

    /* get fixed surface pointers */
    p1 = code_table_4_5a_location(sec);
    p2 = code_table_4_5b_location(sec);
    if (p1 == NULL) {
        if (strcmp("no_level", arg1) == 0) return 0;
        fatal_error("set_lev: PDT does not have fixed surfaces","");
    }

    /* set fixed surface to undefined */

    for (i = 0; i < 6; i++) p1[i] = (unsigned char) 255;

    if (p2 != NULL) {
        for (i = 0; i < 6; i++) p2[i] = (unsigned char) 255;
    }

    // (level)

    if (parse_level1(sec, arg1, &table_4_5, &scale_factor, &scale_value) == 0) {
        p1[0] = table_4_5;
        scaled_char(scale_factor, scale_value, p1 + 1);
        return 0;
    }

    // (level) - (level)
    if (p2 != NULL) {
        for (dash = 1; dash < len_arg1-4; dash++) {
            if (arg1[dash] != ' ') continue;
            if (arg1[dash+1] != '-') continue;
            if (arg1[dash+2] != ' ') continue;
            for (i = 0; i < dash; i++) {
                string[i] = arg1[i];
            }
            string[dash] = 0;
            if (parse_level1(sec, string, &table_4_5, &scale_factor, &scale_value) == 0) {
                p1[0] = table_4_5;
                scaled_char(scale_factor, scale_value, p1 + 1);
                for (i = dash+3; i <= len_arg1; i++) {
                    string[i-dash-3] = arg1[i];
                }
                if (parse_level1(sec, string, &table_4_5, &scale_factor, &scale_value) == 0) {
                    p2[0] = table_4_5;
                    scaled_char(scale_factor, scale_value, p2 + 1);
                    return 0;
	            }
                else {
                    for (i = 0; i < 6; i++) p1[i] = (unsigned char) 255;
                }
            }
        }
    }

    if (mode == 99) fprintf(stderr,"in set_lev arg1=%s\n", arg1);

    // n-n (string) layer
    if (n=-1, sscanf(arg1,"%g-%g %19s layer%n",&val1,&val2, layer_type1, &n), n == len_arg1) {
        layer_type1[19] = 0;
        i = -1;
        if (strncmp(layer_type1,"sigma",20) == 0)  i = 104;
        else if (strncmp(layer_type1,"hybrid",20) == 0)  i = 105;
        else if (strncmp(layer_type1,"Eta",20) == 0)  i = 111;
        if (i != -1) {
            p1[0] = p2[0] = i;
            best_scaled_value(val1, &n, &ival);
            scaled_char(n, ival, p1 + 1);
            best_scaled_value(val2, &n, &ival);
            scaled_char(n, ival, p2 + 1);
            return 0;
        }
    }

    // n-n (string) (string) layer
    if (n=-1, sscanf(arg1,"%g-%g %19s %19s layer%n",&val1,&val2, layer_type1, layer_type2, &n), n == len_arg1) {
        layer_type1[19] = layer_type2[19] = 0;
        i = -1;
        if (strncmp(layer_type1,"sigma",20) == 0 && strncmp(layer_type2,"height",20) == 0)  i = 115;
        else if (strncmp(layer_type1,"hybrid",20) == 0 && strncmp(layer_type2,"height",20) == 0)  i = 118;
        else if (strncmp(layer_type1,"hybrid",20) == 0 && strncmp(layer_type2,"pressure",20) == 0)  i = 119;
        else if (strncmp(layer_type1,"m",20) == 0 && strncmp(layer_type2,"ocean",20) == 0)  i = 161;
        if (i != -1) {
            p1[0] = p2[0] = i;
            best_scaled_value(val1, &n, &ival);
            scaled_char(n, ival, p1 + 1);
            best_scaled_value(val2, &n, &ival);
            scaled_char(n, ival, p2 + 1);
            return 0;
        }
    }

    if (n=-1, sscanf(arg1,"%g-%g mb above ground%n",&val1,&val2,&n), n == len_arg1) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        val1 *= 100.0;
        val2 *= 100.0;
        p1[0] = p2[0] = 108;
        best_scaled_value(val1, &n, &ival);
        scaled_char(n, ival, p1 + 1);
        best_scaled_value(val2, &n, &ival);
        scaled_char(n, ival, p2 + 1);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g mb%n",&val1,&val2,&n), n == len_arg1) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        val1 *= 100.0;
        val2 *= 100.0;
        p1[0] = p2[0] = 100;
        best_scaled_value(val1, &n, &ival);
        scaled_char(n, ival, p1 + 1);
        best_scaled_value(val2, &n, &ival);
        scaled_char(n, ival, p2 + 1);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g m below ground%n",&val1,&val2,&n), n == len_arg1) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        p1[0] = p2[0] = 106;
        best_scaled_value(val1, &n, &ival);
        scaled_char(n, ival, p1 + 1);
        best_scaled_value(val2, &n, &ival);
        scaled_char(n, ival, p2 + 1);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g m above ground%n",&val1,&val2,&n), n == len_arg1) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        p1[0] = p2[0] = 103;
        best_scaled_value(val1, &n, &ival);
        scaled_char(n, ival, p1 + 1);
        best_scaled_value(val2, &n, &ival);
        scaled_char(n, ival, p2 + 1);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g m below sea level%n",&val1,&val2,&n), n == len_arg1) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        p1[0] = p2[0] = 160;

        best_scaled_value(val1, &n, &ival);
        scaled_char(n, ival, p1 + 1);
        best_scaled_value(val2, &n, &ival);
        scaled_char(n, ival, p2 + 1);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g generalized vertical height coordinate%n",&val1,&val2,&n), n == len_arg1) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        p1[0] = p2[0] = 150;

        best_scaled_value(val1, &n, &ival);
        scaled_char(n, ival, p1 + 1);
        best_scaled_value(val2, &n, &ival);
        scaled_char(n, ival, p2 + 1);
        return 0;
    }

    if (strcmp("atmos col", arg1) == 0 ||	// wgrib compatible
            strcmp("entire atmosphere (considered as a single layer)", arg1) == 0) {
        if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        p1[0] = 1;
        p2[0] = 8;
        return 0;
    }

    fatal_error("need to modify set_lev for %s", arg1);
    return 0;
}

/** NCEP Grib Table */
extern struct gribtable_s NCEP_gribtable[];

/** User Grib Table */
extern struct gribtable_s *user_gribtable;

/** ECMWF Grib Table */
extern struct gribtable_s ECMWF_gribtable[];

/** DWD1 Grib Table */
extern struct gribtable_s DWD1_gribtable[];

/** Local Grib Table */
extern struct gribtable_s local_gribtable[];

/*
 * HEADER:100:set_var:misc:1:changes variable name
 */

/** Name of the grib table to use. */
int names;

/**
 * Changes the variable name.
 * 
 * The -set_var option changes the variable name of the in-memory grib (sub-)message. You can 
 * write out the message with the new name by the -grib (fast) and -grib_out (slow) options. 
 * Of course if the in-memory grid values have changed, you have to use the latter option. 
 * 
 * You can set the name to a locally defined variable name only if the center is set correctly. 
 * For example, you can use an NCEP-defined variable name only if the center is already defined 
 * as NCEP. If the center is undefined, you cannot use any locally defined names. 
 * 
 * The search order is for WMO definitions and then the locally defined definitions. The search 
 * order has implications for NCEP users. NCEP often have WMO and NCEP definitions for the same 
 * variable name/type. The -set_var option will choose the WMO definition. This will cause 
 * problems with programs that expect the NCEP definition. 
 * 
 * The default is to search the NCEP defined names. However, wgrib2 will use the dwd or ecmwf 
 * names if the -set_names dwd or -set_names ecmwf options are used. 
 * 
 * The -set_var option will alter the master table. The grib2 standard says that variable 
 * definitions are only valid for specified master tables. Wgrib2 keeps track of range of tables 
 * for which each variable name is valid. In addition, there is an entry for the master table 
 * used by the -set_var option. 
 * 
 * ## Usage
 * -set_var X
 *
 * <pre>
 * X = valid grib variable name such as
 * 
 * 1. text name such as HGT or TMP
 *      - For WMO defined variables, either DWD, ECMWF, NCEP names are used depending on -set_names
 *      (default is NCEP)
 *      - local variable names have to consistent with the local center
 * 2. varA_B_C_D_E_F
 * 3. var discipline=A master_table=B parmcat=E parm=F
 * 4. var discipline=A center=D local_table=C parmcat=E parm=F
 *      - use 3 for WMO defined variables, 4 for locally defined variables 
 * 
 * A = discipline
 * B = version of master table
 * C = version of local table
 * D = center
 * E = parameter category
 * F = parameter
 * </pre>
 * 
 * Formats 2-4 were introduced with wgrib2 v2.0.7. They intended to allow manipulation of variables 
 * which are not in the grib table. Format 2 is the inverse of -set_varX and formats 3 and 4 are the 
 * inverse of the verbose -set_varX
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2/2012 | W. Ebisuzaki | old return 1st match
 *                         new match that is not in local tables,
 *                         if nothing, return match in local tables
 *                         not perfect, doesn't know center.
 * 4/2017 | W. Ebisuzaki | understands
 *                         var discipline=0 center=34 local_table=1 parmcat=1 parm=203
 *                         var discipline=10 master_table=2 parmcat=0 parm=11
 *                         var10_2_1_7_0_11
 * 1/2021 | W. Ebisuzaki | added ecmwf tables
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
 * $ wgrib2 p.grb
 * $ wgrib2 p.grb
 * 1:0:d=2009072100:PRES:mean sea level:anl:
 * $ wgrib2 p.grb -set_var TMP -grib out.grb
 * 1:0:d=2009072100:TMP:mean sea level:anl:
 * $ wgrib2 out.grb
 * 1:0:d=2009072100:TMP:mean sea level:anl:
 * @endcode
 * 
 * You can use -set_var to change from old variable names to the new variable names by the appropriate use of 
 * the -if option. This converts TSOIL (old) to SOILTMP (new):
 * 
 * @code{.sh}
 * $ wgrib2 old.grb -if ":TSOIL:" -set_var SOILTMP -fi -grib new.grb
 * @endcode
 * 
 * @note The -set_var option will rename all the fields in a grib file. If you only want to 
 * rename specific fields, you will have to use the -if and -fi options.
 *
 * @author Wesley Ebisuzaki @date 2/2012
 */
int f_set_var(ARG1) {
    struct gribtable_s *p;
    int i;
    unsigned int discipline, mastertab, localtab, center, parmcat, parmnum;

    /* this function needs to be called with changing parameters */
    if (mode < 0) return 0;

    /* var discipline=0 center=34 local_table=1 parmcat=1 parm=203 */
    /* var discipline=10 master_table=2 parmcat=0 parm=11 */
    /* var10_2_1_7_0_11 */
    if (arg1[0] == 'v' && arg1[1] == 'a' && arg1[2] == 'r') {
        i = sscanf(arg1,"var%u_%u_%u_%u_%u_%u", &discipline, &mastertab, &localtab, &center, &parmcat, &parmnum);
        if (i == 6) {
            sec[0][6] = discipline;
            sec[1][9] = mastertab;
            uint2_char(center,sec[1]+5);
            sec[1][10] = localtab;
            sec[4][9] = parmcat;
            sec[4][10] = parmnum;
            return 0;
        }
        i = sscanf(arg1,"var discipline=%u master_table=%u parmcat=%u parm=%u",&discipline,&mastertab,&parmcat,&parmnum);
        if (i == 4) {
            sec[0][6] = discipline;
            sec[1][9] = mastertab;
            sec[4][9] = parmcat;
            sec[4][10] = parmnum;
            return 0;
        }
        i = sscanf(arg1,"var discipline=%u center=%u local_table=%u parmcat=%u parm=%u",
                  &discipline, &center, &localtab, &parmcat, &parmnum);
        if (i == 5) {
            sec[0][6] = discipline;
            uint2_char(center,sec[1]+5);
            sec[1][10] = localtab;
            sec[4][9] = parmcat;
            sec[4][10] = parmnum;
            return 0;
        }
    }


    p = NULL;
    /* try user table */

    if (user_gribtable != NULL) {
        p = user_gribtable;
        center = GB2_Center(sec);
        while (p->disc != -1) {
            if (strcmp(arg1,p->name) == 0) {
                if (center == p->cntr) break;
                if (p->disc < 192 && p->pcat < 192 && p->pnum < 192) break;
            }
            p++;
        }
    }

    /* search for non-local table match first */
    if (p == NULL || p->disc == -1) {
        if (names == ECMWF) p = ECMWF_gribtable;
        else if (names == DWD1) p = DWD1_gribtable;
        else if (names == DWD2) p = DWD1_gribtable;
        else  p = NCEP_gribtable;
        while (p->disc != -1) {
            if (p->disc < 192 && p->pcat < 192 && p->pnum < 192 && strcmp(arg1,p->name) == 0) {
                break;
            }
            p++;
        }
    }

    /* try local tables */
    if (p->disc == -1) {
        center = GB2_Center(sec);
        if (center == ECMWF) p = ECMWF_gribtable;
        else if (center == NCEP) p = NCEP_gribtable;
        else if (center == DWD1) p = DWD1_gribtable;
        else if (center == DWD2) p = DWD1_gribtable;
        else p = local_gribtable;
        while (p->disc != -1) {
            if (center == p->cntr && strcmp(arg1,p->name) == 0) {
                break;
            }
            p++;
        }
    }

    if (p->disc == -1) fatal_error("set_var: could not find %s", arg1);
    sec[0][6] = p->disc;
    sec[1][9] = p->mtab_set;
    sec[1][10] = p->ltab;
    sec[4][9] = p->pcat;
    sec[4][10] = p->pnum;

    return 0;
}


/*
 * HEADER:-1:set_center:misc:1:changes center X = C or C:S     C and S are center/subcenter numbers
 */

/**
 * This option is deprecated. Please use the -set center option instead.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set_center(ARG1) {
    int i, center, subcenter;

    if (mode >= 0) {
        i = sscanf(arg1,"%d:%d", &center, &subcenter);
        if (i == 0) fatal_error("set_center: bad arg %s", arg1);
        int2_char(center, sec[1]+5);
        if (i == 2) int2_char(subcenter, sec[1]+7);
    }
    return 0;
}

/*
 * HEADER:100:set:misc:2:set X = Y, X=local_table,etc (help: -set help help)
 */

/** List of settable options. */
const char *set_options="discipline, center, subcenter, master_table, local_table, background_process_id, "
    "analysis_or_forecast_process_id, aerosol_size, aerosol_wavelength, process, model_version_date, "
    "chemical, aerosol, table_1.2/type_reftime, table_1.3, table_1.4, "
    "table_3.0, table_3.1/GDT, table_3.2, " 
    "table_3.3, table_3.4, table_4.0/PDT, table_4.1, table_4.2, table_4.3, table_4.5a, table_4.5b, table_4.6, "
    "table_4.7, table_4.8, table_4.10, "
    "table_4.11, table_4.230, table_4.233, table_5.0/DRT, table_6.0, %, cluster";

/** Code Table 4.230 - Atmospheric Chemical or Physical Type */
extern struct codetable_4_230  codetable_4_230_table[];

/**
 * Sets the specified metadata of the in-memory grib (sub-)message.
 *
 * You need to save the messages to make them permanent. 
 * 
 * Expect the list of supported fields to expand as needed. At present, the -set option 
 * only changes fields within the grib file. There is some overlap between various -set_* 
 * options and the -set option. (Ex. -set_center N, and -set center N.) The -set option is 
 * the newer method. The -set option is used by the -set_metadata option. 
 * 
 * ## Parameters that can be set
 * 
 *  The available parameters that can be set will depend on the version of wgrib2 being used. To see the parameter available,
 *
 * @code{.sh}
 * $ wgrib2 grib_file -set junk junk
 * *** FATAL ERROR: set asdf, allowed values: discipline, center, subcenter, master_table, 
 * local_table, background_process_id, analysis_or_forecast_process_id, aerosol_size, 
 * aerosol_wavelength, process, model_version_date, chemical, aerosol, table_1.2, table_1.3, 
 * table_1.4, table_3.0, table_3.1/GDT, table_3.2, table_3.3, table_3.4, table_4.0/PDT, 
 * table_4.1, table_4.2, table_4.3, table_4.5a, table_4.5b, table_4.6, table_4.7, table_4.8, 
 * table_4.10, table_4.11, table_4.230, table_4.233, table_5.0/DRT, table_6.0, %, cluster ***
 * @endcode
 * 
 * ## Usage
 * -set X Y
 * 
 * X = field
 * Y = integer/float/long long int (depending on X)
 * 
 * To find the values of X, use: wgrib2 - -set help all
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ wgrib2 IN.grb -set center 99 -center -grib OUT.grb
 * 1:0:center=De Bilt, Netherlands
 * 2:46042:center=De Bilt, Netherlands
 * 3:63079:center=De Bilt, Netherlands
 * 4.1:86046:center=De Bilt, Netherlands
 * ...
 * @endcode
 * 
 * <pre>
 * -set center 99       sets the "center" to a value of 99 for current grib message (De Bilt)
 * -center              prints the value of "center"
 * -grib OUT.grb        writes the current grib message to OUT.grb
 * </pre>
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set(ARG2) {
    int i, j, scale_factor, scale_value, prob_type, pdt;
    double val, val1, val2;
    int year,mon,day,hr,minute,second;
    unsigned char *p;
    char *endptr;
    unsigned int k;

    if (mode == -1) {
        if (strcmp(arg1,"help") == 0) {
            sprintf(inv_out,"-set: change values of %s\n", set_options);
            return -1;
        }
        return 0;
    }

    if (mode >= 0) {
        pdt = GB2_ProdDefTemplateNo(sec);
        if (strcmp(arg1,"data_*") == 0) {
            val = strtod(arg2, &endptr);
            if (*endptr == 0) {
                if (decode == 1 && data != NULL) {
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                    for (k = 0; k < ndata; k++) {
                        if (DEFINED_VAL(data[k])) {
                            data[k] *= val;
                        }
                    }
                }
                else {
                    fatal_error("set data_*: need option that uses grid data","");
                }
            }
            else {
                fatal_error("set data_*: bad value %s", arg2);
            }
            return 0;
        }

        if (strcmp(arg1,"data_+") == 0) {
            val = strtod(arg2, &endptr);
            if (*endptr == 0) {
                if (decode == 1 && data != NULL) {
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                    for (k = 0; k < ndata; k++) {
                        if (DEFINED_VAL(data[k])) {
                            data[k] += val;
                        }
                    }
                }
                else {
                    fatal_error("set data_+: need option that uses grid data","");
                }
            }
            else {
                fatal_error("set data_+: bad value %s", arg2);
            }
            return 0;
        }

        i = atoi(arg2);
        if (strcmp(arg1,"discipline") == 0 || strcmp(arg1,"table_0.0") == 0) {
            sec[0][6] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"local_table") == 0 || strcmp(arg1,"table_1.1") == 0) {
            sec[1][10] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"master_table") == 0 || strcmp(arg1,"table_1.0") == 0) {
            sec[1][9] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"center") == 0) {
            int2_char(i, sec[1]+5);
            return 0;
        }
        if (strcmp(arg1,"subcenter") == 0) {
            int2_char(i, sec[1]+7);
            return 0;
        }
        if (strcmp(arg1,"background_process_id") == 0) {
            p = background_generating_process_identifier_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"analysis_or_forecast_process_id") == 0) {
            p = analysis_or_forecast_generating_process_identifier_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"model_version_date") == 0) {
            p = year_of_model_version_date_location(sec);
            if (p) {
                i = sscanf(arg2,"%4d%2d%2d%2d%2d%2d", &year,&mon,&day,&hr,&minute,&second);
                if (i != 6) fatal_error("set model_version_date YYYYMMDDHHmmSS","");
                uint2_char(year, p);
                p += 2;
                *p++ = mon;
                *p++ = day;
                *p++ = hr;
                *p++ = minute;
                *p++ = second;
            }
            return 0;
        }
        if (strcmp(arg1,"type_reftime") == 0 || strcmp(arg1,"table_1.2") == 0) {
            /* check if number */
            if (is_uint(arg2)) {
                sec[1][11] = (unsigned char) i;
                return 0;
            }
            i = -1;
            if (strcmp(arg2, "analysis") == 0) i = 0;
            if (strcmp(arg2, "start of forecast") == 0) i = 1;
            if (strcmp(arg2, "verifying time of forecast") == 0) i = 2;
            if (strcmp(arg2, "observation time") == 0) i = 3;
            if (strcmp(arg2, "local time") == 0) i = 4;
            if (strcmp(arg2, "unknown") == 0) i = 255;
            if (i == -1) fatal_error_ss("set: bad arg %s %s", arg1, arg2);
            sec[1][11] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_1.3") == 0) {
            sec[1][19] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_1.4") == 0) {
            sec[1][20] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_3.0") == 0) {
            sec[3][5] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_3.1") == 0 || strcmp(arg1,"GDT") == 0) {
            uint2_char(i, sec[3]+12);
            return 0;
        }
        if (strcmp(arg1,"table_3.2") == 0) {
            p = code_table_3_2_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_3.3") == 0 || strcmp(arg1,"flag_table_3.3") == 0) {
            return set_flag_table_3_3(sec, i);
        }
        if (strcmp(arg1,"table_3.4") == 0 || strcmp(arg1,"flag_table_3.4") == 0) {
            return set_flag_table_3_4(sec, i);
        }
        if (strcmp(arg1,"table_4.0") == 0 || strcmp(arg1,"PDT") == 0) {
            uint2_char(i, sec[4]+7);
            return 0;
        }
        if (strcmp(arg1,"table_4.1") == 0) {
            sec[4][9] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.2") == 0) {
            sec[4][10] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.3") == 0 || strcmp(arg1,"process") == 0) {
            p = code_table_4_3_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.5a") == 0) {
            p = code_table_4_5a_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.5b") == 0) {
            p = code_table_4_5b_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.6") == 0) {
            p = code_table_4_6_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.7") == 0) {
            p = code_table_4_7_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.8") == 0) {
            p = code_table_4_8_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.10") == 0) {
            p = code_table_4_10_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.11") == 0) {
            p = code_table_4_11_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"table_4.230") == 0 || strcmp(arg1, "chemical") == 0) {
            p = code_table_4_230_location(sec);
            if (p == NULL) return 0; /* non-chemical template */
            /* check if number */
            if (is_uint(arg2)) {
                uint2_char(i, p);
                return 0;
            }
            /* must be a name */
            j = 0;
            while (codetable_4_230_table[j].no != 65535) {
                if (strcmp(arg2, codetable_4_230_table[j].name) == 0) {
                    uint2_char(codetable_4_230_table[j].no, p);
                    return 0;
                }
                j++;
            }
            fatal_error("set table_4.230/chemical unrecognized chemical %s", arg2);
        }
        if (strcmp(arg1,"table_4.233") == 0 || strcmp(arg1, "aerosol") == 0) {
            p = code_table_4_233_location(sec);
            if (p == NULL) return 0; /* non-aerosol template */

            /* check if number */
            if (is_uint(arg2)) {
               uint2_char(i, p);
               return 0;
            }
            /* must be a name, table 4.233 is the same as 4.230 */
            j = 0;
            while (codetable_4_230_table[j].no != 65535) {
                if (strcmp(arg2, codetable_4_230_table[j].name) == 0) {
                    uint2_char(codetable_4_230_table[j].no, p);
                    return 0;
                }
                j++;
            }
            fatal_error("set table_4.233/aerosol unrecognized aerosol %s", arg2);
        }
        if (strcmp(arg1,"table_5.0") == 0 || strcmp(arg1,"DRT") == 0) {
            uint2_char(i, sec[5]+9);
            return 0;
        }
        if (strcmp(arg1,"table_6.0") == 0) {
            sec[6][5] = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"%") == 0) {
            p = percentile_value_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"cluster") == 0) {
            p = cluster_identifier_location(sec);
            if (p) *p = (unsigned char) i;
            return 0;
        }
        if (strcmp(arg1,"aerosol_size") == 0) {
            if (pdt != 44 && pdt != 45 && pdt != 46 && pdt != 48 && pdt != 49) return 0;
            p = code_table_4_91_location(sec);
            if (p == NULL) return 0;
            i = scan_code_table_4_91(&prob_type, &val1, &val2, arg2);
            if (i == 0) {
                p[0] = prob_type;
                if (DEFINED_VAL(val1)) {
                    best_scaled_value(val1, &scale_factor, &scale_value);
                    scaled_char(scale_factor, scale_value, p+1);
                }
                else {
                    p[1] = p[2] = p[3] = p[4] = p[5] = 255;
                }
                if (DEFINED_VAL(val2)) {
                    best_scaled_value(val2, &scale_factor, &scale_value);
                    scaled_char(scale_factor, scale_value, p+6);
                }
                else {
                    p[6] = p[7] = p[8] = p[9] = p[10] = 255;
                }
            }
            else fatal_error("set aerosol_size bad size: %s", arg2);
            return 0;
        }
        if (strcmp(arg1,"aerosol_wavelength") == 0) {
            if (pdt != 44 && pdt != 45 && pdt != 46 && pdt != 48 && pdt != 49) return 0;
            p = code_table_4_91b_location(sec);
            if (p == NULL) return 0;
            i = scan_code_table_4_91(&prob_type, &val1, &val2, arg2);
            if (i == 0) {
                p[0] = prob_type;
                if (DEFINED_VAL(val1)) {
                    best_scaled_value(val1, &scale_factor, &scale_value);
                    scaled_char(scale_factor, scale_value, p+1);
                }
                else {
                    p[1] = p[2] = p[3] = p[4] = p[5] = 255;
                }
                if (DEFINED_VAL(val2)) {
                    best_scaled_value(val2, &scale_factor, &scale_value);
                    scaled_char(scale_factor, scale_value, p+6);
                }
                else {
                    p[6] = p[7] = p[8] = p[9] = p[10] = 255;
                }
            }
            else fatal_error("set aerosol_wavelength bad size: %s", arg2);
            return 0;
        }
        fatal_error("set %s, allowed values: %s", arg1, set_options);
    }
    return 0;
}


/*
 * HEADER:100:set_ave:misc:1:set ave/acc .. only use on pdt=4.0/4.8 (old code)
 */

/* the old set_ftime/set_ave is being replaced by a new version of set_ftime */

/**
 * Set the ave/acc. Only use on pdt=4.0/4.8 (old code).
 * 
 * Prior to 7/2016, ftime v1 was used to print the "ftime" values. One used set_time v1 
 * and set_ave to modify the "ftime" values. After 7/2016, the version 1 routines because 
 * -ftime1, -set_ftime1 and -set_ave. The version 2 routines became -ftime2 and -set_ftime2. 
 * The -set_ave was not used. For -ftime and -set_ftime options called -ftime2 and -set_ftime2 
 * by default. 
 * 
 * Eventually the version 1 routines will be removed. 
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set_ave(ARG1) {
    int i, tr, tr2, len, len_arg1;
    char string[STRING_SIZE];
    char string2[STRING_SIZE];
    char string3[STRING_SIZE];
    const char *string4;
    static unsigned char new_sec4[58+12+12];		// now use n == 3 forms

    int year, month, day, hour, minute, second;
    int j,k,j2,k2, m, m2, missing;

    if (mode < 0) return 0;

    len_arg1 = strlen(arg1);

//  6 hour ave anl

    i = sscanf(arg1,"%d %s %s %s%n",&j,string,string2,string3,&len);
    if (len != len_arg1) i = 0;
    if (i == 4 && ((tr = a2time_range(string)) >= 0)) {

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, j, tr);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);
        uint2_char(8, new_sec4+7);              // pdt = 8
        new_sec4[17] = tr;		    	// hour, etc
        int_char(0, new_sec4+18);              // start time
        new_sec4[41] = 1;                       // number of time ranges
        uint_char(0, new_sec4+42);              // missing

        if (strcmp(string2,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string2,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string2,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string2,"min") == 0) new_sec4[46] = 3;
        else if (strcmp(string2,"last-first") == 0) new_sec4[46] = 4;
        else if (strcmp(string2,"RMS") == 0) new_sec4[46] = 5;
        else if (strcmp(string2,"StdDev") == 0) new_sec4[46] = 6;
        else if (strcmp(string2,"covar") == 0) new_sec4[46] = 7;
        else if (strcmp(string2,"first-last") == 0) new_sec4[46] = 8;
        else fatal_error("set_ave: unknown statistical operator %s",string2);

        new_sec4[47] = 1;

        new_sec4[48] = tr;                       // hour
        uint_char(j, new_sec4+49);
        new_sec4[53] = tr;
        uint_char(0, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }

    // 1-5 hour ave fcst

    i = sscanf(arg1,"%d-%d %s %s %s%n",&j,&k,string3,string,string2,&len);
    if (len != len_arg1) i = 0;
    if (i == 5 && (tr = a2time_range(string3)) >= 0) {

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, k, tr);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);
        uint2_char(8, new_sec4+7);              // pdt = 8
        new_sec4[17] = tr;			// hour
        int_char(j, new_sec4+18);		// start time
        new_sec4[41] = 1;			// number of time ranges
        uint_char(0, new_sec4+42);		// missing

        if (strcmp(string,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string,"min") == 0) new_sec4[46] = 3;
        else if (strcmp(string,"last-first") == 0) new_sec4[46] = 4;
        else if (strcmp(string,"RMS") == 0) new_sec4[46] = 5;
        else if (strcmp(string,"StdDev") == 0) new_sec4[46] = 6;
        else if (strcmp(string,"covar") == 0) new_sec4[46] = 7;
        else if (strcmp(string,"first-last") == 0) new_sec4[46] = 8;
        else fatal_error("set_ave: unknown statistical operator %s", string);

        if (strcmp(string2,"anl") == 0) new_sec4[47] = 1;
        else if (strcmp(string2,"fcst") == 0) new_sec4[47] = 2;
        else fatal_error("set_ave: expecting anl/fcst got %s", string);

        new_sec4[48] = tr;			// hour
        uint_char(k-j, new_sec4+49);
        new_sec4[53] = tr;
        uint_char(0, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }

    // obsolete form: 1@6 hour ave anl,missing=0

    i = sscanf(arg1,"%d@%d %s ave anl,missing=%d%n",&j,&k,string,&missing,&len);
    if (len != len_arg1) i = 0;

    // new form 1@6 hour ave(anl),missing=0

    if (i != 4) {
        i = sscanf(arg1,"%d@%d %s ave(anl),missing=%d%n",&j,&k,string,&missing,&len);
        if (len != len_arg1) i = 0;
    }

    if (i == 4) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);		// length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr;                      // hour
        int_char(0, new_sec4+18);              // start time 0 = analysis

        new_sec4[41] = 1;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;                       // 0 = ave 1 = acc
        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }

// 1@6 hour ave(6 hour fcst),missing=0


    i = sscanf(arg1,"%d@%d %s ave(%d %s fcst),missing=%d%n",&j,&k,string, &m, string2, &missing,&len);
    if (len != len_arg1) i = 0;
    if (i == 6) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18) ;              // start time 0 = analysis

        new_sec4[41] = 1;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;                       // 0 = ave 1 = acc
        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }


// 1@6 hour ave(6 hour fcst)++,missing=0

    i = sscanf(arg1,"%d@%d %s ave(%d %s fcst)++,missing=%d%n",&j,&k,string, &m, string2, &missing,&len);
    if (len != len_arg1) i = 0;
    if (i == 6) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 1;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;                       // 0 = ave 1 = acc
        new_sec4[47] = 2;                       // 2 = forecast time is increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }


// 1@6 hour ave(0-6 hour ave fcst),missing=0

    i = sscanf(arg1,"%d@%d %s ave(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
    string4 = "ave";
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s acc(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "acc";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s min(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "min";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s max(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "max";
    }

    if (len != len_arg1) i = 0;
    if (i == 8) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m2, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(70, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 2;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

	// string4
        if (strcmp(string4,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string4,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string4,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string4,"min") == 0) new_sec4[46] = 3;
        else fatal_error("set_ave: unknown statistical operator %s",string4);

        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);

        if (strcmp(string3,"ave") == 0) new_sec4[46+12] = 0;
        else if (strcmp(string3,"acc") == 0) new_sec4[46+12] = 1;
        else if (strcmp(string3,"max") == 0) new_sec4[46+12] = 2;
        else if (strcmp(string3,"min") == 0) new_sec4[46+12] = 3;
        else fatal_error("set_ave: unknown statistical operator %s",string3);
        // new_sec4[46+12] = 0;                       // 0 = ave 1 = acc
        new_sec4[47+12] = 2;                       // same forecast
        new_sec4[53+12] = new_sec4[48+12] = tr2;
        uint_char(m2-m , new_sec4+49+12);
        uint_char(0, new_sec4+54+12);

        sec[4] = &(new_sec4[0]);
        return 0;
    }

// 1@6 hour ave(0-6 hour ave fcst)++,missing=0

    i = sscanf(arg1,"%d@%d %s ave(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
    string4 = "ave";
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s acc(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "acc";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s min(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "min";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s max(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "max";
    }
    if (len != len_arg1) i = 0;
    if (i == 8) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m2, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(70, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 2;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        // string4
        if (strcmp(string4,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string4,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string4,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string4,"min") == 0) new_sec4[46] = 3;
        else fatal_error("set_ave: unknown statistical operator %s",string4);

        new_sec4[47] = 2;                       // 2 = fcst time increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);

        if (strcmp(string3,"ave") == 0) new_sec4[46+12] = 0;
        else if (strcmp(string3,"acc") == 0) new_sec4[46+12] = 1;
        else if (strcmp(string3,"max") == 0) new_sec4[46+12] = 2;
        else if (strcmp(string3,"min") == 0) new_sec4[46+12] = 3;
        else fatal_error("set_ave: unknown statistical operator %s",string3);
        // new_sec4[46+12] = 0;                       // 0 = ave 1 = acc
        new_sec4[47+12] = 2;                       // same forecast
        new_sec4[53+12] = new_sec4[48+12] = tr2;
        uint_char(m2-m , new_sec4+49+12);
        uint_char(0, new_sec4+54+12);

        sec[4] = &(new_sec4[0]);
        return 0;
    }




// old 10@30 year ave(124@6 hour ave anl)
// 10@30 year ave(124@6 hour ave (anl))

    i = sscanf(arg1,"%d@%d %s ave(%d@%d %s ave (anl)),missing=%d%n",&j,&k,string, &j2,&k2,string2, &missing,&len);

    if (len != len_arg1) i = 0;
    if (i == 7) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, (j2-1)*k2, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr;                      // hour
        int_char(0, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 2;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;			// ave
        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);


        new_sec4[46+12] = 0;			// ave
        new_sec4[47+12] = 1;                    // 1 = start of forecast increased
        new_sec4[53+12] = new_sec4[48+12] = tr2;
        uint_char((j2-1)*k2, new_sec4+49+12);
        uint_char(k2, new_sec4+54+12);

        sec[4] = &(new_sec4[0]);
        return 0 ;
    }


// 10@30 year ave(124@6 hour ave 6 hour fcst)
// 10@30 year ave(124@6 hour ave(0-6 hour ave fcst)

    fatal_error("set_ave: not implemented %s", arg1);
    return 0;
}

/*
 * HEADER:-1:set_flag_table_3.3:misc:1:flag table 3.3 = X
 */

/**
 * Set flag table 3.3.
 * 
 * ## Usage
 * -set_flag_table_3_3 X
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set_flag_table_3_3(ARG1) {
    if (mode >= 0) return set_flag_table_3_3(sec, atoi(arg1));
    return 0;
}

/*
 * HEADER:-1:set_flag_table_3.4:misc:1:flag table 3.4 = X
 */

/**
 * Set flag table 3.4.
 * 
 * ## Usage
 * -set_flag_table_3_4 X
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
 */
int f_set_flag_table_3_4(ARG1) {
    if (mode >= 0) return set_flag_table_3_4(sec, atoi(arg1));
    return 0;
}
