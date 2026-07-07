/** @file
 * @brief Level processing functions.
 * @author Public Domain: Wesley Ebisuzaki @date 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Item delimiter string. */
extern const char *item_deliminator;

/*
 * HEADER:200:lvl:inv:0:level raw format (type,missing,scale,factor)
 */

/**
 * Prints the level information (Code Table 4.5) in raw format (type,missing,scale,factor).
 * 
 * ## Usage
 * -lvl
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2023
 */
int f_lvl(ARG0) {
    int level_type1, level_type2;
    float val1, val2;
    int undef_val1, undef_val2;
    unsigned char *p1, *p2;

    if (mode < 0) return 0;

    p1 = code_table_4_5a_location(sec);
    p2 = code_table_4_5b_location(sec);

    fixed_surfaces(sec, &level_type1, &val1, &undef_val1, &level_type2, &val2, &undef_val2);

    if (undef_val1 == 0) sprintf(inv_out,"lvl1=(%d,%lg,%d,%d)%c",level_type1,val1,(int) p1[1], int4(p1+2), *item_deliminator);
    else sprintf(inv_out,"lvl1=(%d,missing,missing,missing%c",level_type1,*item_deliminator);
    inv_out += strlen(inv_out);

    if (undef_val2 == 0) sprintf(inv_out,"lvl2=(%d,%lg,%d,%d)",level_type2,val2,(int)p2[1], int4(p2+2));
    else sprintf(inv_out,"lvl2=(%d,missing,missing,missing)",level_type2);
    inv_out += strlen(inv_out);

    return 0;
}

/*
 * HEADER:200:set_lvl1:misc:1:X=(type,missing,scale,factor) set level 1 raw format
 */

/**
 * Set level 1 (Code Table 4.5) raw format.
 * 
 * ## Usage
 * -set_lvl1 (type,missing,scale,factor)
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2023
 */
int f_set_lvl1(ARG1) {

    unsigned char *p1;
    int i;
    char string1[STRING_SIZE];
    char string2[STRING_SIZE];
    char string3[STRING_SIZE];
    char string4[STRING_SIZE];

    if (mode < 0) return 0;

    p1 = code_table_4_5a_location(sec);
    if (p1 == NULL) return 0;

    i=sscanf(arg1,"(%10[^,],%10[^,],%10[^,],%10[^)])", &(string1[0]),&(string2[0]),&(string3[0]),&(string4[0]));
    if (i != 4) {
        fprintf(stderr,"set_lvl1: args %s/%s/%s/%s\n", string1,string2,string3,string4);
        fatal_error("set_lvl1: levels");
    }

    i = atoi(string1);
    if (i < 0 || i > 255) fatal_error("set_lvl1: bad code table 4.5");
    p1[0] = i;

    if (strcmp(string2,"missing") == 0) {
        for (i = 0; i < 5; i++) p1[i+1] = 255;
    }
    else {
        i = atoi(string3);
        int1_char(i, p1+1);
        i = atoi(string4);
        int_char(i, p1+2);
    }
    return 0;
}

/*
 * HEADER:200:set_lvl2:misc:1:X=(type,missing,scale,factor) set level 2 raw format
 */

/**
 * Set level 2 (Code Table 4.5) raw format.
 * 
 * ## Usage
 * -set_lvl2 (type,missing,scale,factor)
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2023
 */
int f_set_lvl2(ARG1) {

    unsigned char *p1;
    int i;
    char string1[STRING_SIZE];
    char string2[STRING_SIZE];
    char string3[STRING_SIZE];
    char string4[STRING_SIZE];

    if (mode < 0) return 0;

    p1 = code_table_4_5b_location(sec);
    if (p1 == NULL) return 0;

    i=sscanf(arg1,"(%10[^,],%10[^,],%10[^,],%10[^)])", &(string1[0]),&(string2[0]),&(string3[0]),&(string4[0]));
    if (i != 4) {
        fprintf(stderr,"set_lvl2: args %s/%s/%s/%s\n", string1,string2,string3,string4);
        fatal_error("set_lvl2: levels");
    }

    i = atoi(string1);
    if (i < 0 || i > 255) fatal_error("set_lvl2: bad code table 4.5");
    p1[0] = i;
    if (strcmp(string2,"missing") == 0) {
        for (i = 0; i < 5; i++) p1[i+1] = 255;
    }
    else {
        i = atoi(string3);
        int1_char(i, p1+1);
        i = atoi(string4);
        int_char(i, p1+2);
    }
    return 0;
}

