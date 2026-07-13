/** @file
 * @brief EOF and error handling options.
 * 
 * When you want your want to embed wgrib2 in your program, you want to run wgrib2 and 
 * have ways to talk with wgrib2. For example, we set up a pipe between the main program 
 * and wgrib2. Wgrib2 was suppose to send the a decoded grid to the main program. However, 
 * the grid didn't exist (EOF) or something was wrong (ERR). Consequently wgrib2 didn't 
 * send the grid over the pipe and the main program just waited for data that would never 
 * come. One method is to send an EOF or ERR message so that the main program would realize 
 * something was wrong. For the case that started this, sending a binary 0 was enough to 
 * alert the main program. (The main program was expecting a fortran sequential file and a 
 * record size of 0 is not a reasonable value.) 
 * 
 * The -err_bin, -eof_bin, -err_string, and -eof_string options are setup options. Like all 
 * setup options, they are only evaluated before processing of the file and in the case of 
 * duplicate options, only the last applies.
 * 
 * The -err_* options will write output when a "FATAL ERROR" message is triggered. The -eof_* 
 * options will write output when wgrib2 ends without a "FATAL ERROR" message. However, if 
 * wgrib2 fails in an abnormal way (example: segmentation fault), none of these routines will 
 * write output. 
 * 
 * If a "FATAL ERROR" occurs before the The -err_* option is initialized, there will be no 
 * error file written. This can occur when the command line options are wrong (option expects 
 * 3 arguments and you supply a different number). It can also occur when the initialization 
 * of an earlier option failed. (Ex. read from a non-existant file.) Consequently the -err_* 
 * options should be in front of the comamand line arguments.
 * @author Public Domain: Wesley Ebisuzaki @date 11/2012
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Pointer to error handling file. */
FILE *err_file;

/** Binary integer to be written to err_file. */
int err_int;

/** Pointer to error handling string file. */
FILE *err_str_file;

/** Error message. */
char err_str[STRING_SIZE];

/** Append grib file flag. */
extern int file_append;

/*
 * HEADER:100:err_bin:setup:2:send (binary) integer to file upon err exit: X=file Y=integer
 */

/**
 * Sends (binary) integer to file upon error exit (if fatal error is triggered). The size 
 * of the integer will be determined by the size of the native integer (often a compile 
 * option on 64 bit machines). The endian properties will depend whether the code is 
 * compiled a big or little endian machine. 
 *
 * ## Usage
 * -err_bin file integer
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, 1 if err_file can't be opened
 * 
 * @author Wesley Ebisuzaki @date 11/2012
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

/**
 * If error is non-zero, this function writes the error integer to the error file.
 * 
 * @param error Error code.
 * 
 * @author Wesley Ebisuzaki @date 11/2012
 */
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

/**
 * Sends string to file upon error exit (if fatal error is triggered).
 * 
 * ## Usage
 * -err_string file string
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, 1 if err_str_file can't be opened
 * 
 * @author Wesley Ebisuzaki @date 11/2012
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

/**
 * If error is non-zero, this function writes the error string to the error file.
 * 
 * @param error Error code.
 * 
 * @author Wesley Ebisuzaki @date 11/2012
 */
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

/**
 * Sends (binary) integer to file upon EOF (if fatal error is not triggered).
 * 
 * ## Usage
 * -eof_bin file integer
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, 1 if err_file can't be opened
 * 
 * @author Wesley Ebisuzaki @date 11/2012
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

/**
 * Sends string to file upon EOF (if fatal error is not triggered).
 * 
 * ## Usage
 * -eof_string file string
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, 1 if err_file can't be opened
 * 
 * @author Wesley Ebisuzaki @date 11/2012
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
