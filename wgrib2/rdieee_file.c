#include <stdio.h>
#include <stddef.h>
#include "wgrib2.h"

/*
 * rdieee_file: reads a big/little endian file with optional header
 *
 * 10/2008 Public domain Wesley Ebisuzaki
 */

/* BSIZ number of floats to process at one time */
#define BSIZ 4096

extern int ieee_little_endian;

int rdieee_file(float *array, unsigned int n, int header, struct seq_file *input) {

    unsigned int i, j, l;
    size_t nbytes;
    unsigned char buff[BSIZ*4];
    unsigned char h4[4],t4[4];

    nbytes = n*4;

    if (header) {
	if (nbytes >> 32) fatal_error("rdieee: grid too large for 4 byte header","");
	if (fread_file(h4,1,4,input) != 4) fatal_error("rdieee: header read","");
	if (ieee_little_endian) 
	    l = (h4[3] << 24) | (h4[2] << 16) | (h4[1] << 8) | h4[0];
	else
	    l = (h4[0] << 24) | (h4[1] << 16) | (h4[2] << 8) | h4[3];
	
	if (l != nbytes) fatal_error("rdieee: bad header","");
    }

    while (n > 0) {
	j =  n < BSIZ ? n : BSIZ;
	if (fread_file(buff,1,4*j,input) != 4*j) fatal_error("rdieee: data read","");
	if (ieee_little_endian) swap_buffer(buff, 4*j);
#pragma omp parallel for private(i) schedule(static)
	for (i = 0; i < j; i++) {
	    array[i] = ieee2flt(buff + 4*i);
	}
	n = n - j;
    }

    if (header) {
	if (fread_file(t4,1,4,input) != 4) fatal_error("rdieee: trailer read","");
	if (h4[0] != t4[0] || h4[1] != t4[1] || h4[2] != t4[2] || h4[3] != t4[3])
	   fatal_error("rdieee: bad trailer","");
    }

    return 0;
}
