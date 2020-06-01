#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "grb2.h"

unsigned long int uint8(unsigned char *p) {

#if (ULONG_MAX == 4294967295UL) 
	if (p[0] || p[1] || p[2] || p[3]) {
		fprintf(stderr,"unsigned value (8 byte integer) too large for machine\n");
		fprintf(stderr,"fatal error .. run on 64-bit machine\n");
		exit(8);
	}
	return  ((unsigned long int)p[4] << 24) + ((unsigned long int)p[5] << 16) + 
                ((unsigned long int)p[6] << 8) + (unsigned long int)p[7];
#else
	return  ((unsigned long int)p[0] << 56) + ((unsigned long int)p[1] << 48) + 
                ((unsigned long int)p[2] << 40) + ((unsigned long int)p[3] << 32) + 
                ((unsigned long int)p[4] << 24) + ((unsigned long int)p[5] << 16) +
		((unsigned long int)p[6] << 8) + (unsigned long int)p[7];
#endif
}
