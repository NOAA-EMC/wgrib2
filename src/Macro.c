#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"


/* 3/2008 Public Domain Wesley Ebisuzaki
 * 3/2008 Manfred Schwarb added -V
 * 3/2012 Wesley Ebisuzaki: added use_ext_name (extended name)
 * 2/2021 Wesley Ebisuzaki: replaced use_ext_name by type_ext_name as many flavors of ext_name
 */

extern const char *item_deliminator;
extern int file_append, decode, latlon;
extern unsigned int type_ext_name;
extern int ieee_little_endian;

/*
 * HEADER:100:s:inv:0:simple inventory
 */

/*
 * this is a simple macro .. see how easy it is!
 * would be more complicated if functions used static variables
 * minor complication if need to set decode or latlon flags
 */

int f_s(ARG0) {

    if (mode >= 0) {
	f_t(call_ARG0(inv_out,NULL));
	strcat(inv_out,item_deliminator);
	inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_var(call_ARG0(inv_out,NULL));
	else f_ext_name(call_ARG0(inv_out,NULL));

	strcat(inv_out,item_deliminator);
	inv_out += strlen(inv_out);

	f_lev(call_ARG0(inv_out, NULL));
	strcat(inv_out,item_deliminator);
	inv_out += strlen(inv_out);

	f_ftime(call_ARG0(inv_out,NULL));
	strcat(inv_out,item_deliminator);
	inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_misc(call_ARG0(inv_out,NULL));
    }
    return 0;
}

/*
 * HEADER:100:s2:inv:0:simple inventory .. for testing ftime2
 */

/*
 * this is a simple macro .. see how easy it is!
 * would be more complicated if functions used static variables
 * minor complication if need to set decode or latlon flags
 */

int f_s2(ARG0) {

    if (mode >= 0) {
        f_t(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_var(call_ARG0(inv_out,NULL));
        else f_ext_name(call_ARG0(inv_out,NULL));

        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_lev(call_ARG0(inv_out, NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_ftime2(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_misc(call_ARG0(inv_out,NULL));
    }
    return 0;
}

/*
 * HEADER:100:S:inv:0:simple inventory with minutes and seconds (subject to change)
 */

int f_S(ARG0) {

    if (mode >= 0) {
        f_T(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_var(call_ARG0(inv_out,NULL));
	else f_ext_name(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_lev(call_ARG0(inv_out, NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_ftime(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_misc(call_ARG0(inv_out,NULL));
    }
    return 0;
}


/*
 * HEADER:100:s_out:inv_output:1:simple inventory written to X
 */

int f_s_out(ARG1) {
    int i;
    struct seq_file *save;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("bin: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
    }
    else if (mode >= 0) {
        save = *local;
	inv_out[0] = 0;
	f_s(call_ARG0(inv_out,NULL));
        i = strlen(inv_out);
        inv_out[i++] = '\n';
        inv_out[i] = '\0';
        fwrite_file(inv_out, 1, i, save);
	inv_out[0] = 0;
    }
    return 0;
}

/*
 * HEADER:100:inv_f77:inv_output:3:match inventory written to Z with character*(Y) and X=(bin,ieee)
 */

int f_inv_f77(ARG3) {
    int clen, len, i;
    char blanks[100];
    unsigned char header[4];
    struct local_struct {
        int charlen;
        enum {bin, ieee} header_type;
	struct seq_file out;
    };
    struct local_struct *save;

    if (mode == -1) {
	clen = atoi(arg2);
	if (clen <= 0 || clen > 400) fatal_error_i("inv_f77: len (%d) is bad or too large", clen);

	i = 0;
	if (strcmp(arg1,"bin") == 0) i = 1;
	else if (strcmp(arg1,"ieee") != 0) fatal_error("inv_f77: undefined type %s", arg1);

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("inv_f77: memory allocation","");
        if (fopen_file(&(save->out), arg3, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg3);
        }

	save->charlen = clen;
	save->header_type = i != 0 ? bin : ieee;
	return 0;
    }

    save = *local;
    clen = save->charlen;
    if (mode == -2) {                   // cleanup
	fclose_file(&save->out);
	free(save);
    }
    else if (mode >= 0) {
        inv_out[0] = 0;

        f_match_inv(call_ARG0(inv_out,NULL));
	len = strlen(inv_out);

	/* write header */
	if (save->header_type == bin) {
            fwrite_file((void *) &clen, sizeof(int), 1, &save->out);
	}
	else {
	    if (ieee_little_endian) {
		header[0] = clen & 255;
		header[1] = (clen >> 8) & 255;
		header[2] = (clen >> 16) & 255;
		header[3] = (clen >> 24) & 255;
	    }
	    else {
		header[3] = clen & 255;
		header[2] = (clen >> 8) & 255;
		header[1] = (clen >> 16) & 255;
		header[0] = (clen >> 24) & 255;
	    }
            fwrite_file((void *) header, 1, 4, &save->out);
	}


	if (len >= clen) {
	    fwrite_file((void *) inv_out, sizeof(unsigned char), clen, &save->out);
	}
	else {
	    fwrite_file((void *) inv_out, sizeof(unsigned char), len, &save->out);
	    for (i = 0; i < 100; i++) blanks[i] = ' ';
	    while (len < clen) {
		if (clen - len >= 100) {
                    fwrite_file((void *) blanks, sizeof(unsigned char), 100, &save->out);
		    len +=100;
		}
		else {
                    fwrite_file((void *) blanks, sizeof(unsigned char), clen-len, &save->out);
		    len = clen;
		}
	    }
	}
	if (save->header_type == bin) {
            fwrite_file((void *) &clen, sizeof(int), 1, &save->out);
	}
	else {
            fwrite_file((void *) header, 1, 4, &save->out);
	}
        inv_out[0] = 0;
    }
    return 0;
}

/*
 * HEADER:100:verf:inv:0:simple inventory using verification time
 */
int f_verf(ARG0) {

    if (mode >= 0) {
        f_vt(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_var(call_ARG0(inv_out,NULL));
	else f_ext_name(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_lev(call_ARG0(inv_out, NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_ftime(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        if (type_ext_name == 0) f_misc(call_ARG0(inv_out,NULL));
    }
    return 0;
}

/*
 * Manfred Schwarb
 */

/*
 * HEADER:100:V:inv:0:diagnostic output
 */

int f_V(ARG0) {
    int oldmode;
    if (mode == -1) {
	decode = 1;
    }
    if (mode >= 0) {
        f_vt(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_lev(call_ARG0(inv_out, NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        f_ftime(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);

        oldmode=mode;
        mode=1;

        if (type_ext_name == 0) f_var(call_ARG0(inv_out,NULL));
	else f_ext_name(call_ARG0(inv_out,NULL));
        strcat(inv_out,item_deliminator);
        inv_out += strlen(inv_out);
        mode=oldmode;

        if (type_ext_name == 0) f_misc(call_ARG0(inv_out,NULL));
        strcat(inv_out,"\n    ");
        inv_out += strlen(inv_out);

        f_stats(call_ARG0(inv_out,NULL));
        strcat(inv_out,"\n    ");
        inv_out += strlen(inv_out);

        f_grid(call_ARG0(inv_out,NULL));
        strcat(inv_out,"\n");
    }
    return 0;
}
