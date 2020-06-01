#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

// extern int nx, ny, scan;
// extern unsigned int npnts;
// extern int save_translation;

extern int *raw_variable_dim;
extern enum output_order_type output_order_wanted, output_order;

static unsigned int n_translation = 0;
unsigned int *translation = NULL;

/*
 * undo the scan mode madness
 *
 * i, j is grid coordinate (0,0) is south west corner
 * nx, ny are grid dimensions
 * scan_mode is grib2 scan mode parameter
 *
 * returns integer 0 .. nx*ny-1 which is the location of the i, jth data
 *         -1 for error
 * 3/2008 public domain Wesley Ebisuzaki
 * 3/2008 bug fix Manfred Schwarb
 * 7/2009 bug fix Reinoud Bokhorst 
 * 12/2014 more arguments to  to_we_sn_scan(), to_we_ns_scan(), ij2p()
 *         old int to_we_sn_scan(float *data);
 *         ij2p: "if (scan == -1)" becomes "if (scan_mode == -1)"
 * 2/2016 ij2p: 2G*
 *        to_we_ns: lost line added, 2G*
 *        to_we_sn: lost line added, speedup , 2G*
 *
 */

/*
 * ij2p
 *  return pointer to data(i,j) (fortran convention)
 */

float *ij2p(unsigned int i, unsigned j, int scan_mode, unsigned int nx, unsigned int ny, float *data) {

    if (scan_mode == -1) return NULL;

    /* regular grid */
    if (nx > 0 && ny > 0) {
        if (i >= nx || j >= ny) return NULL;

        j = (scan_mode & 64) ? j : ny-1 - j;
        i = ((scan_mode & 16) && (j % 2 == 1)) ?  nx - 1 - i : i;
        i = (scan_mode & 128) ? nx-1 - i : i;

        return (scan_mode & 32) ?  data + j + i*ny : data + i + nx*j;
    }

    /* thinned longitudes or other non-grids */
    return NULL;
}

/*
 * to_we_sn_scan
 *    this routine converts scanning order to standard we:sn
 *    default for binary and text output
 */

int to_we_sn_scan(float *data, int scan, unsigned int npnts, int nx, int ny, int save_translation) {

    float *data2;
    int dx;
    float *p0, *p1, *p2;
    int lscan;
    unsigned int i, ix, iy;
    size_t row_size;

    if (scan == -1) return -1;
    lscan = scan >> 4;
    if (lscan == 4) {
	if (save_translation && translation) {
            free(translation);
            translation = NULL;
            n_translation = 0;
        }
	return 0; 			/* already we:sn order */
    }
    if (save_translation && npnts != n_translation) {
	free(translation);
	if ((translation = (unsigned int *) malloc(((size_t) npnts) * sizeof(int))) == NULL) {
	    fatal_error("translation: not enough memory for translation array","");
	}
	n_translation = npnts;
    }

    if (lscan == 0 && nx > 0 && ny > 0) {	/* regular grid: convert from we:ns to we:sn */
	row_size = ((size_t) nx) * sizeof(float);
        if ((data2 = (float *) malloc(row_size)) == NULL) 
	    fatal_error("translation: allocation of memory error","");
        for (iy = 0; iy < ny/2; iy++) {
	    memcpy(data2, data + ((size_t) iy) * nx, row_size);
	    memcpy(data + iy*nx, data + (ny-1-iy) * ((size_t) nx), row_size);
	    memcpy(data + (ny-1-iy) * ((size_t) nx), data2, row_size);
	}
	free(data2);
	if (save_translation) {
#pragma omp parallel for private(iy, ix)
	    for (iy = 0; iy < ny; iy++) {
		for (ix = 0; ix < nx; ix++) {
		   translation[ix + iy*((size_t) nx)] = ix + (ny-1-iy)*((size_t) nx);
		}
	    }
	}
	return 0;
    }


    if ((data2 = (float *) malloc( ((size_t) npnts) * sizeof(float))) == NULL) 
	fatal_error("translation: allocation of memory error","");
	
//    if (lscan == 0 && nx > 0 && ny > 0) {	/* regular grid: convert from we:ns to we:sn */
//	p0 = data;
//	p1 = data2 + npnts - nx;
//	row_size = ((size_t) nx) * sizeof(float);
//        for (iy = 0; iy < ny; iy++) {
//	    memcpy(p1, p0, row_size);
//	    if (save_translation) for (i = 0; i < nx; i++) translation[p1+i-data2] = p0 - data + i;
//	    p0 += nx;
//	    p1 -= nx;
//	}
//        memcpy(data, data2, ((size_t) npnts) * sizeof(float));
//	free(data2);
//	return 0;
//    }

    if (lscan == 0 && nx < 1 && ny > 0) { /* quasi-regular grid: convert from we:ns to we:sn */
	p0 = data;
	p1 = data2 + npnts;
        for (iy = 0; iy < ny; iy++) {
	    dx = raw_variable_dim[iy];
	    p1 -= dx;
	    memcpy(p1,p0,dx * sizeof(float));
	    if (save_translation) for (i = 0; i < dx; i++) translation[p1+i-data2] = p0 - data + i;
	    p0 += dx;
	}
        memcpy(data, data2, ((size_t) npnts) * sizeof(float));
	free(data2);
	return 0;
    }

    if (nx < 1 || ny < 1) {
	free(data2);
	fatal_error("not handled by to_we_sn_scan","");
	return 1;
    }

    p0 = data2;
    for (iy = 0; iy < ny; iy++) {
	p1 = ij2p(0,iy,scan,nx,ny, data);
	p2 = ij2p(1,iy,scan,nx,ny, data);
	dx = p2 - p1;
	for (ix = 0; ix < nx; ix++) {
	    if (save_translation) translation[p0 - data2] = p1 - data;
	    *p0++ = *p1;
	    p1 += dx;
	}
    }
    memcpy(data, data2, ((size_t) npnts) * sizeof(float));
    free(data2);
    return 0;
}

/*
 * to_we_ns_scan
 *    this routine converts scanning order to standard we:ns
 */

int to_we_ns_scan(float *data, int scan, unsigned int npnts, int nx, int ny, int save_translation) {

    float *data2;
    int dx;
    unsigned int ix, iy, i;
    float *p0, *p1, *p2;
    int lscan;

    if (scan == -1) return -1;
    lscan = scan >> 4;
    if (lscan == 0) {
	if (save_translation && translation) {
            free(translation);
            translation = NULL;
            n_translation = 0;
        }
	return 0;                   /* already we:ns order */
    }

    if (save_translation && npnts != n_translation) {
	free(translation);
	if ((translation = (unsigned int *) malloc(((size_t) npnts) * sizeof(int))) == NULL) {
	    fatal_error("translation: not enough memory for translation array","");
	}
	n_translation = npnts;
    }

    if ((data2 = (float *) malloc(((size_t) npnts) * sizeof(float))) == NULL)
        fatal_error("translation:allocation of memory error","");

    if (lscan == 0 && nx > 0 && ny > 0) {       /* regular grid: convert from we:sn to we:ns */
        p0 = data;
        p1 = data2 + npnts;
        for (iy = 0; iy < ny; iy++) {
            p1 -= nx;
            memcpy(p1, p0, nx * sizeof(float));
	    if (save_translation) for (i = 0; i < nx; i++) translation[p1+i-data2] = p0 - data + i;
            p0 += nx;
        }
        memcpy(data, data2, ((size_t) npnts) * sizeof(float));
        free(data2);
        return 0;
    }

    if (lscan == 0 && nx < 1 && ny > 0) { /* quasi-regular grid: convert from we:sn to we:ns */
        p0 = data;
        p1 = data2 + npnts;
        for (iy = 0; iy < ny; iy++) {
            dx = raw_variable_dim[iy];
            p1 -= dx;
            memcpy(p1,p0,dx * sizeof(float));
	    if (save_translation) for (i = 0; i < dx; i++) translation[p1+i-data2] = p0 - data + i;
            p0 += dx;
        }
        memcpy(data, data2, ((size_t) npnts) * sizeof(float));
        free(data2);
        return 0;
    }

    if (nx < 1 || ny < 1) {
        free(data2);
	fatal_error("not handled by to_we_ns_scan","");
	return 1;
    }

    /* uncommon scan order .. use general routine */

    p0 = data2;
    for (i = 0; i < ny; i++) {
	iy = ny - 1 - i;
        p1 = ij2p(0,iy,scan,nx,ny, data);
        p2 = ij2p(1,iy,scan,nx,ny, data);
        dx = p2 - p1;
        for (ix = 0; ix < nx; ix++) {
	    if (save_translation) translation[p0 - data2] = p1 - data;
            *p0++ = *p1;
            p1 += dx;
        }
    }
    memcpy(data, data2, ((size_t) npnts) * sizeof(float));
    free(data2);
    return 0;
}

/*
 * HEADER:200:order:setup:1:decoded data in X (raw|we:sn|we:ns) order, we:sn is default
 */


int f_order(ARG1) {
    if (mode == -1) {
	if (strcmp(arg1,"raw") == 0) output_order_wanted = raw;
	else if (strcmp(arg1,"we:sn") == 0) output_order_wanted = wesn;
	else if (strcmp(arg1,"we:ns") == 0) output_order_wanted = wens;
	else {
	    fatal_error("order: arg=%s expecting raw|we:sn|we:ns", arg1);
	}
    }
    return 0;
}

/*
 * returns a string with the name of the output_order
 */

const char *output_order_name(void) {
	if (output_order == raw) return "raw";
	if (output_order == wesn) return "WE:SN";
	if (output_order == wens) return "WE:NS";
	return "order?";
}

int undo_output_order(float *data, float *data_old_order, unsigned int npnts) {
    unsigned int i;
    if (translation == NULL) {
        memcpy(data_old_order, data, ((size_t) npnts) * sizeof(float));
	return 0;
    }
    if (npnts != n_translation) fatal_error("undo_output_order: program error", "");
    for (i = 0; i < npnts; i++) {
        data_old_order[translation[i]] = data[i];
    }
    return 0;
}
