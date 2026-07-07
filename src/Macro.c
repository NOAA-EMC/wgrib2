/** @file
 * @brief Macros for inventory generation.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2008 | W. Ebisuzaki | Initial
 * 3/2008 | M. Schwarb | Added -V option
 * 3/2012 | W. Ebisuzaki | Added use_ext_name (extended name)
 * 2/2021 | W. Ebisuzaki | Replaced use_ext_name by type_ext_name as many flavors of ext_name
 *
 * @author Public Domain: Wesley Ebisuzaki @date 3/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Item delimiter string.*/
extern const char *item_deliminator;

/** Append grib file flag. */
extern int file_append;

/** Decode grib file flag.   */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Extended name type. */
extern unsigned int type_ext_name;

/**< IEEE little endian flag. */
extern int ieee_little_endian;

/*
 * HEADER:100:s:inv:0:simple inventory
 */

/*
 * this is a simple macro .. see how easy it is!
 * would be more complicated if functions used static variables
 * minor complication if need to set decode or latlon flags
 */

/**
 * Prints simple inventory.
 * 
 * The -s option is coded like a macro, the option call other options like -t, -var, -lev, 
 * -ftime, and -misc. The last option, -misc, changes with time. For example, when chemical 
 * tracers were added, -misc was expanded to print out the characteristics of the tracers. 
 * 
 * The output of the options, including -s, can be saved by the -inv option. 
 * 
 * ## Usage
 * -s
 * 
 * Equivalent to:
 *      -t -var -lev -ftime -ens
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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

/**
 * Prints simple inventory. For testing ftime2.
 * 
 * ## Usage
 * -s2
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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

/**
 * Prints simple inventory with minutes and seconds (subject to change).
 * 
 * ## Usage
 * -S
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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

/**
 * Writes a simple inventory to a specified output file.
 * 
 * This option is obsolete with the introduction of the -last option.
 * 
 * old:  -s_out FILE
 * new:  -s -last FILE -nl_out FILE
 * 
 * ## Usage
 * -s_out FILE
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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

/**
 * Writes the match inventory (see the -match_inv option) to a file that is readable by a 
 * fortran unformatted I/O statement.
 * 
 * The format is more or less standand and consistes of a 4-byte integer of number of bytes 
 * in the character string, the character string and 4-byte integer with the length of the 
 * character string as a trailer. If the match inventory is shorter than the character string, 
 * it is padded with blanks. If the match inventory is longer, it simply truncated. Not all 
 * fortran compilers support this format. 
 * 
 * This option was added to facilitate the writing of fortran codes that can read wgrib2 output. 
 * 
 * ## Usage
 * -inv_f77 OPTION CLEN FILE
 * 
 * OPTION = "ieee" or "bin"
 *      bin = native format
 *      ieee = 4-byte big_endian header/trailer (default)
 *             4-byte little_endian header/trailer (by option)
 * CLEN = integer, the length of the character string
 * FILE = output file
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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

/**
 * Prints simple inventory using verification time.
 * 
 * The -s, -verf and -V options are really macros which are defined in the Macro.c file. 
 * The -s option is special because if there is no "inv" option used, wgrib2 will add a 
 * -s option to the end of the argument list. 
 * 
 * ## Usage
 * -verf
 * 
 * Equivalent to:
 *      -vt -var -lev -ftime -ens
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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
 * HEADER:100:V:inv:0:diagnostic output
 */

/**
 * Prints diagnostic output.
 * 
 * The -s, -verf and -V options are really macros which are defined in the Macro.c file. 
 * The -s option is special because if there is no "inv" option used, wgrib2 will add a 
 * -s option to the end of the argument list. 
 * 
 * ## Usage
 * -V
 * 
 * Equivalent to:
 *      -vt -lev -ftime -var -ens -stats -grid
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Manfred Schwarb @date 3/2008
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
