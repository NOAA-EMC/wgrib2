/* UDF.c
 *
 *  user defined functions
 *
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
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#if defined USE_UDF

#include <unistd.h>

extern unsigned int npnts;
extern int header;
extern int decode, latlon;

/*
 * HEADER:100:sys:misc:1:run system/shell command, X=shell command
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
