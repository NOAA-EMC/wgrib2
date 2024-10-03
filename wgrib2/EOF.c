#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* EOF.c       10/2024 Public Domain     Wesley Ebisuzaki
 *
 * EOF and error handling
 */


FILE *err_file;
int err_int;

FILE *err_str_file;
char err_str[STRING_SIZE];

extern int file_append;

/*
 * HEADER:100:err_bin:setup:2:send (binary) integer to file upon err exit: X=file Y=integer
 */

int f_err_bin(ARG2) {
    if (mode == -1) {
	/* fatal_error call err_bin, so err_bin can not call fatal_error */
        if ((err_file = (void *) ffopen(arg1, file_append ? "ab" : "wb")) == NULL) {
            fprintf(stderr, "Could not open %s", arg1);
	    return 1;
	}
	err_int = atoi(arg2);
    }
    return 0;
}
void err_bin(int error) {
    int i;
    /* this routine may called by fatal error and end of processing */
    if (error && err_file != NULL) {
        i = fwrite(&err_int, sizeof(int), 1, err_file);
        if (i != 1) fprintf(stderr,"ERROR err_bin: write error\n");
    }
    if (err_file != NULL) ffclose(err_file);
    
    return;
}

/*
 * HEADER:100:err_string:setup:2:send string to file upon err exit: X=file Y=string
 */

int f_err_string(ARG2) {
    int len;
    if (mode == -1) {
	/* fatal_error calls err_string, so don't call fatal_error */
        if ((err_str_file = (void *) ffopen(arg1, file_append ? "ab" : "wb")) == NULL)  {
            fprintf(stderr, "Could not open %s", arg1);
	    return 1;
        }
	len = strlen(arg2)+1;
        if (len > STRING_SIZE) len = STRING_SIZE-1;
	strncpy(err_str, arg2, len);
        err_str[STRING_SIZE-1] = 0;
    }
    return 0;
}

void err_string(int error) {
    int i;
    /* this routine may called by fatal error and end of processing */
    if (error && err_str_file != NULL && err_str[0] != 0) {
	i = fwrite(err_str, strlen(err_str), 1, err_str_file);
	if (i != 1) fprintf(stderr,"ERROR err_string: write error\n");
    }
    if (err_str_file != NULL) ffclose(err_str_file);
    return;
}

/*
 * HEADER:100:eof_bin:setup:2:send (binary) integer to file upon EOF: X=file Y=integer
 */

int f_eof_bin(ARG2) {
    int i,j;
    if (mode == -1) {
        if ((*local = (void *) ffopen(arg1, file_append ? "ab" : "wb")) == NULL) {
            fprintf(stderr,"Could not open %s", arg1);
	    return 1;
        }
    }
    else if (mode == -2 && *local != NULL) {
	j = atoi(arg2);
	i = fwrite(&j, sizeof(int), 1, (FILE *) *local);
	if (i != 1) fprintf(stderr,"Problem eof_bin: write file %s", arg1);
	ffclose((FILE *) *local);
    }
    return 0;
}

/*
 * HEADER:100:eof_string:setup:2:send string to file upon EOF: X=file Y=string
 */

int f_eof_string(ARG2) {
    int i;
    if (mode == -1) {
        if ((*local = (void *) ffopen(arg1, file_append ? "ab" : "wb")) == NULL) {
            fprintf(stderr, "Could not open %s", arg1);
            return 1;
        }
    }
    else if (mode == -2 && *local != NULL) {
        i = fwrite(arg2, strlen(arg2), 1, (FILE *) *local);
        if (i != 1) fprintf(stderr, "Problem: eof_string: write error %s", arg1);
	ffclose((FILE *) *local);
    }
    return 0;
}
