/** @file
 * @brief Reads a GRIB2 message from input file and writes it to output file. Supports gmerge.c.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 05/2009 | W. Ebisuzaki | Initial
 * 12/2025 | A. Stahl | Moved from gmerge.c to support testing
 * @author Public Domain: Wesley Ebisuzaki @date 05/2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aux_progs.h"

/** Maximum Buffer Size */
#define BSIZE 4096*8

/**
 * Reads a GRIB2 message from the input file and writes it to the output file.
 *
 * @param in Pointer to the input file.
 * @param out Pointer to the output file.
 *
 * @return 0 on success, non-zero on error.
 *
 * @author Wesley Ebisuzaki @date 05/2009
 */
int rd_msg(FILE *in, FILE *out) {
    long unsigned int n;
    int i,j,k;
    unsigned char header[BSIZE];

    if (feof(in)) return -1;

    i = fread(header, 1, 16, in);
    if (i != 16) return -1;
    if (header[0] != 'G' || header[1] != 'R' || header[2] != 'I' || 
        header[3] != 'B') return -1;

    n = uint8(&(header[8]));

    j = n < BSIZE ? n : BSIZE;
    k = fread(header+16,1,j-16,in);
    if (k != j-16) return -1;

    fwrite(header,1,j,out);
    n -= j;

    while (n) {
        j = n < BSIZE ? n : BSIZE;
        k = fread(header,1,j,in);
        fwrite(header,1,j,out);
        n -= j;
    }
    return 0;
}
