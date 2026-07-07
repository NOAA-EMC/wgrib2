/** @file
 * @brief Converts an 8-byte unsigned integer from a byte array to an unsigned long integer.
 * @author Public Domain: Wesley Ebisuzaki  @date 2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "aux_progs.h"

/** For unit tests only. Sets ULONG_MAX to 32-bit max if FORCE_32BIT_TEST is defined. */
#ifdef FORCE_32BIT_TEST
#undef ULONG_MAX
#define ULONG_MAX 4294967295UL
#endif

/**
 * Converts an 8-byte unsigned integer from a byte array to an unsigned long integer.
 *
 * @param p Pointer to an array of at least 8 bytes representing the unsigned integer in big-endian order.
 *
 * @return The converted unsigned long integer.
 *
 * @note On 32-bit systems, if the value exceeds the maximum representable value of unsigned long int,
 *       the program will print an error message and exit.
 *
 * @author Wesley Ebisuzaki @date 2006
 */

unsigned long int uint8(const unsigned char *p) {

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
