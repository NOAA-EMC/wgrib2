#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * if_rec start:end:step
 *
 * like if_n but for record numbers
 *
 * 8/2010 in public domain Wesley Ebisuzaki
 */

/*
 * HEADER:100:if_rec:If:1:if (record numbers in range),  X=(start:end:step)
 */

extern int msg_no, run_flag;

int f_if_rec(ARG1)  {
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
	if (msg_no >= save->start && msg_no <= save->end && ((msg_no - save->start) % save->step) 
		== 0) run_flag=1;
	else run_flag = 0;
        return 0;
    }
    return 0;
}

