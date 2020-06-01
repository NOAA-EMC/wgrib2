#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include "wgrib2.h"

/* 6/2009 public domain 	wesley ebisuzaki
 *
 * code taken from wgrib 
 *
 *  takes a bitstream -> vector of unsigned ints
 *  bitstream starts on a byte boundary
 *  for 32 bit machine:  nbits <= 25
 *
 * bitstream (n_bits/uint) -> u[0..n-1]
 *
 * v1.1  limit to nbits is now 32 (32 bit integer), rd_bitstream_offset -> rd_bitstream
 */

static unsigned int ones[]={0, 1,3,7,15, 31,63,127,255};

void rd_bitstream(unsigned char *p, int offset, int *u, int n_bits, int n) {

    unsigned int tbits;
    int i, t_bits, new_t_bits;

    // not the best of tests

    if (INT_MAX <= 2147483647 && n_bits > 31)
                fatal_error_i("rd_bitstream: n_bits is %d", n_bits);

    if (offset < 0 || offset > 7) fatal_error_i("rd_bitstream: illegal offset %d",offset);

    if (n_bits == 0) {
        for (i = 0; i < n; i++) {
            u[i] = 0;
        }
        return;
    }

    t_bits = 8 - offset;
    tbits = (*p++) & ones[t_bits];

    for (i = 0; i < n; i++) {

        while (n_bits - t_bits >= 8) {
            t_bits += 8;
            tbits = (tbits << 8) | *p++;
        }

        if (n_bits > t_bits) {
            new_t_bits = 8 - (n_bits - t_bits);
            u[i]  = (int) ( (tbits << (n_bits - t_bits) | (*p >> new_t_bits) ));
            t_bits = new_t_bits;
            tbits = *p++ & ones[t_bits];
        }
        else if (n_bits == t_bits) {
            u[i]  = (int) tbits;
            tbits = t_bits = 0;
        }
        else {
            t_bits -= n_bits;
            u[i] = (int) (tbits >> t_bits);
            tbits = tbits & ones[t_bits];
        }
    }
}

/*
 * void rd_bitstream_flt
 *   rd_bitstream_flt() is like rd_bitstream() except that returns a float instead of int
 */

void rd_bitstream_flt(unsigned char *p, int offset, float *u, int n_bits, int n) {

    unsigned int tbits;
    int i, t_bits, new_t_bits;

    // not the best of tests

    if (INT_MAX <= 2147483647 && n_bits > 31)
                fatal_error_i("rd_bitstream: n_bits is %d", n_bits);

    if (offset < 0 || offset > 7) fatal_error_i("rd_bitstream_flt: illegal offset %d",offset);

    if (n_bits == 0) {
        for (i = 0; i < n; i++) {
            u[i] = 0.0;
        }
        return;
    }

    t_bits = 8 - offset;
    tbits = (*p++) & ones[t_bits];

    for (i = 0; i < n; i++) {

        while (n_bits - t_bits >= 8) {
            t_bits += 8;
            tbits = (tbits << 8) | *p++;
        }

        if (n_bits > t_bits) {
            new_t_bits = 8 - (n_bits - t_bits);
            u[i]  = (int) ( (tbits << (n_bits - t_bits) | (*p >> new_t_bits) ));
            t_bits = new_t_bits;
            tbits = *p++ & ones[t_bits];
        }
        else if (n_bits == t_bits) {
            u[i]  = (float) tbits;
            tbits = t_bits = 0;
        }
        else {
            t_bits -= n_bits;
            u[i] = (float) (tbits >> t_bits);
            tbits = tbits & ones[t_bits];
        }
    }
}

/*
 * make a bitstream with variable length packing
 *
 * n_bits should be <= 25
 *
 * last byte is zero packed
 *
 * start: init_bitstream(buffer)
 *
 * to write: add_bitstream(data, number of bits to write)
 *
 * to close (zero fill):  finish_bitstream()
 */

static unsigned char *bitstream;
static int rbits, reg, n_bitstream;

void add_bitstream(int t, int n_bits) {
    unsigned int jmask;

    if (n_bits > 16) {
        add_bitstream(t >> 16, n_bits - 16);
        n_bits = 16;
    } 
    if (n_bits > 25) fatal_error_i("add_bitstream: n_bits = (%d)",n_bits);
    jmask = (1 << n_bits) - 1;
    rbits += n_bits;
    reg = (reg << n_bits) | (t & jmask);
    while (rbits >= 8) {
	*bitstream++ = (reg >> (rbits = rbits-8)) & 255;
	n_bitstream++;
    }
    return;
}
void add_many_bitstream(int *t, int n, int n_bits) {
    unsigned int jmask, tt;
    int i;

    if (n_bits > 25) fatal_error_i("add_many_bitstream: n_bits = (%d)",n_bits);
    jmask = (1 << n_bits) - 1;

    for (i = 0; i < n; i++) {
	tt = (unsigned int) *t++;
        rbits += n_bits;
        reg = (reg << n_bits) | (tt & jmask);

        while (rbits >= 8) {
	    rbits -= 8;
	    *bitstream++ = (reg >> rbits) & 255;
            n_bitstream++;
	}

/*

        if (rbits == 32) {
	    rbits = 0;
	    ss = reg;
	    bitstream[0] = (ss >> 24) & 255;
	    bitstream[1] = (ss >> 16) & 255;
	    bitstream[2] = (ss >> 8) & 255;
	    bitstream[3] = ss & 255;
	    bitstream += 4;
            n_bitstream += 4;
	}
	else if (rbits >= 24) {
	    rbits = rbits - 24;
	    ss = reg >> rbits;
	    bitstream[0] = (ss >> 16) & 255;
	    bitstream[1] = (ss >> 8) & 255;
	    bitstream[2] = ss & 255;
	    bitstream += 3;
            n_bitstream += 3;
	}
	else if (rbits >= 16) {
	    rbits = rbits - 16;
	    ss = reg >> rbits;
	    bitstream[1] = ss & 255;
	    bitstream[0] = (ss >> 8) & 255;
	    bitstream += 2;
            n_bitstream += 2;
	}
	else if (rbits >= 8) {
	    rbits = rbits-8;
	    *bitstream++ = (reg >> rbits) & 255;
            n_bitstream++;
	}
*/
    }
    return;
}
void init_bitstream(unsigned char *new_bitstream) {
    bitstream = new_bitstream;
    n_bitstream = reg = rbits = 0;
    return;
}

void finish_bitstream(void) {
    if (rbits) {
	n_bitstream++;
        *bitstream++ = (reg << (8-rbits)) & 255;
	rbits = 0;
    }
    return;
}
