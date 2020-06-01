#include <stdio.h>
#include <stddef.h>
#include "wgrib2.h"

/* wesley ebisuzaki v1.3
 *
 * write ieee file -- big endian format
 *
 * input float *array		data to be written
 *	 int n			size of array
 *	 int header		1 for f77 style header 0 for none
 *				(header is 4 byte header
 *	 FILE *output		output file
 *
 * v1.2 7/97 buffered, faster
 * v1.3 2/99 fixed (typo) error in wrtieee_header found by
 *     Bob Farquhar
 * v1.4 3/2008 w. ebisuzaki added little-endian output
 * v1.5 11/2013 w. ebisuzaki remove h4[] to cleanup not ititialized warning
 * v1.6 7/2015 w. ebisuzaki OpenMP support, write to fwrite_file, bigger buffer
 * v1.7 12/2017 w. ebisuzaki: size(float) -> 4  
 */

/* BSIZ has to be a multiple of 4 */

#define BSIZ (64u*1024u*4u)

extern int ieee_little_endian;

int wrtieee(float *array, unsigned int n, int header, struct seq_file *out) {

	unsigned int i, j, l, nbuf, loop;
	unsigned char buff[BSIZ];

	nbuf = 0;
	if (header) {
		if (n >= 4294967295U / 4) 	// size(ieee) == 4
			fatal_error("wrtieee: grid too large for 4-byte header","");
		l = n * 4;

		buff[nbuf  ] = (l >> 24) & 255;
		buff[nbuf+1] = (l >> 16) & 255;
		buff[nbuf+2] = (l >>  8) & 255;
		buff[nbuf+3] = l         & 255;
		nbuf += 4;
	}
	i = 0;
	while (i < n) {
		loop = (BSIZ - nbuf)/4;
		loop  = (n-i) > loop ? loop : (n-i);
#pragma omp parallel for private(j) schedule(static)
		for (j = 0 ; j < loop; j++) {
		    flt2ieee(array[i+j], buff + nbuf + j*4);
		}
		i += loop;
		nbuf += 4*loop;

		if (nbuf >= BSIZ) {		// nbuf should never be > BSIZ
		    if (ieee_little_endian) swap_buffer(buff, BSIZ);
		    fwrite_file(buff, 1, BSIZ, out);
		    nbuf = 0;
		}
	}
	if (header) {
		l = n * 4;
		buff[nbuf  ] = (l >> 24) & 255;
		buff[nbuf+1] = (l >> 16) & 255;
		buff[nbuf+2] = (l >>  8) & 255;
		buff[nbuf+3] = l         & 255;
		nbuf += 4;
	}
	if (nbuf) {
	    if (ieee_little_endian) swap_buffer(buff, nbuf);
	    fwrite_file(buff, 1, nbuf, out);
	}
	return 0;
}
