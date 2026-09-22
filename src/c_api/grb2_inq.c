/** @file
 * @brief Inquiry routines for the C API.
 * 
 * This is part of c_wgrib2api: grb2_inq(...)
 *
 * To support a variable number of search strings,
 * a C99 is required:
 *    __VA_ARGS__
 * To use a C compiler that doesn't support this C99 feature
 *   1) remove "#define grb2_inq(...) grb2_inqVA(__VA_ARGS__, NULL)"
 *      from c_wgrib2api.h
 *   2) Change calls to grb2_inq(...) to grb2_inqVA(...,NULL)
 *   The call code to grb2_inqVA() will work in C99 systems.
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 3/2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "c_wgrib2api.h"

size_t wgrib2_get_mem_buffer_size(int n);
int wgrib2_get_mem_buffer(unsigned char *my_buffer, size_t size, int n);
int wgrib2_get_reg_data(float *data, size_t size, int reg);

/** Last options used. */
static int last_options;

/** Flag indicating if the inquiry was successful. */
static int good;

/** Number of points in grid. */
static unsigned int npnts;

/** Number of grid points in the x direction. */
static unsigned int nx_;

/** Number of grid points in the y direction. */
static unsigned int ny_;

/** Sub-message number. */
static int submsg;

/** Message number. */
static int msg_no;

/** Inventory number. */
static int inv_no;

/**
 * This function performs an inquiry of a grib message.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes
 * 
 * @param grb Input grib file.
 * @param inv Inventory of input file.
 * @param options Bitwise OR of option flags.
 * options = SEQUENTIAL | DATA | LATLON | WENS | RAW_ORDER | META | GRIDMETA | REGEX
 * @param ... Additional optional arguments.
 *
 * @return The number of points in the grid, error code otherwise
 * - 0 :: Failure in wgrib2_get_mem_buffer_size()
 * - -1 :: Conflicting options (e.g., WENS and LATLON used together)
 * - -2 :: Failed to call wgrib2
 * - -3 :: Error retrieving memory buffer
 * - -4 :: Error parsing basic grid info
 * - -5 :: Inventory number greater than 1
 * 
 * @return The number of points in the grid, or 0 if an error occurred.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
long long int grb2_inqVA(const char *grb, const char *inv, unsigned int options, ... ) {

    va_list valist;
    char *search;
    int i;
    size_t bufsize;
    char buffer[71];

    good = 0;
    last_options = options;

    wgrib2_init_cmds();
    wgrib2_add_cmd(grb);
    wgrib2_add_cmd("-i_file");
    wgrib2_add_cmd(inv);
    if (options & SEQUENTIAL) {
        wgrib2_add_cmd("-end");
    }
    else {
        wgrib2_add_cmd("-rewind_init");
        wgrib2_add_cmd(inv);
    }


//    wgrib2_add_cmd("-inv");
//    wgrib2_add_cmd("/dev/null");

    va_start(valist, options);
    search = (char *) va_arg(valist, char * );
    while (search) {
        wgrib2_add_cmd( (options & REGEX) ? "-egrep" : "-fgrep" );
        wgrib2_add_cmd(search);
        search = (char *) va_arg(valist, char * );
    }
    va_end(valist);

    /* basic grid info */
    wgrib2_add_cmd("-ftn_api_fn0");
    wgrib2_add_cmd("-last0");
    wgrib2_add_cmd("@mem:19");

    /* grid */
    if (options & DATA) {
        wgrib2_add_cmd("-rpn_sto");
        wgrib2_add_cmd("19");
    }

    /* latlon */
    if (options & LATLON) {
        wgrib2_add_cmd("-rpn");
        wgrib2_add_cmd("rcl_lon:sto_17:rcl_lat:sto_18");
    }

    /* WENS - order of the data */
    if (options & WENS) {
        if (options & LATLON) {
            fprintf(stderr,"grb2_inq: WENS option cannot be used at same time as LATLON option\n");
            return -1;
        }
        if (options & RAW_ORDER) {
            fprintf(stderr,"grb2_inq: WENS option cannot be used at same time as RAW_ORDER option\n");
            return -1;
        }
        wgrib2_add_cmd("-order");
        wgrib2_add_cmd("we:ns");
    }

    /* RAW_ORDER - order of the data */
    if (options & RAW_ORDER) {
        if (options & LATLON) {
            fprintf(stderr,"grb2_inq: RAW_ORDER option cannot be used at same time as LATLON option\n");
            return -1;
        }
        wgrib2_add_cmd("-order");
        wgrib2_add_cmd("raw");
    }

    /* metadata */
    if (options & META) {
        wgrib2_add_cmd("-S");
        wgrib2_add_cmd("-last0");
        wgrib2_add_cmd("@mem:18");
    }

    /* grid metadata */
    if (options & GRIDMETA) {
        wgrib2_add_cmd("-grid");
        wgrib2_add_cmd("-last0");
        wgrib2_add_cmd("@mem:17");
    }

    wgrib2_list_cmd();

    i = wgrib2_cmd();
    if (i) return -2;		/* failed call to wgrib2 */

    /* read basic parameters in register 19 */

    bufsize = sizeof(buffer);
    i = wgrib2_get_mem_buffer((unsigned char *) buffer, bufsize, 19);
    if (i != 0) return -3; /* something wrong .. probably not found */

    i = sscanf(buffer, "%11d %11u %11u %11u %11d %11d",&inv_no,&npnts,&nx_,&ny_,&msg_no, &submsg);
    printf(">>> wgrb2_scannf = %d\n",i);
    if (i != 6) return -4;

    if (inv_no > 1) return -5;

    /* finally success */
    good = 1;
//    fprintf(stderr,"good>>n=%d n=%u nx=%u ny=%u\n", inv_no, npnts,nx_, ny_);
    return (long long int) npnts;
}

/**
 * Get memory-copy of grid data from RPN register.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes
 * 
 * @param data Pointer to the data array.
 * @param ndata Number of data points.
 * 
 * @return 
 * - 0 :: success
 * - 10 :: last find did not work
 * - 11 :: wrong size data
 * - 12 :: grb2_inq did not request reading data
 * 
 * See documentation for wgrib2_get_reg_data() for additional error codes.
 * 
 * @note Grid data stored in register 19.
 * 
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_get_data(float *data, int ndata) {

    if (good == 0) {
        fprintf(stderr,"grb2_get_data: last find did not work.\n");
        return 10;
    }
    if (ndata != npnts) {
        fprintf(stderr,"grb2_get_data: wrong size data.\n");
        return 11;
    }
    if ((last_options & DATA) == 0) {
        fprintf(stderr,"grb2_get_data: grb2_inq did not request reading data.\n");
        return 12;
    }

    return  wgrib2_get_reg_data(data, ndata, 19);
}

/**
 * Get memory-copy of longitude and latitude data from RPN registers.
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes
 * 
 * @param lon Pointer to the longitude array.
 * @param lat Pointer to the latitude array.
 * @param ndata Number of data points.
 *
 * @return 
 * - 0 :: success
 * - 10 :: last find did not work
 * - 11 :: wrong size data
 * - 12 :: grb2_inq did not request reading lonlat
 * 
 * See documentation for wgrib2_get_reg_data() for additional error codes. This function
 * returns the sum of the error codes from reading the longitude and latitude registers.
 *
 * @note Longitude and latitude data stored in registers 17 and 18, respectively.
 * 
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_get_lonlat(float *lon, float *lat, int ndata) {
    int err1, err2;
    if (good == 0) {
        fprintf(stderr,"grb2_get_lonlat: last find did not work.\n");
        return 10;
    }
    if (ndata != npnts) {
        fprintf(stderr,"grb2_get_lonlat: wrong size data.\n");
        return 11;
    }
    if ((last_options & LONLAT) == 0) {
        fprintf(stderr,"grb2_get_lonlat: grb2_inq did not request reading lonlat.\n");
        return 12;
    }

    err1 = wgrib2_get_reg_data(lon, ndata, 17);
    err2 = wgrib2_get_reg_data(lat, ndata, 18);
    return err1 + err2;
}

/**
 * Get the size of the memory buffer for metadata stored in RPN register.
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes
 * 
 * @return Size of the memory buffer for metadata on success, error code otherwise
 * - 0 :: Failure in wgrib2_get_mem_buffer_size()
 * - -1 :: last find did not work
 * - -2 :: grb2_inq did not request reading metadata
 * 
 * @note Metadata stored in register 18.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_size_meta(void) {
    unsigned int size;

    if (good == 0) {
        fprintf(stderr,"grb2_size_meta: last find did not work.\n");
        return -1;
    }
    if ((last_options & META) == 0) {
        fprintf(stderr,"grb2_size_meta: grb2_inq did not request reading metadata.\n");
        return -2;
    }
    size = (unsigned int) wgrib2_get_mem_buffer_size(18);
    if (size == 0) return 0;
    return (int) (size + 1);
}

/**
 * Get memory-copy of metadata from RPN register.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes
 * 
 * @param meta Pointer to the metadata array.
 * @param nbytes Size of the metadata buffer.
 * 
 * @return 
 * - 0 :: success
 * - 10 :: last find did not work
 * - 11 :: grb2_inq did not request reading metadata
 * - 12 :: grib format error
 * - 13 :: size of metadata is too big
 * 
 * See documentation for wgrib2_get_mem_buffer() for additional error codes.
 * 
 * @note Metadata stored in register 18.
 * 
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_get_meta(unsigned char *meta, int nbytes) {
    size_t size;
    int err;

    if (good == 0) {
        fprintf(stderr,"grb2_get_meta: last find did not work.\n");
        return 10;
    }
    if ((last_options & META) == 0) {
        fprintf(stderr,"grb2_get_meta: grb2_inq did not request reading metadata.\n");
        return 11;
    }

    size = wgrib2_get_mem_buffer_size(18);
    if (size == 0) {
        fprintf(stderr,"grb2_get_meta: size = 0, grib format error\n");
        return 12;
    }
    if (size > INT_MAX  || size+1 > (size_t) nbytes) {
        fprintf(stderr,"grb2_get_meta: size of metadata is too big.\n");
        return 13;
    }
    err = wgrib2_get_mem_buffer(meta, size, 18);
    if (err == 0) meta[size] = 0;	/* end the string */
    return err;
}

/**
 * Get the size of the memory buffer for grid metadata stored in RPN register.
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes + replaced META w/ GRIDMETA
 * 
 * @return Size of the memory buffer for grid metadata on success, error code otherwise
 * - 0 :: Failure in wgrib2_get_mem_buffer_size()
 * - -1 :: last find did not work
 * - -2 :: grb2_inq did not request reading grid metadata
 *
 * @note Grid metadata stored in register 17.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_size_gridmeta(void) {
    unsigned int size;

    if (good == 0) {
        fprintf(stderr,"grb2_size_gridmeta: last find did not work.\n");
        return -1;
    }
    if ((last_options & GRIDMETA) == 0) {
        fprintf(stderr,"grb2_size_gridmeta: grb2_inq did not request reading gridmetadata\n");
        return -2;
    }
    size = (unsigned int) wgrib2_get_mem_buffer_size(17);
    if (size == 0) return 0;
    return (int) (size + 1);
}

/**
 * Get memory-copy of grid metadata from RPN register.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2018 | W. Ebisuzaki | Initial
 * 7/2026 | A. Stahl | New error codes for testing purposes
 * 
 * @param meta Pointer to the grid metadata array.
 * @param nbytes Size of the grid metadata buffer.
 * 
 * @return 
 * - 0 :: success
 * - 10 :: last find did not work
 * - 11 :: grb2_inq did not request reading grid metadata
 * - 12 :: size of grid metadata = 0, grid problem?
 * - 13 :: size of metadata is too big
 * 
 * @note Grid metadata stored in register 17.
 * 
 * @author Wesley Ebisuzaki @date 3/2018
 */
int grb2_get_gridmeta(unsigned char *meta, int nbytes) {
    size_t size;
    int err;

    if (good == 0) {
        fprintf(stderr,"grb2_get_gridmeta: last find did not work.\n");
        return 10;
    }
    if ((last_options & GRIDMETA) == 0) {
        fprintf(stderr,"grb2_get_gridmeta: grb2_inq did not request reading metadata.\n");
        return 11;
    }

    size = wgrib2_get_mem_buffer_size(17);
    if (size == 0) {
        fprintf(stderr,"grb2_get_gridmeta: size of gridmeta = 0, grid problem?.\n");
        return 12;
    }
    if (size > INT_MAX  || size+1 > (size_t) nbytes) {
        fprintf(stderr,"grb2_get_gridmeta: size of metadata is too big.\n");
        return 13;
    }
    err = wgrib2_get_mem_buffer(meta, size, 17);
    if (err == 0) meta[size] = 0;	/* end the string */
    return err;
}

/**
 * For testing purposes only. Set the state of the static variables last_options, good, and npnts.
 *
 * @param last_options_val The value to set for last_options. Bitwise OR of option flags used in last inquiry.
 * options = SEQUENTIAL | DATA | LATLON | WENS | RAW_ORDER | META | GRIDMETA | REGEX
 * @param good_val The value to set for good. It is a flag indicating if last inquiry was successful. (0 = failure, otherwise success)
 * @param npnts_val The value to set for npnts. It represents the number of data points in the last inquiry.
 * 
 * @author Alyson Stahl @date 7/2026
 */
void grb2_inq_set_state(int last_options_val, int good_val, int npnts_val) {
    last_options = last_options_val;
    good = good_val;
    npnts = npnts_val;
}
