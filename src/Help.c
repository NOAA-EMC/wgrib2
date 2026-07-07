/** @file
 * @brief Routines to print wgrib2 options and version.
 * @author Public Domain: Wesley Ebisuzaki @date 4/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:h:misc:0:help, shows common options
 */

/**
 * Prints out list of common options.
 * 
 * ## Usage
 * -h
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 1.
 * 
 * @author Wesley Ebisuzaki @date 4/2008
 */
int f_h(ARG0) {
    const char *arg1 = "most";
    mode = -1;
    f_help(call_ARG1(inv_out, NULL, arg1)); 
    // printf("%s\n", inv_out);
    err_bin(1);
    err_string(1);
    return 1;
}

/**
 * Like C standard library function strstr(), but ignore case.
 * 
 * @param s Pointer to null-terminated string to be searched.
 * @param t Pointer to null-terminated substring to search for.
 * 
 * @return Pointer to the first character of the first occurrence of t in s, or NULL if not found.
 *
 * @author Wesley Ebisuzaki @date 4/2008
 */
const char *nc_strstr(const char *s, const char *t) {
    int ns, nt, i, j, t0;

    ns = strlen(s);
    nt = strlen(t);
    t0 = tolower((unsigned char) t[0]);

    for (i = 0; i < ns - nt; i++) {
        if (tolower((unsigned char) s[i]) == t0) {
            if (nt == 1) return s+i;
            for (j = 1; j < nt; j++) {
                if (tolower((unsigned char) s[i+j]) != tolower((unsigned char) t[j])) break;
            }
            if (j == nt) return s+i;
        }
    }
    return NULL;
}


/*
 * HEADER:100:help:misc:1:help [search string|all], -help all, shows all options
 */

/**
 * Prints out list of all options that match a query.
 * 
 * ## Usage
 * -help [search string]
 *
 * Will print out options matching the search string.
 * 
 * -help all
 *
 * Will print out all options.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 1.
 * 
 * @author Wesley Ebisuzaki @date 4/2008
 */
int f_help(ARG1) {

    int i, j, all, most, count;
    char *l;
    const char *str;
    str = arg1;

    count = 0;
    most = strcmp(str,"most") == 0;
    all = strcmp(str,"all") == 0;
    
    sprintf(inv_out, "wgrib2 " WGRIB2_VERSION "\n   " BUILD_COMMENTS "\n");
    inv_out += strlen(inv_out);
    for (i = 0; i < nfunctions; i++) {
        /* do no list sort -1 for "most" lists */
        if (most && functions[i].sort == -1) continue;

        l = inv_out;
        sprintf(l," -%s", functions[i].name);
        l += strlen(l);

        j = HELP_NAME_LEN - strlen(functions[i].name);
        j = j < 0 ? 0 : j;
        while (j--) *l++ = ' ';

        if (functions[i].type == output)         	sprintf(l," out   ");
        else if (functions[i].type == inv)       	sprintf(l," inv   ");
        else if (functions[i].type == misc)      	sprintf(l," misc  ");
        else if (functions[i].type == setup)     	sprintf(l," init  ");
        else if (functions[i].type == inv_output)       sprintf(l," inv>  ");
        else if (functions[i].type == If)               sprintf(l," if    ");
        else if (functions[i].type == Else)             sprintf(l," else  ");
        else if (functions[i].type == Elseif)           sprintf(l," elif  ");
        else if (functions[i].type == Endif)            sprintf(l," endif ");
        else                                     sprintf(l," ???   ");
        l += strlen(l);

        switch(functions[i].nargs) {
            case 0:  sprintf(l,"       %s\n", functions[i].desc);
                    break;
            case 1:  sprintf(l,"X      %s\n", functions[i].desc);
                    break;
            case 2:  sprintf(l,"X Y    %s\n", functions[i].desc);
                    break;
            case 3:  sprintf(l,"X Y Z  %s\n", functions[i].desc);
                    break;
            case 4:  sprintf(l,"X..Z,A %s\n", functions[i].desc);
                    break;
            default: sprintf(l,"%d args %s\n", functions[i].nargs,functions[i].desc);
                    break;
        }
        l += strlen(l);
        if (most || all || (nc_strstr(inv_out, str) != NULL)) {
            inv_out = l;
            count++;
        }
        else {
            *inv_out = 0;
        }
    }
    if (count == 0) strcat(inv_out," search failed\n");
    return 1;
}

/*
 * HEADER:100:version:misc:0:print version
 */

int f_version(ARG0) {
    if (mode != -2) {
        sprintf(inv_out, "%s\n", WGRIB2_VERSION);
    }
    return 1;
}
