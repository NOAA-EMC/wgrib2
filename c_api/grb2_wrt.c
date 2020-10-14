#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "c_wgrib2api.h"

/* 3/2018 Public Domain, Wesley Ebisuzaki
 *
 * This is part of c_wgrib2api: grb2_wrt(...)
 *
 * To support a variable number of search strings,
 * C99 is required:
 *    __VA_ARGS__
 * To use a C compiler that doesn't support this C99 feature
 *   1) remove "#define grb2_wrt(...) grb2_wrtVA(__VA_ARGS__, NULL)"
 *      from c_wgrib2api.h
 *   2) Change calls to grb2_wrt(...) to grb2_wrtVA(...,NULL)
 *   The call code to grb2_wrtVA() will work in C99 systems.
 *
 * v0.99 3/2018
 */

int grb2_wrtVA(const char *grb, const char *template, int msgno, float *data, 
         unsigned int ndata, ...) {
    va_list valist;
    char *option, *str_arg;
    int i, ierr;
    long long int ival8;
    size_t bufsize;
    char line[100];
    char buffer[71];

    /* copy data to register 9, check for size mismatch is internal to wgrib2(..)  */

    if (ndata == 0) {
	fprintf(stderr,"grb2_wrt error: ndata == 0\n");
        return 1;
    }
    ierr = wgrib2_set_reg(data, ndata, 9);
    if (ierr) {
	fprintf(stderr,"grb2_wrt error: saving data to reg_9\n");
        return 1;
    }

    /* create wgrib2 command line */
    wgrib2_init_cmds();
    wgrib2_add_cmd(template);
    wgrib2_add_cmd("-rewind_init");
    wgrib2_add_cmd(template);
    wgrib2_add_cmd("-d");
    sprintf(line,"%d", msgno);
    wgrib2_add_cmd(line);
    wgrib2_add_cmd("-rpn_rcl");
    wgrib2_add_cmd("9");


    /* loop over all the optional arguments
     * all arguments come in pairs, string, value
     *   value = string pointer
     *   value = integer
     *   value = integer pointer
     *   value = unsigned integer
     *   value = unsigned integer pointer
     *   value = long integer
     *   value = long integer pointer
     *   value = float
     *   value = float pointer
     */

    va_start(valist, ndata);
    option = (char *) va_arg(valist, char * );
    while (option) {
        if (strcmp(option,"lev") == 0) {
            wgrib2_add_cmd("-set_lev");
	    str_arg = (char *) va_arg(valist, char * );
            wgrib2_add_cmd(str_arg);
        }
        else if (strcmp(option,"grib_type") == 0) {
            wgrib2_add_cmd("-set_grib_type");
	    str_arg = (char *) va_arg(valist, char * );
            wgrib2_add_cmd(str_arg);
        }
        else if (strcmp(option,"ftime") == 0) {
            wgrib2_add_cmd("-set_ftime");
	    str_arg = (char *) va_arg(valist, char * );
            wgrib2_add_cmd(str_arg);
        }
        else if (strcmp(option,"var") == 0) {
            wgrib2_add_cmd("-set_var");
	    str_arg = (char *) va_arg(valist, char * );
            wgrib2_add_cmd(str_arg);
        }
        else if ( (strcmp(option,"meta") == 0) || (strcmp(option,"metadata_str") == 0) ) {
            wgrib2_add_cmd("-set_metadata_str");
	    str_arg = (char *) va_arg(valist, char * );
            wgrib2_add_cmd(str_arg);
        }
        else if (strcmp(option,"bin_prec") == 0) {
            wgrib2_add_cmd("-set_bin_prec");
	    i = (int) va_arg(valist, int );
	    sprintf(line,"%d",i);
            wgrib2_add_cmd(line);
	    fprintf(stderr,"binprec: %s\n",line);
        }
        else if (strcmp(option,"percentile") == 0) {
            wgrib2_add_cmd("-set_percentile");
	    i = (int) va_arg(valist, int );
	    sprintf(line,"%d",i);
            wgrib2_add_cmd(line);
        }
        else if (strcmp(option,"set") == 0) {
	    str_arg = (char *) va_arg(valist, char * );
	    if (strcmp(str_arg,"model_version_date") == 0) {
	        ival8 = (long long int) va_arg(valist, long long int );
	        sprintf(line,"%lld",ival8);
                wgrib2_add_cmd("-set");
		wgrib2_add_cmd("model_version_date");
	        sprintf(line,"%lld",ival8);
                wgrib2_add_cmd(line);
	    }
	    else {
                wgrib2_add_cmd("-set");
                wgrib2_add_cmd(str_arg);
	        i = (int) va_arg(valist, int );
	        sprintf(line,"%d",i);
                wgrib2_add_cmd(line);
	    }
        }
        else if (strcmp(option,"date") == 0) {
            wgrib2_add_cmd("-set_date");
	    ival8 = (long long int) va_arg(valist, long long int );
	    sprintf(line,"%lld",ival8);
            wgrib2_add_cmd(line);
	    fprintf(stderr,"date: %s\n",line);
        }
	else {  
	    printf("unknown option=%s\n",option);
	    str_arg = (char *) va_arg(valist, char * );
	}
	option = (char *) va_arg(valist, char * );
    }
    va_end(valist);
    wgrib2_list_cmd();
    wgrib2_add_cmd("-grib_out");
    wgrib2_add_cmd(grb);
    wgrib2_list_cmd();
    i = wgrib2_cmd();
    if (i) return 2;		/* failed call to wgrib2 */
    return  0;
}
