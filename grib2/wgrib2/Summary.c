#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Summary.c
 *
 * options that produce output at the end of the job (mode == 2)
 * they can provide a summary of the operations
 *
 * 10/2008 Public Domain Wesley Ebisuzaki
 * 1/2011 chagned new_GDS by GDS_chagne_no WNE
 *
 * count: writes to stdout the number of records processed
 * grid_changes: checks to see that only 1 grid type was processed
 *
 */



extern enum input_type input;
extern int header, dump_rec, dump_submsg;

extern int file_append;

extern int GDS_change_no;

/*
 * HEADER:100:count:misc:0:prints count, number times this -count was processed
 */
int f_count(ARG0) {
    struct local_struct {
        int count;
    };
    struct local_struct *save;

    if (mode == -1) {
        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("f_count memory allocation ","");
	save->count = 0;
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
	save->count += 1;
    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
	sprintf(inv_out,"number of records: %d", save->count);
	free(save);
    }
    return 0;
}

/*
 * HEADER:100:grid_changes:misc:0:prints number of grid changes
 */
int f_grid_changes(ARG0) {
    if (mode == -2) {
	switch(GDS_change_no) {
	case 0: sprintf(inv_out,"Warning: no grib2 records");
		break;
	case 1: sprintf(inv_out,"Good: only one grid");
		break;
	default: sprintf(inv_out,"Warning: muliple grids, %d changes", GDS_change_no);
	}
    }
    return 0;
}
/*
 * HEADER:100:error_final:misc:3:error if at end X=count Y=ne,eq,le,lt,gt,ge Z=integer 
 */

int f_error_final(ARG3) {
    int error,i;
    struct local_struct {
        int op, val;
        int count;
    };
    struct local_struct *save;

    error = 0;
    if (mode == -1) {
	if (strcmp(arg1,"count")) fatal_error("error_final: X=%s is not allowed",arg1);
	i = 0;
	if (strcmp(arg2,"ne") == 0) {
	    i = 0;
	}
	else if (strcmp(arg2,"eq") == 0) {
	    i = 1;
	}
	else if (strcmp(arg2,"lt") == 0) {
	    i = 2;
	}
	else if (strcmp(arg2,"le") == 0) {
	    i = 3;
	}
	else if (strcmp(arg2,"gt") == 0) {
	    i = 4;
	}
	else if (strcmp(arg2,"ge") == 0) {
	    i = 5;
	}
	else fatal_error("error_final: Y=%s is not allowed",arg2);

        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("error_final memory allocation ","");
	save->count = 0;
	save->op = i;
	save->val = atoi(arg3);
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
	save->count += 1;
    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
	error = save->count != atoi(arg1);
	switch(save->op) {
	    case 0:	error = save->count != save->val; break;
	    case 1:	error = save->count == save->val; break;
	    case 2:	error = save->count <  save->val; break;
	    case 3:	error = save->count <= save->val; break;
	    case 4:	error = save->count >  save->val; break;
	    case 5:	error = save->count >= save->val; break;
	}
	free(save);
    }
    return error;
}
