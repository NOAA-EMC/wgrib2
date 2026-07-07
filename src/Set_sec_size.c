/** @file
 * @brief Set the size of a section. Keeps old data and new data is set to 255 (missing).
 * @author Public Domain: Wesley Ebisuzaki @date 5/2011
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:set_sec_size:misc:2:resizes section , X=section number, Y=size in octets, DANGEROUS
 */

/**
 * Sets the size of a section. This option is DANGEROUS.
 * 
 * Grib messages are made of sections where the section number varies from 0 to 8. Suppose you 
 * want to modify a grib message by changing section 4 (Product Definition Section). You would 
 * use -set_sec_size to change the size of section 4 if necessary and then use -set_byte to 
 * change the contents of your new section 4. This is not pretty but when testing a new template, 
 * you have to do ugly things. 
 * 
 * After you have altered the grib message, you can save the message by either -grib or the -grib_out. 
 * You need to use the latter option if the grid values were altered because the data section needs 
 * to be updated. 
 * 
 * ## Usage
 * -set_sec_size SECTION SIZE
 * 
 * SECTION = section number (0 to 8)
 * SIZE = integer, size of new section. 
 * 
 * SIZE can be zero for a missing section. Generally, size is greater than 5.
 * 
 * ## Results
 * 
 * The -set_sec_size option expands or contracts a section. For expansions, the new octets are 
 * filled with 255. This option puts the size of the new section in octets 1..4 and the section 
 * number in octet 5 assuming the size is greater than equal to 5. Some sections can be missing. 
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2011
 */
int f_set_sec_size(ARG2) {

    int section, n, old_size;
    int i, j;
    static unsigned char *new_sec[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} ;

    section = atoi(arg1);
    n = atoi(arg2);

    if (mode == -1) {	/* error checking */
        if (section < 0 || section > 8) fatal_error_i("set_sec_size: bad section %d", section);
        if (n < 0) fatal_error_i("set_sec_size: number of octets must be >= 0 not %d",n);
        if (section == 0 && n != 16) fatal_error("set_sec_size: sec 0 must be 16 octets","");
        if (section == 8 && n != 4) fatal_error("set_sec_size: sec 8 must be 4 octets","");
    }
    if (mode < 0) return 0;

    if (sec[section] == NULL) old_size = 0;
    else if (section == 0) old_size = 16;
    else if (section == 8) old_size = 4;
    else old_size = uint4(sec[section]);

    if (new_sec[section] != NULL) {
        free(new_sec[section]);
        new_sec[section] = NULL;
    }

    if (n == 0) {
        sec[section] =  NULL;
        return 0;
    }

    new_sec[section] = (unsigned char *) malloc(n * sizeof(unsigned char));
    if (new_sec[section] == NULL) fatal_error("set_sec_size: failed memory alloction","");

    for (i = 0; i < n; i++) new_sec[section][i] = 255;

    j = (old_size < n) ? old_size : n;
    for (i = 0; i < j; i++) new_sec[section][i] = sec[section][i];

    if (section >= 2 && section <= 7 && n >= 5) {
        uint_char(n, new_sec[section]);
        new_sec[section][4] = (unsigned char) section;
    }
    sec[section] = new_sec[section];
    return 0;
}
