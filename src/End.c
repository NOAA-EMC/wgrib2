/** @file
 * @brief Stops the processing of the grib file.
 * @author Public Domain: Wesley Ebisuzaki @date 02/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:end:misc:0:stop after first (sub)message (save time)
 */

/** Last message to process. */
extern unsigned int last_message;

/**
 * Stops the processing of the grib file after one line of the inventory has been written. 
 * This option is designed to improve speed when used with the -match option.
 * 
 * ## Usage:
 * -end
 * 
 * @param ARG0 Arguments and context for the wgrib2 function macro.
 * 
 * @return 0 on success.
 * 
 * ## Example:
 * ???
 * 
 * @author Wesley Ebisuzaki @date 02/2008
 */
int f_end(ARG0) {
    // if last message is already set,  don't change error code
    if (mode >= 0 && last_message == 0) last_message = 1;
    return 0;
}

