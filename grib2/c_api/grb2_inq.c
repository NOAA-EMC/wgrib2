#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "c_wgrib2api.h"

/* 3/2018 Public Domain, Wesley Ebisuzaki
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
 *  options = SEQUENTIAL | DATA | LATLON | WENS | RAW_ORDER | META | GRIDMETA | REGEX
 *
 * v0.99 3/2018
 */


static int last_options, good;
static unsigned int npnts, nx_, ny_;
static int submsg, msg_no, inv_no;

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
	    return 1;
	}
        if (options & RAW_ORDER) {
	    fprintf(stderr,"grb2_inq: WENS option cannot be used at same time as RAW_ORDER option\n");
	    return 1;
	}
        wgrib2_add_cmd("-order");
        wgrib2_add_cmd("we:ns");
    }

    /* RAW_ORDER - order of the data */
    if (options & RAW_ORDER) {
        if (options & LATLON) {
	    fprintf(stderr,"grb2_inq: RAW_ORDER option cannot be used at same time as LATLON option\n");
	    return 1;
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
    if (i) return 0;		/* failed call to wgrib2 */

    /* read basic parameters in register 19 */

    bufsize = sizeof(buffer);
    i = wgrib2_get_mem_buffer((unsigned char *) buffer, bufsize, 19);
    if (i != 0) return 0; /* something wrong .. probably not found */

    i = sscanf(buffer, "%11d %11u %11u %11u %11d %11d",&inv_no,&npnts,&nx_,&ny_,&msg_no, &submsg);
    printf(">>> wgrb2_scannf = %d\n",i);
    if (i != 6) return 0;

    if (inv_no > 1) return 0;

    /* finally success */
    good = 1;
//    fprintf(stderr,"good>>n=%d n=%u nx=%u ny=%u\n", inv_no, npnts,nx_, ny_);
    return (long long int) npnts;
}

int grb2_get_data(float *data, int ndata) {

    if (good == 0) {
	fprintf(stderr,"grb2_get_data: last find did not work.\n");
	return 1;
    }
    if (ndata != npnts) {
	fprintf(stderr,"grb2_get_data: wrong size data.\n");
	return 1;
    }
    if ((last_options & DATA) == 0) {
	fprintf(stderr,"grb2_get_data: grb2_inq did not request reading data.\n");
	return 1;
    }

    return  wgrib2_get_reg_data(data, ndata, 19);
}


int grb2_get_lonlat(float *lon, float *lat, int ndata) {
    int err1, err2;
    if (good == 0) {
	fprintf(stderr,"grb2_get_lonlat: last find did not work.\n");
	return 1;
    }
    if (ndata != npnts) {
	fprintf(stderr,"grb2_get_lonlat: wrong size data.\n");
	return 1;
    }
    if ((last_options & LONLAT) == 0) {
	fprintf(stderr,"grb2_get_lonalt: grb2_inq did not request reading lonlat.\n");
	return 1;
    }

    err1 = wgrib2_get_reg_data(lon, ndata, 17);
    err2 = wgrib2_get_reg_data(lat, ndata, 18);
    return err1 + err2;
}

int grb2_size_meta() {
    unsigned int size;

    if (good == 0) {
	fprintf(stderr,"grb2_size_meta: last find did not work.\n");
	return 0;
    }
    if ((last_options & META) == 0) {
	fprintf(stderr,"grb2_size_meta: grb2_inq did not request reading metadata.\n");
	return 0;
    }
    size = (unsigned int) wgrib2_get_mem_buffer_size(18);
    if (size == 0) return 0;
    return (int) (size + 1);
}

int grb2_get_meta(char *meta, int nbytes) {
    size_t size;
    int err;

    if (good == 0) {
	fprintf(stderr,"grb2_get_meta: last find did not work.\n");
	return 1;
    }
    if ((last_options & META) == 0) {
	fprintf(stderr,"grb2_get_meta: grb2_inq did not request reading metadata.\n");
	return 1;
    }

    size = wgrib2_get_mem_buffer_size(18);
    if (size == 0) {
	fprintf(stderr,"grb2_get_meta: size = 0, grib format error\n");
	return 1;
    }
    if (size > INT_MAX  || size+1 > (size_t) nbytes) {
	fprintf(stderr,"grb2_get_meta: size of metadata is too big.\n");
	return 1;
    }
    err = wgrib2_get_mem_buffer(meta, size, 18);
    if (err == 0) meta[size] = 0;	/* end the string */
    return err;
}

int grb2_size_gridmeta() {
    unsigned int size;

    if (good == 0) {
        fprintf(stderr,"grb2_size_gridmeta: last find did not work.\n");
        return 0;
    }
    if ((last_options & META) == 0) {
        fprintf(stderr,"grb2_size_gridmeta: grb2_inq did not request reading gridmetadata\n");
        return 0;
    }
    size = (unsigned int) wgrib2_get_mem_buffer_size(17);
    if (size == 0) return 0;
    return (int) (size + 1);
}

int grb2_get_gridmeta(char *meta, int nbytes) {
    size_t size;
    int err;

    if (good == 0) {
	fprintf(stderr,"grb2_get_gridmeta: last find did not work.\n");
	return 1;
    }
    if ((last_options & GRIDMETA) == 0) {
	fprintf(stderr,"grb2_get_gridmeta: grb2_inq did not request reading metadata.\n");
	return 1;
    }

    size = wgrib2_get_mem_buffer_size(17);
    if (size == 0) {
	fprintf(stderr,"grb2_get_gridmeta: size of gridmeta = 0, grid problem?.\n");
	return 1;
    }
    if (size > INT_MAX  || size+1 > (size_t) nbytes) {
	fprintf(stderr,"grb2_get_gridmeta: size of metadata is too big.\n");
	return 1;
    }
    err = wgrib2_get_mem_buffer(meta, size, 17);
    if (err == 0) meta[size] = 0;	/* end the string */
    return err;
}


