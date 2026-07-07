/** @file
 * @brief Process submessages.
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */
#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Submessage to process. */
extern int only_submsg;

/*
 * HEADER:100:submsg:misc:1:process submessage X (0=process all messages)
 */

/**
 * Process by submessage number.
 * 
 * Usually wgrib2 ignores the distinction between messages and submessages; wgrib2 treats submessages like messages except that the "message number" in the inventory is replaced by a "message number.submessage number" in the inventory. If you want to work with submessages, the following options are available.
 * 
 * -GRIB FILE : copy a message to FILE
 * -ncep_uv FILE : combine U,V into a message like in NCEP operations
 * -tosubmsg FILE : create a file with submessages
 * -submsg N : process by submessage number
 * 
 * The -submsg N option allows to process by submessage number. If the N is zero, all the submessages are processed which pretty pointless as this the default operation. If N is one, then all the messages are processed once. (Messages with only one field are considered to have one submessage.) The following will copy from IN.grb to OUT.grb and preserve the submessage structure. 
 * 
 * @code{.sh}
 * wgrib2 IN.grb -submsg 1 -GRIB OUT.grb
 * @endcode{}
 * 
 * This will copy all the 200 mb fields assuming U/V are in the same message and keep U/V in the same message. 
 * 
 * ## Usage
 * -submsg N
 * 
 * N is an integer, usually 1
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_submsg(ARG1) {

    only_submsg = atoi(arg1);

    return 0;
}
