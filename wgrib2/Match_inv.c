#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Match_inv.c                        10/2024 Public Domain Wesley Ebisuzaki
 *
 * Various text tests (ex. -if, -not) compare a string to the match inventory.
 * The match inventory is larger and has more items than the normal inventory.
 *
 * Since the match inventory cannot include everything, the option has
 * been added to add extra data to the match inventory.
 *
 * 2/2025: didn't finialize for functions with 2 args
 *         C23 needs prototypes
 */

int (*match_extra_fn[MATCH_EXTRA_FN])(ARG8);
void *match_extra_fn_local[MATCH_EXTRA_FN];
int match_extra_fn_nargs[MATCH_EXTRA_FN];
char *match_extra_fn_arg1[MATCH_EXTRA_FN] = { NULL } ;
char *match_extra_fn_arg2[MATCH_EXTRA_FN] = { NULL } ;
int match_extra_fn_n;

extern int warn_nonzero_min_sec;

/*
 * HEADER:100:match_inv_add:setup:3:add new options to match_inventory
 */
int f_match_inv_add(ARG3) {
    int i, j;
    if (mode == -1) {	/* find fn(..) based on name, add to list */
	if (match_extra_fn_n == MATCH_EXTRA_FN) fatal_error("match_inv_add: table full by %s",arg1);

	/* if arg1 == -abc  remove - */
	if (arg1[0] == '-') arg1++;

	/* search for option in table */
        for (j = 0; j < nfunctions; j++) {
            if (strcmp(arg1,functions[j].name) == 0) break;
        }
	if (j == nfunctions) fatal_error("match_inv_add: %s not found", arg1);

	/* add fn(..) to table of extra functions for match_inv */
        match_extra_fn[match_extra_fn_n] = functions[j].fn;

	/* add args to table and intialize fn() */
        if (functions[j].nargs == 0 && functions[j].type == inv) {
	    // initialize fn(..)
            match_extra_fn[match_extra_fn_n](init_ARG0(inv_out, &(match_extra_fn_local[match_extra_fn_n])));
        }
        else if (functions[j].nargs == 1 && functions[j].type == inv) {
	    // initialize fn(..)
            match_extra_fn[match_extra_fn_n](init_ARG1(inv_out, &(match_extra_fn_local[match_extra_fn_n]),arg1));
            i = strlen(arg1) + 1;
            if ((match_extra_fn_arg1[match_extra_fn_n] = (char *) malloc(i)) == NULL) 
		fatal_error("match_inv_add: memory allocation","");
            strncpy(match_extra_fn_arg1[match_extra_fn_n], arg1, i);
        }
        else if (functions[j].nargs == 2 && functions[j].type == inv) {
	    // initialize fn(..)
            match_extra_fn[match_extra_fn_n](init_ARG2(inv_out, &(match_extra_fn_local[match_extra_fn_n]),arg1,arg2));
            i = strlen(arg1) + 1;
            if ((match_extra_fn_arg1[match_extra_fn_n] = (char *) malloc(i)) == NULL) 
		fatal_error("match_inv_add: memory allocation","");
            strncpy(match_extra_fn_arg1[match_extra_fn_n], arg1, i);
            i = strlen(arg2) + 1;
            if ((match_extra_fn_arg2[match_extra_fn_n] = (char *) malloc(i)) == NULL) 
		fatal_error("match_inv_add: memory allocation","");
            strncpy(match_extra_fn_arg2[match_extra_fn_n], arg2, i);
        }
	else fatal_error("match_inv_add: %s not inv option or multple args",arg1);
	match_extra_fn_nargs[match_extra_fn_n] = functions[j].nargs;
        match_extra_fn_n++;
    }
    else if (mode == -2) {	/* finalize all the functions */
	for (j = 0; j < match_extra_fn_n; j++) {
	    if (match_extra_fn_nargs[j] == 0)
                match_extra_fn[j](fin_ARG0(inv_out, &(match_extra_fn_local[j])));
	    else if (match_extra_fn_nargs[j] == 1)
                match_extra_fn[j](fin_ARG1(inv_out, &(match_extra_fn_local[j]), match_extra_fn_arg1[j] ));
	    else if (match_extra_fn_nargs[j] == 2)
                match_extra_fn[j](fin_ARG2(inv_out, &(match_extra_fn_local[j]), match_extra_fn_arg1[j], 
                     match_extra_fn_arg2[j] ));
	}
	for (j = 0; j < match_extra_fn_n; j++) {
	    if (match_extra_fn_arg1[j] != NULL) {
		free(match_extra_fn_arg1[j]);
	        match_extra_fn_arg1[j] = NULL;
	    }
	    if (match_extra_fn_arg2[j] != NULL) {
		free(match_extra_fn_arg2[j]);
	        match_extra_fn_arg2[j] = NULL;
	    }
	}
        match_extra_fn_n = 0;
    }
    return 0;
}


extern const char *item_deliminator;

/*
 * HEADER:100:match_inv:inv:0:inventory used by -match, -not, -if and -not_if
 */

/*
 * this is a simple macro .. see how easy it is!
 * would be more complicated if functions used static variables
 * minor complication if need to set decode or latlon flags
 *
 * note: match_inv should be a superset of -s
 * makes life easier for users of wgrib2api
 */

extern unsigned int type_ext_name;
extern int ftime_mode;

#define SHORT_DATECODE 0
#define LONG_DATECODE 1

int f_match_inv(ARG0) {
   if (mode < 0) {
	warn_nonzero_min_sec = 0;
	return 0;
    }
    return match_inv(SHORT_DATECODE,call_ARG0(inv_out,local));
}



/*
 * HEADER:100:Match_inv:inv:0:same as -match_inv except d=YYYYMMDDHH <-> D=YYYYMMDDHHmmss
 */
int f_Match_inv(ARG0) {
    if (mode < 0) {
	warn_nonzero_min_sec = 0;
	return 0;
    }
    return match_inv(LONG_DATECODE,call_ARG0(inv_out,local));
}


int match_inv(int type_datecode, ARG0) {
    int old_mode, j, old_ftime_mode;
    if (mode >= 0) {
        old_mode = mode;
	mode = 0;

	if (type_datecode == SHORT_DATECODE) f_t(call_ARG0(inv_out,NULL));
	else f_T(call_ARG0(inv_out,NULL));
	
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

        if (type_ext_name) f_ext_name(call_ARG0(inv_out,NULL));
	else f_var(call_ARG0(inv_out,NULL));
	
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

        f_lev(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

        f_ftime(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	f_misc(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 11/2010 */
	if (type_ext_name == 0) {
	    f_ext_name(call_ARG0(inv_out,NULL));
            strcat(inv_out,":");
            inv_out += strlen(inv_out);
	}

	/* added 4/2011 */
	f_n(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 1/2014 */
	f_npts(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 1/2015 */
        f_varX(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 2/2015 */
        f_pdt(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 1/2015 */
	if (type_datecode == SHORT_DATECODE) f_T(call_ARG0(inv_out,NULL));
	else f_t(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 1/2015 */
        f_start_FT(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 1/2015 */
        f_end_FT(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 7/2017 */
        f_scaling(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

	/* added 8/2017 */
        for (j = 0; j < match_extra_fn_n; j++) {
	    if (match_extra_fn_nargs[j] == 0) 
                match_extra_fn[j](call_ARG0(inv_out, &(match_extra_fn_local[j])));
	    else if (match_extra_fn_nargs[j] == 1) 
                match_extra_fn[j](call_ARG1(inv_out, &(match_extra_fn_local[j]),match_extra_fn_arg1[j]));
	    else if (match_extra_fn_nargs[j] == 2) 
                match_extra_fn[j](call_ARG2(inv_out, &(match_extra_fn_local[j]), match_extra_fn_arg1[j], match_extra_fn_arg2[j]));
            strcat(inv_out,":");
            inv_out += strlen(inv_out);
	}

	/* added 1/2023 */
	old_ftime_mode = ftime_mode;
	ftime_mode = ftime_mode == 0 ? 1 : 0;
	f_ftime(call_ARG0(inv_out,NULL));
	strcat(inv_out,":");
	inv_out += strlen(inv_out);
	ftime_mode = old_ftime_mode;

        f_vt(call_ARG0(inv_out,NULL));
        strcat(inv_out,":");
        inv_out += strlen(inv_out);

        mode = old_mode;
    }
    return 0;
}
