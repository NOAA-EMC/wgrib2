/** @file
 * @brief User Defined Functions.
 * 
 * Paranoid people should turn off UDFs. UDFs may not work on Windows machines.
 * @author Public Domain: Wesley Ebisuzaki @date 2010
 */

/* 
 * -sys : run a shell command
 *
 * -udf_arg: open UDF calling file, append data to it, close UDF calling file
 *
 * -udf: run shell command, take UDF return file and replace the data array
 *
 * to do later: -udf_arg will use RPN registers
 *                 RPN register will have names
 *               udf will be able to return multiple (named) fields
 *
 * paranoid people should turn off UDFs
 *  UDFs may not work on windows machines
 *
 *  public domian 2010 Wesley Ebisuzaki
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

#if defined USE_UDF

#include <unistd.h>

/** Number of grid points. */
extern unsigned int npnts;

/** File header flag. */
extern int header;

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/*
 * HEADER:100:sys:misc:1:run system/shell command, X=shell command
 */

/**
 * Run a shell command.
 * 
 * The -sys option is only available when the UDF (User Defined Function) extension is 
 * installed. The -sys option runs a shell command. 
 * 
 * ## Usage
 * -sys STRING
 * 
 * STRING is a shell command.
 * 
 * ## Comments
 * The UDF options may not work in windows. 
 * 
 * The use of the UDF options is limited and the UDF options should, IMHO, not be enabled 
 * unless the UDF options are needed. 
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws a fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 2010
 */
int f_sys(ARG1) {
    FILE *fp;
    int c;
    char *p;

    /* if uid != euid - then setuid on file .. security concern */
    /* if gid != egid - then setgid on file .. security concern */
    /* check if euid and eguid are same as uid and gid */

    if (mode >= 0) {
        if (getuid() != geteuid()) fatal_error("sys: setuid bit should not be set","");
        if (getgid() != getegid()) fatal_error("sys: setgid bit should not be set","");

        fp = popen(arg1,"r");
        if (fp == NULL) fatal_error("sys: popen failed","");
        p = inv_out;
        while ((c = fgetc(fp)) != EOF) {
            *p++ = c;
        }
        *p = 0;
        if (pclose(fp) == -1) fatal_error("sys: pclose failed","");
    }
    return 0;
}

/*
 * HEADER:100:udf_arg:misc:2:add grib-data to UDF argument file, X=file Y=name
 */

/**
 * Open UDF calling file, append data to it, close UDF calling file.
 * 
 * ## Usage
 * -udf_arg FILE NAME
 *
 * FILE is the name of the UDF calling file.
 * NAME is the name of the data field.
 * 
 * ## Comments
 * The UDF options may not work in windows. 
 * 
 * The use of the UDF options is limited and the UDF options should, IMHO, not be enabled 
 * unless the UDF options are needed. 
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws a fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 2010
 */
int f_udf_arg(ARG2) {
    FILE *out;
    int ibuf[3];
    int i;
    char string[STRING_SIZE];

    if (mode == -1) {
        decode = 1;
    }
    if (mode < 0) return 0;

    if (data == NULL) fatal_error("udf_arg: data was not decoded","");

    if ((out = fopen(arg1,"ab")) == NULL) fatal_error("udf_arg: problem opening file ", arg1);

    if (strcmp(arg2,"-") == 0) {
        *string = 0;
        f_full_name(call_ARG0(string,NULL) );
        arg2 = string;
    }

    ibuf[0] = strlen(arg2) + 1;
    ibuf[1] = npnts;
    ibuf[2] = 256 + sizeof(float) ;
    i = 3 * sizeof(int);
    if (header) fwrite((void *) &i, sizeof(int),1,out);
    fwrite((void *) &(ibuf[0]), sizeof(int), 3, out);
    if (header) fwrite((void *) &i, sizeof(int),1,out);

    i = strlen(arg2) + 1;
    if (header) fwrite((void *) &i, sizeof(int),1,out);
    fwrite((void *) arg2, sizeof(char), i, out);
    if (header) fwrite((void *) &i, sizeof(int),1,out);

    i = npnts * sizeof(float);
    if (header) fwrite((void *) &i, sizeof(float),1,out);
    fwrite((void *) data, sizeof(float), npnts, out);
    if (header) fwrite((void *) &i, sizeof(float),1,out);

    fclose(out);
    return 0;
}

/*
 * HEADER:100:udf:misc:2:run UDF, X=program+optional_args, Y=return file
 */

/**
 * Run shell command, take UDF return file and replace the data array.
 * 
 * The -udf option is only available when the UDF (User Defined Function) extension is 
 * installed. The -udf option runs a shell command (arg1) and then sets the grid point so to 
 * contents of a binary file (arg2). 
 * 
 * ## Usage
 * -udf STRING FILE
 * 
 * STRING is a shell command.
 * FILE is a binary file which contains the contents of grid points.
 * 
 * Some functions can be written in RPN and others are more conveniently writen is a language 
 * like C for Fortran. The idea behind UDF is that the input file to the user define function 
 * is prepared while scanning the grib file. When the input file is complete, the user defined 
 * function is called. If the user defined function returns no results, then the -sys option 
 * is used to run the user defined function otherwise -udf is used. 
 * 
 * ## Comments
 * The UDF options may not work in windows. 
 * 
 * The use of the UDF options is limited and the UDF options should, IMHO, not be enabled 
 * unless the UDF options are needed. 
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws a fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 2010
 */
int f_udf(ARG2) {

    FILE *out;
    int i, j;

    if (mode == -1) {
        decode = 1;
        // f_sys(call_ARG1(inv_out,NULL,arg1));
    }

    if (mode >= 0) {
        /* run udf */
        f_sys(call_ARG1(inv_out,NULL,arg1));
        // remove(arg1);

        out = fopen(arg2,"rb");
        if (out == NULL) fatal_error("udf: could not open file ", arg2);

        if (header) {
            if (fread((void *) &i, sizeof(int), 1, out) != 1)
                fatal_error("udf: read error header","");
            if (i != sizeof(float) * ndata)
                fatal_error_i("udf: trailer record size wrong, %u", i);
        }
        j = fread(data, sizeof(float), ndata, out);
        if (j != ndata) fatal_error_i("udf: read error ndata = %u", i);
        if (header) {
            if (fread((void *) &i, sizeof(int), 1, out) != 1)
                fatal_error("udf: read error trailer","");
            if (i != sizeof(float) * ndata)
                fatal_error_i("udf: trailer record size wrong, %u", i);
        }
        fclose(out);
    }
    return 0;
}

#else

int f_sys(ARG1) {
    fatal_error("User Defined Functions are not installed","");
    return 1;
}
int f_udf_arg(ARG2) {
    fatal_error("User Defined Functions are not installed","");
    return 1;
}
int f_udf(ARG2) {
    fatal_error("User Defined Functions are not installed","");
    return 1;
}
#endif
