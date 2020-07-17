#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"

/*
 * save_string.c 2012 Public domain Wesley Ebisuzaki
 *
 * when you read a table from a file, you want to setup structures
 *
 * strings are a problem because the size chaanges
 * this package allows you to save a string in memory
 * 
 * 3/2012: W. Ebisuzaki
 */

static int csize = 0;
static int csize_max = 0;
static char *cbuffer = NULL;

char *save_string(char *string) {
    int len;
    char *cbuffer_old;

    len = strlen(string);

    while (len + csize + 1 > csize_max) {
        /*  need to add more memory */
        if (csize_max == 0) {
	    csize_max = EXT_TABLE_SIZE;
            cbuffer = malloc(csize_max);
        }
	else {
	    csize_max += csize_max;
	    cbuffer = realloc(cbuffer_old=cbuffer, csize_max);
	    if (cbuffer == NULL) {
	        free(cbuffer_old);
	        fatal_error_i("save_string: memory allocation failed, %d bytes", csize);
	    }
	}
    }
    strncpy(cbuffer+csize,string,len+1);
    csize += len + 1;
    return cbuffer + csize - len - 1;
}
