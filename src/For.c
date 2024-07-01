#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * for: allows you set up a for-loop for records to process
 *
 * for example only want to do the first 100 records
 *
 * -for 1:100
 *
 * 10/2008 in public domain Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:for:setup:1:process record numbers in range,  X=(start:end:step), only one -for allowed
 */

extern int for_mode, for_start, for_end, for_step;

int f_for(ARG1)  {
    if (mode == -1) {
	if (for_mode == 1) fatal_error("for: only one for allow","");
        for_mode = 1;
        parse_loop(arg1, &for_start, &for_end, &for_step);
    }
    return 0;
}
