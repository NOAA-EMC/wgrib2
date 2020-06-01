#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "grb2.h"
#include "wgrib2.h"

/*
 * rd_grib2_msg_seq_file.c *                              Wesley Ebisuzaki
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


/* ascii values of GRIB */
#define G       71
#define R       82
#define I       73
#define B       66

/* memory buffers */
extern unsigned char *mem_buffer[N_mem_buffers];
extern size_t mem_buffer_size[N_mem_buffers];
extern size_t mem_buffer_pos[N_mem_buffers];


/*
 * fopen_open_file
 *
 * ffopens a file
 *  sets up structure for reading the file
 *
 */

int fopen_file(struct seq_file *file, const char *filename, const char *open_mode) {
    struct stat stat_buf;
    int i;

    file->unget_cnt = 0;
    file->pos = 0;
    file->buffer = NULL;

    if (strcmp("-",filename) == 0) {
	if (open_mode[0] == 'r') {
	    file->cfile = stdin;
            file->file_type = PIPE;
	    return 0;
	}
	if (open_mode[0] == 'w') {
	    file->cfile = stdout;
            file->file_type = PIPE;
	    return 0;
	}
	fatal_error("fopen_file, programming error unexpected open mode (%s)", open_mode);
    }

    if (strncmp(filename,"@mem:",5) == 0) {
        i = atoi(filename+5);
        if (i < 0 || i >= N_mem_buffers || (i == 0 && filename[5] != '0')) {
            fatal_error_i("fopen_file: @mem:N  N from 0..%d", N_mem_buffers-1);
        }
	file->cfile = NULL;
        file->file_type = MEM;
	file->n_mem_buffer = i;
	// mem_buffer_pos[i] = 0;
	if (open_mode[0] == 'w') {
	    free_mem_buffer(i);
	    mem_buffer_pos[i] = 0;
	}
	else if (open_mode[0] == 'a') {
	    mem_buffer_pos[i] = mem_buffer_size[i];
	}
	return 0;
    }

    file->file_type = NOT_OPEN;
    file->cfile = ffopen(filename,open_mode);
    if (file->cfile == NULL) return 1;
    if (strncmp(filename,"@tmp:",5) == 0) {
        file->file_type = DISK;
        return 0;
    }

    if (stat(filename, &stat_buf) != -1) {
	if (S_ISREG(stat_buf.st_mode)) {
            file->file_type = DISK;
        }
        else if (S_ISDIR(stat_buf.st_mode)) {
                fatal_error("grib input is a directory: %s",filename);
        }
        else if (S_ISCHR(stat_buf.st_mode)) {
            file->file_type = PIPE;
        }
        else if (S_ISBLK(stat_buf.st_mode)) {
            file->file_type = PIPE;
        }
        else if (S_ISFIFO(stat_buf.st_mode)) {
            file->file_type = PIPE;
        }
        else {
            fprintf(stderr, "stat() function had unknown type file type, assumed DISK\n");
            file->file_type = DISK;
        }
    }
    else {
        fprintf(stderr, "stat() function did no work, assumed to be a disk file\n");
        file->file_type = DISK;
    }
    return 0;
}

void fclose_file(struct seq_file *file) {

    /* do not close stdin or stdout */
    if (file->cfile == stdin || file->cfile == stdout) return;

    if (file->file_type == PIPE || file->file_type == DISK) ffclose(file->cfile);
    if (file->buffer != NULL) {
        if (file->file_type == PIPE || file->file_type == DISK)  {
	    free(file->buffer);
	    file->buffer = NULL;
	    file->buffer_size = 0;
	}
    }
    file->file_type = NOT_OPEN;
    return;
}

int fseek_file(struct seq_file *file, long position, int whence) {

    file->unget_cnt = 0;
    if (file->file_type == DISK) {
	if (fseek(file->cfile, position, whence) == -1) return -1;
        if (whence == SEEK_SET) file->pos = position;
	else file->pos = ftell(file->cfile);
    }
    else if (file->file_type == MEM) {
	if (fseek_mem(file->n_mem_buffer, position, whence) == -1) return -1;
	file->pos = ftell_mem(file->n_mem_buffer);
    }
    else {
	fatal_error("fseek_file: programming error","");
    }
    return 0;
}

long ftell_file(struct seq_file *file) {
    if (file->file_type == DISK) return ftell(file->cfile);
    if (file->file_type == PIPE) return file->pos;
    if (file->file_type == MEM) return ftell_mem(file->n_mem_buffer);
    return -1;
}

char *fgets_file(char *s, int size, struct seq_file *file) {
    if (file->file_type == DISK || file->file_type == PIPE) return fgets(s, size, file->cfile);
    if (file->file_type == MEM) return fgets_mem(s, size, file->n_mem_buffer);
    return NULL;
}

/*
 *  only this routine handles multiple ungetc
 */

static int getchar_seq_file(struct seq_file *file) {
    file->pos++;
    if (file->unget_cnt) { return file->unget_buf[--file->unget_cnt]; }
    return fgetc_file(file);
}

static int unget_seq_file(struct seq_file *file, int c) {
    file->pos--;
    file->unget_buf[file->unget_cnt++] = (unsigned char) c;
    return 0;
}

size_t fread_file(void *ptr, size_t size, size_t nmemb, struct seq_file *file) {
    size_t iret;

    if (file->file_type == DISK || file->file_type == PIPE) {
	iret = fread(ptr, size, nmemb, file->cfile);
	if (ferror(file->cfile)) fprintf(stderr,"\nWARNING: file read error\n");
	return iret;
    }
    if (file->file_type == MEM) return fread_mem(ptr, size, nmemb, file->n_mem_buffer);
    return 0;
}


int fgetc_file(struct seq_file *file) {
    int n;
    if (file->file_type == DISK || file->file_type == PIPE) return fgetc(file->cfile);
    if (file->file_type == MEM) {
	n = file->n_mem_buffer;
	if (mem_buffer_size[n] > mem_buffer_pos[n]) {
	    return mem_buffer[n][mem_buffer_pos[n]++];
	}
     }
     return EOF;
}

size_t fwrite_file(const void *ptr, size_t size, size_t nmemb, struct seq_file *file) {
    size_t iret;

// fprintf(stderr,"in fwrite_file\n");
    if (file->file_type == DISK || file->file_type == PIPE) {
// fprintf(stderr,"in fwrite_file  disk or pipe >> fwrite\n");
	iret = fwrite(ptr, size, nmemb, file->cfile);
// fprintf(stderr,"in fwrite_file  <<fwrite ret=%lu\n", iret);
	if (ferror(file->cfile)) fatal_error("write error","");
	if (feof(file->cfile)) fatal_error("write error (EOF)","");
	return iret;
    }
    if (file->file_type == MEM) return fwrite_mem(ptr, size, nmemb, file->n_mem_buffer);
    return (size_t) 0;
}

void fflush_file(struct seq_file *file) {
   if (file->file_type == DISK || file->file_type == PIPE) fflush(file->cfile);
   return;
}

unsigned char *rd_grib2_msg_seq_file(unsigned char **sec, struct seq_file *input, long int *pos, 
        unsigned long int *len, int *num_submsgs) {
    int i, c, c1, c2, c3, c4;
    size_t j, len_grib;
    unsigned char *p, *end_of_msg;

    /* setup grib buffer */
    if (input->buffer == NULL) {
        if ((input->buffer = (unsigned char *) malloc(BUFF_ALLOC0)) == NULL) {
            fatal_error("not enough memory: rd_grib2_msg","");
        }
        input->buffer_size = BUFF_ALLOC0;
    }

    /* search for GRIB...2 */

    while (1) {
	c = getchar_seq_file(input);
	if (c == EOF) { *len = 0; return NULL; }
	if (c != G) continue;
        if ( (c = getchar_seq_file(input)) != R) { unget_seq_file(input, c); continue; }
        if ( (c = getchar_seq_file(input)) != I) { unget_seq_file(input, c); continue; }
        if ( (c = getchar_seq_file(input)) != B) { unget_seq_file(input, c); continue; }
        c1 = getchar_seq_file(input);
        c2 = getchar_seq_file(input);
        c3 = getchar_seq_file(input);
        c4 = getchar_seq_file(input);
        if (c4 == 1) {
	    fprintf(stderr,"grib1 message ignored (use wgrib)\n");
	    continue;
	}
        if (c4 != 2) {
	    unget_seq_file(input, c4);
	    unget_seq_file(input, c3);
	    unget_seq_file(input, c2);
	    unget_seq_file(input, c1);
	    continue;
	}
	input->buffer[0] = G;
	input->buffer[1] = R;
	input->buffer[2] = I;
	input->buffer[3] = B;
	input->buffer[4] = c1;
	input->buffer[5] = c2;
	input->buffer[6] = c3;
	input->buffer[7] = c4;

	/* fill in the size 8-15, unget buffer is empty */

	for (i = 0; i < 8; i++) { 
	    input->buffer[8+i] = c = getchar_seq_file(input);
	    if (c == EOF) {
	    	*len = 0;
		return NULL;
	    }
    	}
	break;
    }

    *len = len_grib = uint8(input->buffer+8);

    *pos = input->pos - 16;

    if (input->buffer_size < len_grib) {
        input->buffer_size = (len_grib*5)/4;
        input->buffer = (unsigned char *) realloc((void *) input->buffer, input->buffer_size);
    }

    j=fread_file(input->buffer+16, sizeof (unsigned char), len_grib-16, input);
    input->pos += j;
    
    if (j != len_grib-16) fatal_error("rd_grib2_msg_seq_file, read outside of file, bad grib file","");

    sec[0] = input->buffer;
    sec[8] = sec[0] + len_grib - 4;
    if (sec[8][0] != 55 || sec[8][1] != 55 || sec[8][2] != 55 || sec[8][3] != 55) {
        fatal_error("rd_grib2_msg_seq_file, missing end section ('7777')","");
    }

    /* scan message for number of submessages and perhaps for errors */
    p = sec[0] +  GB2_Sec0_size;
    end_of_msg = sec[0] + len_grib;

    i = 0;
    while (p < sec[8]) {
        if (p[4] == 7) i++;
	if (uint4(p) < 5) fatal_error_i("rd_grib2_msg_file: illegal grib: section length, section %i", p[4]);
        p += uint4(p);
        if (p > end_of_msg) fatal_error("bad grib format","");
    }
    if (p != sec[8]) {
        fatal_error("rd_grib2_msg: illegal format, end section expected","");
    }
    *num_submsgs = i;

    *len = len_grib;
    return sec[0];
}
