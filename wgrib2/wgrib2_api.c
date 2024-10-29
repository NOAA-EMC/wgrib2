#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "wgrib2.h"


/* 
   sort of like the command line version of wgrib2

   int wgrib2a(string1, string2. .. stringN, NULL);
 */

int wgrib2a(char *arg1, ...) {
    va_list valist;

    int count, i;
    const char *targ, **argv;

    /* count number of arguments */
    count = 0;
    va_start(valist, arg1);
    targ = arg1;
    while (targ != NULL) {
	count++;
	targ = va_arg(valist, const char *);
    }
    va_end(valist);

    /* setup argv */
    argv = (const char **) malloc( (count + 2) * sizeof(const char *));
    argv[0] = "wgrib2a_c_api";
    argv[1] = arg1;
    va_start(valist, arg1);
    for (i=2; i <= count; i++) {
	argv[i] = va_arg(valist, const char *);
    }
    va_end(valist);
    argv[count+1] = NULL;

    i = wgrib2(count+1, argv);
    free(argv);
    return i;
}

