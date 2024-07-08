/*
 * Last.c Public Domain 2015 Wesley Ebisuzaki
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int file_append;
extern char *last_inv_out;

/*
 * HEADER:100:last:inv_output:1:write last inv item to file X
 */
int f_last(ARG1) {
    struct seq_file *save;
    size_t i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("last: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "a" : "w") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
	save = *local;
        fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = *local;
	i = strlen(last_inv_out);
	fwrite_file(last_inv_out, sizeof(char), i, save);	/* i+1 .. for \0 */
	repeat_inv_out();
    }
    return 0;
}

/*
 * HEADER:100:last0:inv_output:1:write last inv item to beginning of file X
 */
int f_last0(ARG1) {
    struct seq_file *save;
    size_t i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("last: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "a" : "w") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
	save = *local;
        fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = *local;
	i = strlen(last_inv_out);
	fseek_file(save, 0L, SEEK_SET);
	fwrite_file(last_inv_out, sizeof(char), i, save);	/* i+1 .. for \0 */
	repeat_inv_out();
    }
    return 0;
}
