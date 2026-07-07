/*
 * find next grib header
 *
 * file = what do you think?
 * pos = initial position to start looking at  ( = 0 for 1st call)
 *       returns with position of next grib header (units=bytes)
 * len_grib = length of the grib record (bytes)
 * buffer[buf_len] = buffer for reading/writing
 *
 * returns (char *) to start of GRIB header+PDS
 *         NULL if not found
 *
 * adapted from SKGB (Mark Iredell)
 *
 * v1.1 9/94 Wesley Ebisuzaki
 * v1.2 3/96 Wesley Ebisuzaki handles short records at end of file
 * v1.3 8/96 Wesley Ebisuzaki increase NTRY from 3 to 100 for the folks
 *      at Automation decided a 21 byte WMO bulletin header wasn't long 
 *      enough and decided to go to an 8K header.  
 * v1.4 11/10/2001 D. Haalman, looks at entire file, does not try
 *      to read past EOF
 *      3/8/2010 echack added by Brian Doty
 * v1.5 5/2011 changes for ECMWF who have grib1+grib2 files, scan entire file
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "grib.h"
#include "pds4.h"

/* #define LEN_HEADER_PDS (28+42+100) */
#define LEN_HEADER_PDS (28+8)

int ec_large_grib = 0,  len_ec_bds;

unsigned char *seek_grib(FILE *file, unsigned long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len) {

    int i, len;
    long length_grib;
    static int warn_grib2 = 0;
    clearerr(file);
    while ( !feof(file) ) {

        if (fseek(file, *pos, SEEK_SET) == -1) break;
	i = fread(buffer, sizeof (unsigned char), buf_len, file);     
        if (ferror(file)) break;
        len = i - LEN_HEADER_PDS;
     
        for (i = 0; i < len; i++) {
            if (buffer[i] == 'G' && buffer[i+1] == 'R' && buffer[i+2] == 'I'
                && buffer[i+3] == 'B') {
		/* grib edition 1 */
		if (buffer[i+7] == 1) {
                    *pos = i + *pos;
                    *len_grib = length_grib = (buffer[i+4] << 16) + (buffer[i+5] << 8) +
                            buffer[i+6];

		    /* small records don't have ECMWF hack */
		    if ((length_grib & 0x800000) == 0) { ec_large_grib = 0; return (buffer + i); }

		    /* potential for ECMWF hack */
		    ec_large_grib = 1;
		    *len_grib = echack(file, *pos, length_grib);
                    return (buffer+i);
		}

		/* grib edition 2 */
		else if (buffer[i+7] == 2) {
		    if (warn_grib2++ == 0) fprintf(stderr,"grib2 message ignored (use wgrib2)\n");
		}

            }
        }
	*pos = *pos + (buf_len - LEN_HEADER_PDS);
    }

    *len_grib = 0;
    return (unsigned char *) NULL;
}


/* If the encoded grib record length is long enough, we may have an encoding
   of an even longer record length using the ecmwf hack.  To check for this
   requires getting the length of the binary data section.  To get this requires
   getting the lengths of the various sections before the bds.  To see if those
   sections are there requires checking the flags in the pds.  */

long echack(FILE *file, long pos, long len_grib) {

    int gdsflg, bmsflg, center;
    unsigned int pdslen, gdslen, bmslen, bdslen;
    unsigned char buf[8];
    long len;

    len = len_grib;

    /* Get pdslen */

    if (fseek(file, pos+8, SEEK_SET) == -1) return 0;
    if (fread(buf, sizeof (unsigned char), 8, file) != 8) return 0;
    pdslen = __LEN24(buf);

    center = buf[4];

    /* know that NCEP and CMC do not use echack */
    if (center == NMC || center == CMC) {
	ec_large_grib = 0;
        return len_grib;
    }


    gdsflg = buf[7] & 128;
    bmsflg = buf[7] & 64;

    gdslen=0;
    if (gdsflg) {
        if (fseek(file, pos+8+pdslen, SEEK_SET) == -1) return 0;
        if (fread(buf, sizeof (unsigned char), 3, file) != 3) return 0;
        gdslen = __LEN24(buf);
    }

    /* if there, get length of bms */

    bmslen = 0;
    if (bmsflg) {
       if (fseek(file, pos+8+pdslen+gdslen, SEEK_SET) == -1) return 0;
       if (fread(buf, sizeof (unsigned char), 3, file) != 3) return 0;
       bmslen = __LEN24(buf);
    }

    /* get bds length */

    if (fseek(file, pos+8+pdslen+gdslen+bmslen, SEEK_SET) == -1) return 0;
    if (fread(buf, sizeof (unsigned char), 3, file) != 3) return 0;
    bdslen = __LEN24(buf);

    /* Now we can check if this record is hacked */

    if (bdslen >= 120) {
	/* normal record */
	ec_large_grib = 0;
    }
    else {
        /* ECMWF hack */
        len_grib = (len & 0x7fffff) * 120 - bdslen + 4;
        len_ec_bds = len_grib - (12 + pdslen + gdslen + bmslen);
	ec_large_grib = 1;
    }
    return len_grib;
}
