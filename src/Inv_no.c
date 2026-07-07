/** @file
 * @brief Inventory number routines.
 * 
 * inv_number is the line number of the inventory
 *
 * To multitask, you can split up the work by the inv number.
 *
 * Note: inv number is not the message number because
 *    1) a grib message can have multiple submessages
 *    2) a match command can select out records
 *       for example, -match :HGT: will only give you the heght fields
 *         and you will want to split the processing over the various height fields
 *    3) the -i command will read an inventory
 *
 * the -n and -for_n are preliminary
 * @author Public Domain: Wesley Ebisuzaki @date 5/2009
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:n:inv:0:prints out inventory number
 */

/** Inventory number */
extern int inv_no;

/** Run flag. */
extern int run_flag;

/**
 * Prints out inventory number.
 *
 * ## Message Number vs Inventory Number
 * A grib file is made of messages and each message can contain one or more gridded fields. 
 * If the message has two or more gridded fields, we say the message has submessages.
 * 
 * ### Message/submessage numbering convention
 * 
 * No submessage, one field:        I
 * 
 * I is integer starting from 1 and is the message number
 * 
 * Submessage, multiple fields:     I.J
 * 
 * I is integer starting from 1 and is the message number
 * J is integer and is the sub-message number
 * J = 1 for the first submessage of the Ith message
 * 
 * ### Inventory numbering convention
 * 
 * I - I is integer starting from 1 and is the number of the field
 * 
 * An alternative method for numbering the fields is the inventory number. The inventory number 
 * is simply the grids starting from one. The inventory number is simply the line number of the 
 * default wgrib2 inventory. The message number is the first column of the the default wgrib2 
 * inventory. 
 * 
 * Both the message number and the inventory number are valid ways of numbering the fields in a 
 * grib file. The message number is the original method and reflects the structure of the grib 
 * message/submessage structure. The inventory number is a logical numbering scheme when trying 
 * to multitask wgrib2. (For example, send even fields to CPU1 and odd fields to CPU2.) 
 * 
 * ## Usage
 * -n
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 *
 * @author Wesley Ebisuzaki @date 5/2009
 */
int f_n(ARG0)  {
    if (mode >= 0) {
        sprintf(inv_out,"n=%d",inv_no);
    }
    return 0;
}

/*
 * HEADER:100:for_n:setup:1:process inv numbers in range,  X=(start:end:step), only one -for allowed
 */

/** For loop n mode flag. */
extern int for_n_mode;

/** For loop n start value. */
extern int for_n_start;

/** For loop n end value. */
extern int for_n_end;

/** For loop n step value. */
extern int for_n_step;

/**
 * Process inventory numbers in range. Only one -for allowed.
 * 
 * A grib file has set of grids and you can reference the field by its grib message number 
 * and submessage number. You can also reference the field by its inventory number which 
 * starts at 1. The -for_n option allows you to process a subset of the grib file using a 
 * for-loop syntax on the inventory number. Suppose you want to process grids 10 through 20 
 * by 2, you can add the option -for_n 10:20:2. 
 * 
 * ## Usage
 * -for_n I:J:K        same as for n = I to J by K
 * -for_n I:J          same as for n = I to J by 1
 * -for_n I::K         same as for n = I to MAX_INTEGER by K
 * -for_n I            same as for n = I to MAX_INTEGER by 1
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
 * @author Wesley Ebisuzaki @date 5/2009
 */
int f_for_n(ARG1)  {
    if (mode == -1) {
        if (for_n_mode == 1) fatal_error("for_n: only one for_n allowed","");
        for_n_mode = 1;
        parse_loop(arg1, &for_n_start, &for_n_end, &for_n_step);
    }
    return 0;
}

/*
 * HEADER:100:if_n:If:1:if (inv numbers in range),  X=(start:end:step)
 */

/**
 * An -if option for a range of inventory numbers.
 * 
 * The -if_n option uses the same syntax as the "for_n" option. If you want to operate on 
 * inventory records 10 to 20, you would use the parameter 10:20. If you want to operate on 
 * all the even inventory records from 10 to 20, you would use 10:20:2. The restrictions are 
 * the start value must be less than the ending value and the step has to be greater than zero. 
 *
 * The -if_n option is similar to the -for_n option in that they both select a range of inventory 
 * records. The difference is that the -for_n option selects the range of records that wgrib2 
 * will process. With the the -if_n option, all the records will be processed but only the additional 
 * processing within the -if block will be only done for the selected records. The -for_n option can 
 * be much faster if the field has to be decoded. With the -for_n option, only the selected records 
 * need to be decoded whereas all the records would be decoded when using the -if_n option. The command 
 * line, however, can have multiple -if_n options.
 *
 * ## Usage
 * -if_n I:J:K        same as for n = I to J by K
 * -if_n I:J          same as for n = I to J by 1
 * -if_n I::K         same as for n = I to MAX_INTEGER by K
 * -if_n I            same as for n = I to MAX_INTEGER by 1
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
 * @author Wesley Ebisuzaki @date 5/2009
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
            == 0) run_flag=1;
        else run_flag = 0;
        return 0;
    }
    return 0;
}

