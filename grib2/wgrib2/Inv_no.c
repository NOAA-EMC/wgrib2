#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Inv_number.c inventory_number routines
 *
 * inv_number is the line number of the inventory
 *
 * to multitask, you can split up the work by the inv number
 *  note: inv number is not the message number because
 *    1) a grib message can have multiple submessages
 *    2) a match command can select out records
 *       for example, -match :HGT: will only give you the heght fields
 *         and you will want to split the processing over the various height fields
 *    3) the -i command will read an inventory
 *
 * the -n and -for_n are preliminary
 *
 * 5/2009 in public domain Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:n:inv:0:prints out inventory number
 */

extern int inv_no;
extern int match_flag;

int f_n(ARG0)  {
    if (mode >= 0) {
	sprintf(inv_out,"n=%d",inv_no);
    }
    return 0;
}

/*
 * HEADER:100:for_n:setup:1:process inv numbers in range,  X=(start:end:step), only one -for allowed
 */

extern int for_n_mode, for_n_start, for_n_end, for_n_step;

int f_for_n(ARG1)  {
    if (mode == -1) {
        if (for_n_mode == 1) fatal_error("for_n: only one for_n allowed","");
        for_n_mode = 1;
        parse_loop(arg1, &for_n_start, &for_n_end, &for_n_step);
    }
    return 0;
}

/*
 * HEADER:100:if_n:misc:1:if (inv numbers in range),  X=(start:end:step)
 */


int f_if_n(ARG1)  {
    struct local_struct {
        int start, end, step;
    };
    struct local_struct *save;


    if (mode == -1) {

        save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_if_n","");

	parse_loop(arg1, &(save->start), &(save->end), &(save->step));

	*local = save;
	return 0;
    }
    if (mode == -2) {
	free(*local);
	return 0;
    }
    if (mode >= 0) {
	save = (struct local_struct *) *local;
	if (inv_no >= save->start && inv_no <= save->end && ((inv_no - save->start) % save->step) 
		== 0) match_flag=0;
	else match_flag = 1;
        return 0;
    }
    return 0;
}

