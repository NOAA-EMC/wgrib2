#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 4/2008 public domain Wesley Ebisuzaki
 */

/*
 * HEADER:100:h:misc:0:help, shows common options
 */
int f_h(ARG0) {
    const char *arg1 = "most";
    mode = -1;
    f_help(call_ARG1(inv_out, NULL, arg1)); 
    // printf("%s\n", inv_out);
    err_bin(1); err_string(1);
    return 1;
}

/*
 * like strstr but ignore case
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

        if (functions[i].type == output)         	sprintf(l," out  ");
        else if (functions[i].type == inv)       	sprintf(l," inv  ");
        else if (functions[i].type == misc)      	sprintf(l," misc ");
        else if (functions[i].type == setup)     	sprintf(l," init ");
        else if (functions[i].type == inv_output)       sprintf(l," inv> ");
        else                                     sprintf(l," ???  ");
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
