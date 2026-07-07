/** @file
 * @brief Most C programs flush the output when the buffers are full.
 * This is usually the efficient method.
 *
 * However, life is different when you start writting to pipe instead of disk files.
 * In this case, you want to flush the output buffers at the end of the write.
 * If you don't flush after every write, the pipe line can stall and never complete.
 * Wgrib2 gets around the stalling pipeline by checking the output files to see
 * whether is a pipe. If so, it turns on the flush mode. This causes a flush
 * after all writes.
 *
 * The automatic detection for flush mode works well. However, it is stll
 * possible to turn on the flush mode manually which was done in the old
 * days.
 * @author Public Domain: Wesley Ebisuzaki @date 07/2006
 */
#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Flush out output flag. */
extern int flush_mode;

/*
 * HEADER:-1:flush:setup:0:flush output buffers after every write (interactive)
 */

/**
 * Flush output buffers after every write (interactive).
 * 
 * When the flush mode is off, output is buffered. Than means the output is saved to a 
 * memory buffer and is only flushed (written out) when the buffer is full or the 
 * program ends. This mode speeds up the output. However, this mode fails when writting 
 * to a pipe or file and another program is reading from that pipe or file while wgrib2 
 * is executing. 
 * 
 * The -flush option causes wgrib2 to flush the output buffers after every write. This 
 * option is now rarely used because wgrib2 internally sets the flush option on when 
 * detects a write to a pipe. The only current need for the -flush option is when another 
 * program is reading the disk file while wgrib2 is writting that file. In this case, you 
 * would use this option to ensure that the disk file is written as soon as possible. 
 * 
 * In systems that do not have a POSIX-compatible stat() function, the flush mode is 
 * turned on. 
 * 
 * ## Usage
 * -flush
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 07/2006
 */
int f_flush(ARG0) {
    flush_mode = 1;
    return 0;
}

/*
 * HEADER:-1:no_flush:setup:0:flush output buffers when full (default)
 */

/**
 * Flush output buffers when full (default).
 * 
 * This option causes wgrib2 to flush the output buffers when the buffers are full or 
 * the program ends. This is the opposite of the The -flush option. 
 * 
 * The only practical use of this option would be in a non-POSIX system where the flush 
 * mode is turned on, you are not using output pipes, and you wanted to speed up the output. 
 * For this option to be effective, the option has to be the last option that uses a file. 
 * (Opening a file can update the flush mode.) 
 * 
 * ## Usage
 * -no_flush
 * 
 * This should be the last option on the command line.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 07/2006
 */
int f_no_flush(ARG0) {
    flush_mode = 0;
    return 0;
}
