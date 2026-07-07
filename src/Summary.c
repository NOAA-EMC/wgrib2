/** @file
 * @brief Options that produce output at the end of the job (mode == 2). They can provide a 
 * summary of the operations.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 10/2008 | W. Ebisuzaki | Initial
 * 1/2011  | W. Ebisuzaki | Changed new_GDS by GDS_change_no
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Current input type (inv, dump, or all). */
extern enum input_type input;

/** File header flag. */
extern int header;

/** Dump record flag. */
extern int dump_rec;

/** Dump submessage flag. */
extern int dump_submsg;

/** Append grib file flag. */
extern int file_append;

/** GDS change number. */
extern int GDS_change_no;

/*
 * HEADER:100:count:misc:0:prints count, number times this -count was processed
 */

/**
 * Prints the number of records processed.
 * 
 * The -count option just counts the number of fields. It is very similar to the piping the 
 * output of wgrib to the "wc -l". The advantage of using -count is that the inventory is 
 * displayed and the count goes to stderr. By retaining stderr, one could make sure the 
 * expected number of records were processed. 
 * 
 * ## Usage
 * -count
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
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

/**
 * Checks to see that only 1 grid type was processed,
 * 
 * The -grid_changes option is a safety option. Normally we don't expect the grid to change 
 * within a grib file, and many programs would fail if the grid did change. Wgrib2 will work 
 * if the grid changes but it would work much slower. (Each grid change would require a 
 * recalculation of the grid parameters such as the the grid point locations if needed.) The 
 * -grid_changes option prints to stderr, the number of times the grid changed during processing 
 * of the file. Only grib (sub-)messages that were processed and not skipped by a -match or 
 * similar option will count. 
 * 
 * ## Usage
 * -grid_changes
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
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

/**
 * Tests the final value and can raise an error condition when wgrib2 returns. For version 1, 
 * the only value that can be tested is the count of the number time the option is called in 
 * the grib-processing phase. 
 * 
 * When -error_final is intialized, the count is set to zero. Then -error_final increments the 
 * count whenever it processes a grib message. Finally when -error_final is run after processing 
 * all the grib messages, it tests count to a specified integer and sets the error return. 
 * 
 * ## Usage
 * -error_final count TEST INTEGER
 * 
 * TEST = eq, ne, lt, gt, le, ge
 * INTEGER = value to compare to count
 * 
 * Sets return error to one if "count TEST INTEGER" is true.
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
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
