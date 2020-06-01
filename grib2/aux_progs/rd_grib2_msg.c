#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "grb2.h"
#include "wgrib2.h"

/*
 * rd_grib2_msg.c *                              Wesley Ebisuzaki
 *
 * unsigned char rd_grib2_msg(FILE *input, long int pos, *int len)
 *
 * to read grib2 message
 *
 *    msg = rd_grib_msg(input, position, &len)
 *
 *    input is the file
 *    position is the byte location (should be set to zero for first record 
 *    msg = location of message
 *
 *    to get next record: *  position = position + len;
 *
 * rd_grib_msg allocates its own buffer which can be seen by the
 * parsing routines
 *
 * 1/2007 cleanup M. Schwarb
 * 6/2009 fix repeated bitmaps W. Ebisuzuaki
 * 1/2011 added seq input W. Ebisuzaki
 */

#define BUFF_ALLOC0	(1024*64)
#define MSEEK 2048

static unsigned char *buffer = NULL, *Msg = NULL, *Sec[9], *Sec6_bitmap;
static long int buffer_size = 0;


/* ascii values of GRIB */
#define G       71
#define R       82
#define I       73
#define B       66

static int io_buffer[10], io_buffer_cnt = 0;	/* only need 8 bytes to id a grib file */
static long int pos_input = 0;
static int unget_input(int c);
static int get_input(FILE *input);

static int unget_input(int c) {
// if (io_buffer_cnt == sizeof(io_buffer) != io_buffer_cnt < 0)  fatal_error("programming error: unget_input","");
   if ((io_buffer_cnt == sizeof(io_buffer)) ||(io_buffer_cnt < 0))  fatal_error("programming error: unget_input","");
   io_buffer[io_buffer_cnt++] = c;
   pos_input--;
   return 0;
}

static int get_input(FILE *input) {
   pos_input++;
   if (io_buffer_cnt) return io_buffer[--io_buffer_cnt];
   return getc(input);
}


unsigned char *rd_grib2_msg_seq(FILE *input, long int *pos, unsigned long int *len, int *num_submsgs) {
    int i, j, c, c1, c2, c3, c4;
    long int len_grib;
    unsigned char *p, *end_of_msg;

    /* setup grib buffer */
    if (buffer == NULL) {
        if ((buffer = (unsigned char *) malloc(BUFF_ALLOC0)) == NULL) {
            fatal_error("not enough memory: rd_grib2_msg","");
        }
        buffer_size = BUFF_ALLOC0;
    }
    /* search for GRIB...2 */

    while (1) {
	c = get_input(input);
	if (c == EOF) { *len = 0; return NULL; }
	if (c != G) continue;
        if ( (c = get_input(input)) != R) { unget_input(c); continue; }
        if ( (c = get_input(input)) != I) { unget_input(c); continue; }
        if ( (c = get_input(input)) != B) { unget_input(c); continue; }
        c1 = get_input(input);
        c2 = get_input(input);
        c3 = get_input(input);
        c4 = get_input(input);
        if (c4 == 1) {
	    fprintf(stderr,"grib1 message ignored (use wgrib)\n");
	    continue;
	}
        if (c4 != 2) {
	    unget_input(c4);
	    unget_input(c3);
	    unget_input(c2);
	    unget_input(c1);
	    continue;
	}
	buffer[0] = G;
	buffer[1] = R;
	buffer[2] = I;
	buffer[3] = B;
	buffer[4] = c1;
	buffer[5] = c2;
	buffer[6] = c3;
	buffer[7] = c4;
	/* fill in the size 8-15, unget buffer is empty */
	for (i = 0; i < 8; i++) { 
	    buffer[8+i] = c = get_input(input);
	    if (c == EOF) {
	    	*len = 0;
		return NULL;
	    }
    	}
	break;
    }

    *len = len_grib = uint8(buffer+8);
    *pos = pos_input - 16;

    if (buffer_size < len_grib) {
        buffer_size = len_grib + len_grib / 10;
        buffer = (unsigned char *) realloc((void *) buffer, buffer_size);
    }

    if (io_buffer_cnt) fatal_error("rd_grib2_msg_seq: program error: io_buffer is not flushed","");
    j=fread(buffer+16, sizeof (unsigned char), len_grib-16, input);
    pos_input += j;
    
    if (j != len_grib-16) fatal_error("rd_grib2_msg_seq, read outside of file, bad grib file","");

    Msg = buffer;

    Sec[8] = Msg + len_grib - 4;
    if (Sec[8][0] != 55 || Sec[8][1] != 55 || Sec[8][2] != 55 || Sec[8][3] != 55) {
        fatal_error("rd_grib2_msg_seq, missing end section ('7777')","");
    }
    Sec[0] = Msg;

    /* scan message for number of submessages and perhaps for errors */
    p = Msg +  GB2_Sec0_size;
    end_of_msg = Msg + len_grib;

    i = 0;
    while (p < Sec[8]) {
        if (p[4] == 7) i++;
	if (uint4(p) < 5) fatal_error_i("rd_grib2_msg: illegal grib: section length, section %i", p[4]);
        p += uint4(p);
        if (p > end_of_msg) fatal_error("bad grib format","");
    }
    if (p != Sec[8]) {
        fatal_error("rd_grib2_msg: illegal format, end section expected","");
    }
    *num_submsgs = i;

    *len = len_grib;
    return Msg;
}

unsigned char *rd_grib2_msg(FILE *input, long int *pos, unsigned long int *len, int *num_submsgs){

    unsigned long int len_grib;
    long int position, i;
    unsigned long int tmp;
    long int n_bytes;
    int j;
    unsigned char *p, *end_of_msg;
    /* setup grib buffer */
    if (buffer == NULL) {
        if ((buffer = (unsigned char *) malloc(BUFF_ALLOC0)) == NULL) {
	    fatal_error("not enough memory: rd_grib2_msg","");
	}
        buffer_size = BUFF_ALLOC0;
    }

    /* find Msg and length of message */
#ifdef DEBUG
    fprintf(stderr,"rd_grib2_msg: start looking at pos %ld\n", *pos);
#endif
    position = *pos;
    Msg = seek_grib2(input, &position, &len_grib, buffer, MSEEK, &n_bytes);
#ifdef DEBUG
    fprintf(stderr,"rd_grib2_msg: found at %ld .. n_bytes %ld\n", position, n_bytes);
#endif
    *pos = position;

    if (Msg == NULL) {
        *len = 0;
	return NULL;
    }

// for (i = 0; i <4; i++) fprintf(stderr," (%c) ", Msg[i]);

    /* read all whole grib record .. to save I/O time, add to end of buffer */

    if (len_grib + Msg - buffer > buffer_size) {
	tmp = Msg - buffer;
        buffer_size = len_grib + Msg - buffer + 5000;
        buffer = (unsigned char *) realloc((void *) buffer, buffer_size);
        if (buffer == NULL) fatal_error("rd_grib2_msg: ran out of memory","");
	Msg = buffer + tmp;
    }

    if (fseek(input, *pos+n_bytes-(Msg-buffer), SEEK_SET) == -1) {
        fatal_error("rd_grib2_msg seek, outside the file, bad grib file","");
    }

    i = len_grib + Msg - buffer - n_bytes; 	/* no. of bytes need to read */
    if (i > 0 && ((j=fread(buffer+n_bytes, sizeof (unsigned char), i, input)) != i))  {
        fatal_error("rd_grib2_msg, read outside of file, bad grib file","");
    }

// for (i = 0; i <4; i++) fprintf(stderr," (%c) ", Msg[i+len_grib-4]);

    Sec[8] = Msg + len_grib - 4;
    if (Sec[8][0] != 55 || Sec[8][1] != 55 || Sec[8][2] != 55 || Sec[8][3] != 55) {
        fatal_error("rd_grib2_msg, missing end section ('7777')","");
    }
    Sec[0] = Msg;

    /* scan message for number of submessages and perhaps for errors */
    p = Msg +  GB2_Sec0_size;
    end_of_msg = Msg + len_grib;

    i = 0;
    while (p < Sec[8]) {
#ifdef DEBUG
    fprintf(stderr,"rd_grib2_msg: section %d found\n", p[4]);
#endif
	if (p[4] == 7) i++;
	if (uint4(p) < 5) fatal_error_i("rd_grib2_msg: illegal grib: section length, section %i", p[4]);
	p += uint4(p);
	if (p > end_of_msg) fatal_error("bad grib format","");
    }
    if (p != Sec[8]) {
	fatal_error("rd_grib2_msg: illegal format, end section expected","");
    }
    *num_submsgs = i;

    *len = len_grib;
#ifdef DEBUG
    fprintf(stderr,"rd_grib2_msg: found at %ld .. len_grib %ld\n", position, len_grib);
#endif
    return Msg;
}

/*
 * with grib 1, a message = 1 field
 * with grib 2, a message can have more than one field
 *
 * this routine parses a grib2 message that has already been read into buffer
 *
 * parse_1st_msg .. returns 1st message starting at Msg
 */ 

int parse_1st_msg(unsigned char **sec) {

	unsigned char *p, *end_of_msg;
	int i;
 
	if (Msg == NULL) fatal_error("parse_1st_msg .. Msg == NULL","");

	Sec[0] = Msg;
	Sec[1] = Sec[2] = Sec[3] = Sec[4] = Sec[5] = Sec[6] = Sec[7] = 
	Sec6_bitmap = NULL;
	end_of_msg = Msg + GB2_MsgLen(Sec);

	p = Msg + 16;

	while (Sec[8] - p > 0) {
	    if (p[4] > 8) fatal_error_i("parse_1st_msg illegal section %d", (int) p[4]);
	    Sec[p[4]] = p;

	    /* Section 6: bitmap */
	    if (p[4] == 6) {
		if (p[5] == 0) {
		    Sec6_bitmap = p;
		}
		else if (p[5] >= 1 && p[5] <= 253) {
	            fatal_error("parse_1st_msg: predefined bitmaps are not handled","");
		}
		else if (p[5] == 254) {
	            fatal_error("parse_1st_msg: illegal grib msg, bitmap not defined code, table 6.0=254","");
		}
	    }

	    /* last section */
	    if (p[4] == 7) {
		for (i = 0; i < 9; i++) {
		    sec[i] = Sec[i];
		}
		return 0;
	    }
	    p += uint4(p);
	    if (p > end_of_msg) fatal_error("bad grib fill","");
	}
	fatal_error("parse_1st_msg illegally format grib","");
	return 1;
}

int parse_next_msg(unsigned char **sec) {

	unsigned char *p, *end_of_msg;
	int i;
 
	end_of_msg = Msg + GB2_MsgLen(sec);
	p = sec[7];
	if (p[4] != 7) {
            fatal_error("parse_next_msg: parsing error","");
	}
	p += uint4(p);
	if (p > end_of_msg) fatal_error("bad grib fill","");

	while (p < Sec[8]) {
	    Sec[p[4]] = p;

	    // code to handle code table 6.0
	    if (p[4] == 6) {
		if (p[5] == 0) {
		    Sec6_bitmap = p;
		}
		else if (p[5] >= 1 && p[5] <= 253) {
	            fatal_error("parse_next_msg: predefined bitmaps are not handled","");
		}
		else if (p[5] == 254) {
	            if (Sec6_bitmap == NULL) {
	                fatal_error("parse_1st_msg: illegal grib msg, bitmap not defined code, table 6.0=254","");
		    }
		    Sec[6] = Sec6_bitmap;
		}
	    }
	    if (p[4] == 7) {		// end of message .. save on sec[]
		for (i = 0; i < 9; i++) {
		    sec[i] = Sec[i];
		}
		return 0;
	    }
	    p += uint4(p);
	    if (p > end_of_msg) fatal_error("bad grib fill","");
	}
	return 1;
}
