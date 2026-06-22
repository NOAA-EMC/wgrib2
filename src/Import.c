/** @file
 * @brief Some routines to read data into data buffer.
 * @author Public Domain: Wesley Ebisuzaki @date 10/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Use scale flag. */
extern int use_scale;

/** Number of grid points in x direction. */
extern unsigned int nx_;

/** Number of grid points in y direction. */
extern unsigned int ny_;

/** IEEE little endian flag. */
extern int ieee_little_endian;

/** Header flag. */
extern int header;

/*
 * HEADER:100:import_text:misc:1:read text file (X) for data
 */

/**
 * Import data from text file.
 * 
 * Wgrib2 will decode the grib message and save the decoded grid point values in a floating 
 * point array (DATA). The -import options read grid point values from a specified file and 
 * replace the values of DATA. The size of DATA and imported grid should match. The -import 
 * options are often used to read data that is later written out as a grib message. 
 * 
 * Note that the import functions will reset the scaling and precision of the grib writing 
 * (new files) to the default (ECMWF-style, 12 bits). Any -set_metadata should be done after 
 * the -import functions. 
 * 
 * ## Scan Order
 * The grib message's scan order is called the "input" scan order (wgrib2 -grid). Wgrib2 
 * converts this to the "output" scan order. (This is the scan order for options like: -bin, 
 * -text, -cvs.) The import file needs to be in the "output" scan order. Of course, you can 
 * change the output scan order using the -order option. 
 * 
 * ## Import Format
 * The file that you import needs to be in a special format. 
 * 
 * - grib: grib2 message
 * - bin: native single point format
 * - ieee: IEEE single point format
 * - bin: may have a f77 style header depending on the -header option
 * - ieee: may have a f77 style header depending on the -header option
 * - ieee: may be little or big (default) ending depending on options
 * - text: may have a "nx ny" header depending on the -header option (see -text option)
 * 
 * ## Usage
 * -import_text FILE
 * 
 * FILE is the text file to read.
 * 
 * Note that the -header and -no_header affect the data file format
 * 
 * @note Grid size (if it can be determined) must match the current grid.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
 */
int f_import_text(ARG1) {
    unsigned int ix, iy;
    unsigned int i;
    float t;

    if (mode == -1) {
        if ((*local = (void *) fopen(arg1, "r")) == NULL) 
            fatal_error("import_text: Could not open %s", arg1);
        decode = 1; 
    }
    else if (mode == -2) {
        fclose((FILE *) *local);
    }
    else if (mode >= 0) {
        if (header) {
            if (fscanf((FILE *) *local, "%u %u", &ix, &iy) != 2) {
                fatal_error("import_text: Could not read nx, ny in file %s", arg1);
            }
            if (nx_ != ix) {
                fatal_error_u("import_text: nx=%u is wrong",ix);
            }
            if (ny_ != iy) {
                fatal_error_u("import_text: ny=%u is wrong",iy);
            }
        }
        for (i = 0; i < ndata; i++) {
            if (fscanf((FILE *) *local, "%f", &t) != 1)
                fatal_error_u("import_text: Could not read data. location=%u",i+1);
            data[i] = t;
        }
        use_scale = 0;
    }
    return 0;
}

/*
 * HEADER:100:import_ieee:misc:1:read ieee file (X) for data
 */

/**
 * Import data from ieee file.
 * 
 * Wgrib2 will decode the grib message and save the decoded grid point values in a floating 
 * point array (DATA). The -import options read grid point values from a specified file and 
 * replace the values of DATA. The size of DATA and imported grid should match. The -import 
 * options are often used to read data that is later written out as a grib message. 
 * 
 * Note that the import functions will reset the scaling and precision of the grib writing 
 * (new files) to the default (ECMWF-style, 12 bits). Any -set_metadata should be done after 
 * the -import functions. 
 * 
 * ## Scan Order
 * The grib message's scan order is called the "input" scan order (wgrib2 -grid). Wgrib2 
 * converts this to the "output" scan order. (This is the scan order for options like: -bin, 
 * -text, -cvs.) The import file needs to be in the "output" scan order. Of course, you can 
 * change the output scan order using the -order option. 
 * 
 * ## Import Format
 * The file that you import needs to be in a special format. 
 * 
 * - grib: grib2 message
 * - bin: native single point format
 * - ieee: IEEE single point format
 * - bin: may have a f77 style header depending on the -header option
 * - ieee: may have a f77 style header depending on the -header option
 * - ieee: may be little or big (default) ending depending on options
 * - text: may have a "nx ny" header depending on the -header option (see -text option)
 * 
 * ## Usage
 * -import_ieee FILE
 * 
 * FILE is the big-endian 32-bit ieee file to read.
 * 
 * Note that the -header and -no_header affect the data file format.
 * 
 * @note Grid size (if it can be determined) must match the current grid.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
 */
int f_import_ieee(ARG1) {
    int i;
    struct seq_file *save;
    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("import_ieee: memory allocation","");
        if (fopen_file(save, arg1, "rb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        decode = 1;
    }
    else if (mode == -2) {
        save = (struct seq_file *) *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = (struct seq_file *) *local;
        i = rdieee_file(data, ndata, header, save);
        if (i) fatal_error_i("import_ieee, error %d", i);
        use_scale = 0;
    }
    return 0;
}

/*
 * HEADER:100:import_bin:misc:1:read binary file (X) for data
 */

/**
 * Import data from binary file.
 * 
 * Wgrib2 will decode the grib message and save the decoded grid point values in a floating 
 * point array (DATA). The -import options read grid point values from a specified file and 
 * replace the values of DATA. The size of DATA and imported grid should match. The -import 
 * options are often used to read data that is later written out as a grib message. 
 * 
 * Note that the import functions will reset the scaling and precision of the grib writing 
 * (new files) to the default (ECMWF-style, 12 bits). Any -set_metadata should be done after 
 * the -import functions. 
 * 
 * ## Scan Order
 * The grib message's scan order is called the "input" scan order (wgrib2 -grid). Wgrib2 
 * converts this to the "output" scan order. (This is the scan order for options like: -bin, 
 * -text, -cvs.) The import file needs to be in the "output" scan order. Of course, you can 
 * change the output scan order using the -order option. 
 * 
 * ## Import Format
 * The file that you import needs to be in a special format. 
 * 
 * - grib: grib2 message
 * - bin: native single point format
 * - ieee: IEEE single point format
 * - bin: may have a f77 style header depending on the -header option
 * - ieee: may have a f77 style header depending on the -header option
 * - ieee: may be little or big (default) ending depending on options
 * - text: may have a "nx ny" header depending on the -header option (see -text option)
 * 
 * ## Usage
 * -import_binary FILE
 * 
 * FILE is the binary (native format floating point) file to read.
 * 
 * @note Grid size (if it can be determined) must match the current grid.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example 
 * ???
 * 
 * @author Wesley Ebisuzaki @date 10/2008
 */
int f_import_bin(ARG1) {
    unsigned int i, j;
    struct seq_file *save;
    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("import_bin: memory allocation","");
        if (fopen_file(save, arg1, "rb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        decode = 1;
    }
    else if (mode == -2) {
        save = (struct seq_file *) *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = (struct seq_file *) *local;
        if (header) {
            if (fread_file((void *) &i, sizeof(unsigned int), 1, save) != 1) {
                fclose_file(save);
                free(save);
                fatal_error("import_bin: read error header (lack of data)","");
            }
            if (i != sizeof(float) * (size_t) ndata) {
                fclose_file(save);
                free(save);
                fatal_error("import_bin: header record size wrong","");
            }
        }
        j = fread_file(data, sizeof(float), ndata, save);
        if (j != ndata) fatal_error_uu("import_bin: read error read %u words, expected %u", j, ndata);
        if (header) {
            if (fread_file((void *) &i, sizeof(int), 1, save) != 1)
                fatal_error("import_bin: read error trailer","");
            if (i != sizeof(float) * ndata)
                fatal_error_u("import_bin: trailer record size wrong, %u", i);
        }
        use_scale = 0;                  // new field, unknown scaling
    }
    return 0;
}
