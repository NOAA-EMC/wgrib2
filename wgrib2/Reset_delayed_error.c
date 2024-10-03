#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Reset_delayed_error.c      12/2020 Public Domain   Wesley Ebisuzaki
 *
 * wgrib2 v3.0.1 adds delayed fatal errors,
 * This allows the user to examine the grib message that caused the fatal
 * error.  This is useful for finding the problem in the grib message.
 *
 * -reset_delayed_error  allows you to reset the delayed error flag
 *  and continue processing.  Note that this may cause the because
 *  the processing of bad grib files could cause seg faults.
 *  For example, Section 3 is the wrong size, and you process the
 *  file as if section 3 is the right size.
 */

extern unsigned int last_message;

/*
 * HEADER:100:reset_delayed_error:inv:0:clear reset_delayed_error flag
 */
int f_reset_delayed_error(ARG0) {
    if (mode >= 0) {
	// sprintf(inv_out,"delayed_error=%u", (last_message >> 1) << 1);
	last_message &= DELAYED_NONERROR_END;
    }
    return 0;
}
