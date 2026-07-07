/** @file
 * @brief Routines that import/export sections of a grib message. Only support bin and bin 
 * with f77 header.
 * @author Public Domain: Wesley Ebisuzaki @date 11/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wgrib2.h"
#include "fnlist.h"

/** File header flag. */
extern int header;

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/*
 * HEADER:100:write_sec:output:2:write grib message section X (0-8) to binary file Y
 */

/**
 * Writes the current grib message section (0-8) to a file.
 * 
 * The only options that applies to the file are the -header, -no_header, -append, and 
 * -no_append options. The default -header option put a 4-byte unsigned integer header and 
 * trailer around the section data. The header and trailer are the number of bytes of the 
 * section. 
 * 
 * The -write_sec option is for codes that need low-level access to the grib data.
 * 
 * ## Usage
 *  -write_sec N file
 *
 * N = section number (0-8)
 * file = output file name
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 11/2020
 */
int f_write_sec(ARG2) {
    unsigned int size;
    int n;
    size_t j;
    struct seq_file *save;

    if (isdigit((unsigned char) arg1[0]) == 0) fatal_error("export_sec: illegal section %s", arg1);
    n = atoi(arg1);
    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("export_sec: memory allocation","");
        if (fopen_file(save, arg2, file_append ? "ab" : "wb") != 0) fatal_error("Could not open %s", arg2);
        if (n < 0 || n > 8) fatal_error_i("export_sec: illegal section %d", n);
        return 0;
    }
    save = *local;
    if (mode == -2) {
        fclose_file(save);
        free(save);
        return 0;
    }
    if (sec[n] == NULL) size = 0;
    else if (n == 0) size = 16;
    else if (n == 8) size = 4;
    else size = uint4(sec[n]);

    if (header) {
        j = fwrite_file((void *) &size, sizeof(unsigned int), 1, save);
        if (j != 1) fatal_error_i("export_sec: sec %d writing header", n);
    }
    if (size > 0) {
        j = fwrite_file((void *) sec[n], 1, size, save);
        if (j != size) fatal_error_i("export_sec: sec %d error writing", n);
    }
    if (header) {
        j = fwrite_file((void *) &size, sizeof(unsigned int), 1, save);
        if (j != 1) fatal_error_i("export_sec: sec %d error writing trailer", n);
    }
    if (flush_mode) fflush_file(save);
    return 0;
}


/*
 * HEADER:100:read_sec:misc:2:read grib message section (0-8) X from binary file (Y)
 */

/**
 * Replace the section (0-8) of the current grib message with the contents from a file.
 * 
 * The only options that applies to the file are the -header and -no_header options. The 
 * default -header option put a 4-byte unsigned integer header and trailer around the section 
 * data. The header and trailer are the number of bytes of the section. 
 * 
 * The -read_sec option is used to create grib files where you do not have an appropriate 
 * template. 
 * 
 * ## Usage
 * -read_sec N file
 *
 * N = section number (0-8)
 * file = input file name
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 11/2020
 */
int f_read_sec(ARG2) {

    struct local_struct {
        struct seq_file in;
        int n;
        unsigned char *data[9];
        unsigned int data_size[9];
    };

    struct local_struct *save;

    unsigned int size, size2;
    int n, i;
    size_t j;
    unsigned char csize[4];

    if (mode == -1) {
        if (isdigit((unsigned char) arg1[0]) == 0) fatal_error("import_sec: illegal section %s", arg1);
        n  = atoi(arg1);
        if (n < 0 || n > 8) fatal_error_i("import_sec: illegal section %d", n);

        // allocate static variables
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct) );
        if (save == NULL) fatal_error("import_sec: memory allocation","");
        if (fopen_file(&(save->in), arg2, "rb") != 0) fatal_error("import_sec: Could not open %s", arg2);
        save->n = n;
        for (i = 0; i < 9; i++) {
            save->data_size[i] =  0;
            save->data[i] = NULL;
        }
        return 0;
    }
    save = (struct local_struct *) *local;
    if (mode == -2) {
        // close file, free static varaibles
        save = (struct local_struct *) *local;
        fclose_file(&(save->in));
        for (i = 0; i < 9;  i++) {
            if (save->data_size[i] != 0) free(save->data[i]);
        }
        free(save);
        return 0;
    }

    n = save->n;
    if (header) {
        /* read header */
        if (fread_file((void *) &size, sizeof(unsigned int), 1, &(save->in)) != 1) fatal_error_i("import_sec: sec %d read header",n);

        /* read data */
        if (size != 0) {
            if (size > save->data_size[n]) {
                if (save->data_size[n] != 0) free(save->data[n]);
                save->data[n] = malloc(size*sizeof(unsigned char));
                if (save->data[n] == NULL) fatal_error_i("import_sec: sec %d memory allocation",n);
                save->data_size[n] = size;
            }
            j = fread_file(save->data[n], sizeof(char), size, &(save->in));
            if (size != j) fatal_error_i("import_sec: sec %d read with header failed",n);
        }

        /* read trailer */
        if (fread_file((void *) &size2, sizeof(unsigned int), 1, &(save->in)) != 1) fatal_error_i("import_sec: sec %d read trailer",n);
        if (size != size2) fatal_error_i("import_sec: sec %d trailer size mismatch",n);
    }

    /* no header, can stack only if sections are not missing */
    else {
        /* read fixed length sections */
        if (n == 0 || n == 8 ) {
            size = n == 0 ? 16 : 4;
            if (size > save->data_size[n]) {
                if (save->data_size[n] != 0) free(save->data[n]);
                save->data[n] = malloc(size*sizeof(unsigned char));
                if (save->data[n] == NULL) fatal_error_i("import_sec: sec %d memory allocation",n);
                save->data_size[n] = size;
            }
            j = fread_file(save->data[n], sizeof(unsigned char), size, &(save->in));
            if (size != j) fatal_error_i("import_sec: sec %d read failed",n);
        }
        else {
            // size in in  bytes 1-4
            j = fread_file(csize, sizeof(unsigned char), 4, &(save->in));
            if (j > 0 && j < 4) fatal_error_i("import_sec: sec %d read error",n);
            if (j == 0)  size = 0;
            else size = uint4(csize);

            if (size > 0) {
                if (size > save->data_size[n]) {
                    if (save->data_size[n] != 0) free(save->data[n]);
                    save->data[n] = malloc(size*sizeof(unsigned char));
                    if (save->data[n] == NULL) fatal_error_i("import_sec: sec %d memory allocation",n);
                    save->data_size[n] = size;
                }
                j = fread_file(save->data[n]+4, sizeof(char), size-4, &(save->in));
                if (j != size-4) fatal_error_i("import_sec: sec %d read error",n);
                save->data[n][0] = csize[0];
                save->data[n][1] = csize[1];
                save->data[n][2] = csize[2];
                save->data[n][3] = csize[3];
            }
        }
    }
    /* now to replace section */
    sec[n] = size == 0 ? NULL : save->data[n];
    return 0;
}
