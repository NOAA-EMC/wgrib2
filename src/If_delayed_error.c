/** @file
 * @brief Handle delayed errors.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2020
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:if_delayed_error:If:0:if delayed error
 */

/** Run flag for delayed error. */
extern int run_flag;

/** Last message to process. */
extern unsigned int last_message;

/**
 * Wgrib2 v3.0.1 introduces delayed errors. The (-if_delayed_error) allows you to check the delayed error flag and perhaps run a process to fixed the delayed errors. Unlike most -if options, there isn't an equivalent -elseif_delayed_error option. 
 * 
 * If you are interested in writting an "if" option for wgrib2, the source code, If_delayed_error.c, is an ideal example. 
 * 
 * ## Usage
 * -if_delayed_error
 * 
 * Will enter if block if there is a delayed error
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return Always returns 0.
 *
 * @author Wesley Ebisuzaki @date 12/2020
 */
int f_if_delayed_error(ARG0)  {
    if (mode < 0) return 0;
    /* if delayed_error .. run_flag = 1 otherwise 0 */
    run_flag = (last_message >> 1) ? 1 : 0;
    return 0;
}

