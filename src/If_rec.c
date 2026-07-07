/** @file
 * @brief Handle record number conditions.
 * 
 * Like if_n, but for record numbers
 * @author Public Domain: Wesley Ebisuzaki @date 08/2010
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:if_rec:If:1:if (record numbers in range),  X=(start:end:step)
 */

/** Message number. */
extern int msg_no;

/** Run flag for record number conditions. */
extern int run_flag;

/**
 * This option is an -if for a range of record numbers. The -if_rec option uses the same syntax 
 * as the "for" option. If you want to operate on records 10 to 20, you would use the parameter 
 * 10:20. If you want to operate on all the even records from 10 to 20, you would use 10:20:2. 
 * The restrictions are the start value must be less than the ending value and the step has to be 
 * greater than zero. 
 * 
 * The -if_rec option is similar to the -for option in that they both select a range of records. 
 * The difference is that the -for option selects the range of records that wgrib2 will process. 
 * With the the -if_rec option, all the records will be processed but only the additional processing 
 * within the -if block will be only done for the selected records. The -for option can be much faster 
 * if the field has to be decoded. With the -for option, only the selected records need to be docoded 
 * whereas all the records would be decoded when using the -if_rec option. The command line, however, 
 * can have multiple -if_rec options. 
 * 
 * ## Usage
 * -if_rec I:J:K        same as for n = I to J by K
 * -if_rec I:J          same as for n = I to J by 1
 * -if_rec I::K         same as for n = I to MAX_INTEGER by K
 * -if_rec I            same as for n = I to MAX_INTEGER by 1
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 08/2010
 */
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

