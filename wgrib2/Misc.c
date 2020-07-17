#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern enum input_type input;
extern int header, dump_rec, dump_submsg;
extern int mode;
extern int ieee_little_endian;

extern int file_append;
extern const char *item_deliminator;
extern struct seq_file inv_file;
extern const char *nl;
extern const char *end_inv;
extern struct seq_file rd_inventory_input;

extern int use_g2clib;

/*
 * HEADER:100:i:setup:0:read Inventory from stdin
 */
int f_i(ARG0) {
    if (mode == -1) input = inv_mode;
    return 0;
}

/*
 * HEADER:100:i_file:setup:1:read Inventory from file
 */
int f_i_file(ARG1) {
    if (mode == -1) {
        input = inv_mode;
        if (fopen_file(&rd_inventory_input, arg1, "r") != 0) {
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
	fclose_file(&rd_inventory_input);
    }
    return 0;
}

/*
 * HEADER:100:v0:misc:0:not verbose (v=0)
 */

int f_v0(ARG0) {
    set_mode(0);
    return 0;
}

/*
 * HEADER:100:v:misc:0:verbose (v=1)
 */

int f_v(ARG0) {
    set_mode(1);
    return 0;
}

/*
 * HEADER:100:v2:misc:0:really verbose (v=2)
 */

int f_v2(ARG0) {
    set_mode(2);
    return 0;
}

/*
 * HEADER:-1:v97:misc:0:verbose mode for debugging only (v=97) 
 */

int f_v97(ARG0) {
    set_mode(97);
    return 0;
}

/*
 * HEADER:-1:v98:misc:0:verbose mode for debugging only (v=98) 
 */

int f_v98(ARG0) {
    set_mode(98);
    return 0;
}

/*
 * HEADER:-1:v99:misc:0:verbose mode for debugging only (v=99) 
 */

int f_v99(ARG0) {
    set_mode(99);
    return 0;
}

/*
 * HEADER:100:header:misc:0:f77 header or nx-ny header in text output (default)
 */

int f_header(ARG0) {
    header = 1;
    return 0;
}

/*
 * HEADER:100:no_header:misc:0:no f77 header or nx-ny header in text output
 */

int f_no_header(ARG0) {
    header = 0;
    return 0;
}

/*
 * HEADER:100:nl:inv:0:inserts new line into inventory
 */

int f_nl(ARG0) {
    if (mode >= 0) strcpy(inv_out, "\n");
    return 0;
}

/*
 * HEADER:100:nl_out:inv_output:1:write new line in file X
 */
int f_nl_out(ARG1) {
    struct seq_file *save;
    char nl = '\n';

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("nl_out: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
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
	if (fwrite_file(&nl, sizeof(char), 1, save) != 1) 
		fatal_error("nl_out: write error %s", arg1);
    }
    return 0;
}

/*
 * HEADER:100:print:inv:1:inserts string (X) into inventory
 */

int f_print(ARG1) {
    if (mode >= 0) {
	strcpy(inv_out, arg1);
    }
    return 0;
}

/*
 * HEADER:100:print_out:inv_output:2:prints string (X) in file (Y)
 */
int f_print_out(ARG2) {
    struct seq_file *save;
    int i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("nl_out: memory allocation","");
        if (fopen_file(save, arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }
    }
    else if (mode == -2) {
	save = *local;
	fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = *local;
        i = strlen(arg1);
	if (fwrite_file(arg1,1, i, save) != i) return 1;
    }
    return 0;
}

/*
 * HEADER:100:colon:misc:1:replace item deliminator (:) with X
 */

int f_colon(ARG1) {
    item_deliminator = arg1;
    return 0;
}

/*
 * HEADER:100:one_line:setup:0:puts all on one line (makes into inventory format)
 */

int f_one_line(ARG0) {
    nl = " ";
    return 0;
}

/*
 * HEADER:100:crlf:setup:0:make the end of the inventory a crlf (windows) instead of newline (unix)
 */

int f_crlf(ARG0) {
    end_inv = "\r\n";
    return 0;
}

/*
 * HEADER:100:append:setup:0:append mode, write to existing output files
 */
int f_append(ARG0) {
    if (mode == -1) file_append = 1;
    return 0;
}
/*
 * HEADER:100:no_append:setup:0:not append mode, write to new output files (default)
 */
int f_no_append(ARG0) {
    if (mode == -1) file_append = 0;
    return 0;
}

/*
 * HEADER:100:inv:setup:1:write inventory to X
 */

int f_inv(ARG1) {
    if (mode == -1) {
	fclose_file(&inv_file);
        if (fopen_file(&inv_file, arg1, file_append ? "a+" : "w+") != 0) {
            fatal_error("Could not open %s", arg1);
	}
    }
    else if (mode == -2) {
	fclose_file(&inv_file);
	/* reopen inv to point to stdout */
	if (fopen_file(&inv_file,"-","w") != 0) fatal_error("Could not set inv_file to stdout","");
    }
    return 0;
}

/*
 * HEADER:100:little_endian:misc:0:sets ieee output to little endian (default is big endian)
 */
int f_little_endian(ARG0) {
    ieee_little_endian = 1;
    return 0;
}

/*
 * HEADER:100:big_endian:misc:0:sets ieee output to big endian (default is big endian)
 */

int f_big_endian(ARG0) {
    ieee_little_endian = 0;
    return 0;
}

/*
 * HEADER:100:g2clib:setup:1:X=0/1/2 0=WMO std 1=emulate g2clib 2=use g2clib
 */
int f_g2clib(ARG1) {
    use_g2clib = atoi(arg1);
    if (use_g2clib == 0 || use_g2clib == 1) return 0;
#ifdef USE_G2CLIB
    if (use_g2clib == 2) return 0;
#endif
    if (use_g2clib == 2) fatal_error("g2clib not installed","");
    fatal_error("illegal g2clib option %s", arg1);
    return 0;
}

