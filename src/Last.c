/** @file
 * @brief Write last inventory item to file.
 * @author Public Domain: Wesley Ebisuzaki @date 2015
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Append grib file flag. */
extern int file_append;

/** Pointer to last inventory output string. */
extern char *last_inv_out;

/*
 * HEADER:100:last:inv_output:1:write last inv item to file X
 */

/**
 * Write last inventory item to specified output file.
 * 
 * The -last FILE option writes the results of the previous option to FILE. The FILE can be a 
 * disk file, temporary file or memory file. If the -last option preceeds any inventory options, 
 * then the "grib message number[.submessage number:byte location" will be written to the file. 
 * 
 * The -last option was designed for callable wgrib2 to obtain inventory information. Note the 
 * -s_out FILE option should be replaced by the more powerful -s -last FILE syntax. 
 * 
 * The -last option does not write to the inventory, so if you have two consecutive -last options, 
 * the second -last will have no output. With wgrib2 v3.0.0+, the -last and -last0 options will not 
 * clear the last option output buffer. So the the second -last option will have the same output 
 * as the immediately preceeding -last or -last0 option. 
 * 
 * ## Usage
 * -last FILE
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2015
 */
int f_last(ARG1) {
    struct seq_file *save;
    size_t i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("last: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "a" : "w") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = *local;
        i = strlen(last_inv_out);
        fwrite_file(last_inv_out, sizeof(char), i, save);	/* i+1 .. for \0 */
        repeat_inv_out();
    }
    return 0;
}

/*
 * HEADER:100:last0:inv_output:1:write last inv item to beginning of file X
 */

/**
 * Write last inventory item to beginning of specified output file.
 * 
 * The -last0 FILE option writes the results of the previous option to the beginning of FILE. 
 * The FILE can be a disk file, temporary file or memory file. If the -last option preceeds 
 * any inventory options, then the "grib message number[.submessage number:byte location" 
 * will be written to the file. 
 * 
 * The -last0 option was designed for callable wgrib2. This allows calls to wgrib2 to obtain 
 * inventory information. Note the -s_out FILE option should be replaced by the more powerful 
 * -s -last FILE syntax. 
 * 
 * The -last0 option does not write to the inventory, so if you have two consecutive -last0 
 * options, the second -last0 will have zero output. With wgrib2 v3.0.0+, the -last and -last0 
 * options will not clear the last option output buffer. So the the second -last0 option will 
 * have the same output as the immediately preceeding -last or -last0 option. 
 * 
 * ## Usage
 * -last0 FILE
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2015
 */
int f_last0(ARG1) {
    struct seq_file *save;
    size_t i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("last: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "a" : "w") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = *local;
        i = strlen(last_inv_out);
        fseek_file(save, 0L, SEEK_SET);
        fwrite_file(last_inv_out, sizeof(char), i, save);	/* i+1 .. for \0 */
        repeat_inv_out();
    }
    return 0;
}
