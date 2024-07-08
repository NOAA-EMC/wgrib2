#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "wgrib2_api.h"
#include "fnlist.h"

/*
 * mem_buffers.c    5/2015 Public Domain Wesley Ebisuzaki
 *
 * memory buffer(s) to store grib messages
 *
 */

/*
 * want memory buffers defined before wgrib2 is run because of callable wgrib2.
 * A program could setup a memory buffer, and then call wgrib2().
 *   ex. set @mem:0 to a grib message
 *       call wgrib2 to decode grib message
 *       get @mem:1 with the decoded grib message
 *
 * to check for compile-use init_mem_buffers
 */

/*
 * data structure for memory buffers
 *
 * unsigned char *mem_buffer[N_mem_buffers];		// data for @mem:n
 * size_t mem_buffer_size[N_mem_buffers];		// file size of @mem:N
 * size_t mem_buffer_allocated[N_mem_buffers];		// allocated memory for @mem:N >= buffer size
 * size_t mem_buffer_pos[N_mem_buffers];		// position
 *
 * to reduce calls to realloc/malloc, more memory is allocated than needed for the write
 */


unsigned char *mem_buffer[N_mem_buffers] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

size_t mem_buffer_size[N_mem_buffers] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
size_t mem_buffer_allocated[N_mem_buffers] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
size_t mem_buffer_pos[N_mem_buffers] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

extern int file_append;

/* 
   When we allocate or reallocate memory, want to request more than the 
   minimal needed memory so that we can avoid some reallocs. Also
   try to make it a multiple of because that is the page size on many machines.

   Probably better to make size slightly smaller than a N*4096 to account
   for malloc using some of page for its internal use.  Maybe later.
 */

static size_t calc_new_size(size_t size)
{
    size_t n = size / 4096;
    return (n + 1 + n/10)*4096 + 16*4096;
}

void init_mem_buffers(void) {
    if (sizeof(mem_buffer) / sizeof (unsigned char *) != N_mem_buffers) 
	fatal_error("Mem_buffer.c: mem_buffer improper initialization","");
    if (sizeof(mem_buffer_size) / sizeof (size_t) != N_mem_buffers) 
	fatal_error("Mem_buffer.c: mem_buffer_size improper initialization","");
    if (sizeof(mem_buffer_allocated) / sizeof (size_t) != N_mem_buffers) 
	fatal_error("Mem_buffer.c: mem_buffer_allocated improper initialization","");
    if (sizeof(mem_buffer_pos) / sizeof (size_t) != N_mem_buffers) 
	fatal_error("Mem_buffer.c: mem_buffer_pos improper initialization","");
    return;
}

void free_mem_buffer(int n) {
    if (n < 0 || n >= N_mem_buffers) return;
    if (mem_buffer[n] != NULL) {
	free(mem_buffer[n]);
	mem_buffer[n] = NULL;
	mem_buffer_pos[n] = mem_buffer_allocated[n] = mem_buffer_size[n] = 0;
    }
    return;
}

unsigned char *new_mem_buffer(int n, size_t size) {
    size_t new_size;

    if (n < 0 || n >= N_mem_buffers) return NULL;

    /* use already allocated buffer */
    if (mem_buffer[n] != NULL && size <= mem_buffer_allocated[n]) {
	mem_buffer_size[n] = size;
	mem_buffer_pos[n] = 0;
	return mem_buffer[n];
    }

    /* find new size and allocate memory for buffer */
    if (mem_buffer[n] != NULL) {
        free(mem_buffer[n]);
    }
    new_size = calc_new_size(size);
    mem_buffer[n] = (unsigned char *) malloc(new_size);
    if (mem_buffer[n]) {
	mem_buffer_size[n] = size;
	mem_buffer_allocated[n] = new_size;
    }
    else {
	mem_buffer_size[n] = mem_buffer_allocated[n] = 0;
    }
    mem_buffer_pos[n] = 0;
    return mem_buffer[n];
}

/*
 * realloc_mem_buffer(n, size)
 *   change mem_buffer[n] to size
 *   keep contents
 */

unsigned char *realloc_mem_buffer(int n, size_t size) {
    size_t new_size;
    unsigned char *p;
    if (n < 0 || n >= N_mem_buffers) return NULL;
    new_size = calc_new_size(size);
    if (mem_buffer[n] == NULL) {
        mem_buffer[n] = (unsigned char *) malloc(new_size);
        if (mem_buffer[n]) {
	    mem_buffer_size[n] = size;
	    mem_buffer_allocated[n] = new_size;
        }
        else {
	    mem_buffer_pos[n] = mem_buffer_size[n] = mem_buffer_allocated[n] = 0;
        }
    }
    else {
        if ((p = realloc(mem_buffer[n], new_size)) == NULL) {
	    free(mem_buffer[n]);		
	    mem_buffer_pos[n] = mem_buffer_size[n] = mem_buffer_allocated[n] = 0;
	}
	else {
            mem_buffer_size[n] = size;
            mem_buffer_allocated[n] = new_size;
	}
        mem_buffer[n] = p;
    }
    return mem_buffer[n];
}

/*
 * to support memory files, need standard i/o routine
 * such as fread, fwrite, fseek, ftell, fgets
 */


/*
 * fwrite for memory file n
 */
size_t fwrite_mem(const void *ptr, size_t size, size_t nmemb, int n) {
    size_t nwrite, new_size;

    if (n < 0 || n >= N_mem_buffers) return 0;

    nwrite = size * nmemb;
    new_size = mem_buffer_pos[n] + nwrite;

    if (new_size > mem_buffer_allocated[n]) {
        realloc_mem_buffer(n, new_size);
    }
    if (mem_buffer[n] == NULL) return (size_t) 0;
    memcpy(mem_buffer[n] + mem_buffer_pos[n], ptr, nwrite);
    mem_buffer_size[n] = mem_buffer_pos[n] = new_size;
    return nmemb;
}

/*
 * fread for memory file n
 */

size_t fread_mem(void *ptr, size_t size, size_t nmemb, int n) {
    size_t nread, i;

    if (n < 0 || n >= N_mem_buffers) return 0;

    nread = (mem_buffer_size[n] - mem_buffer_pos[n]) / size;
    if (nread > nmemb) nread = nmemb;
    i = nread * size;
    memcpy((void *) ptr, (void *) (mem_buffer[n] + mem_buffer_pos[n]), i);
    mem_buffer_pos[n] += i;
    return nread;
}

/*
 * fseek for memory file n
 */
 
int fseek_mem(int n, long position, int whence) {

    if (n < 0 || n >= N_mem_buffers) return -1;
    if (whence == SEEK_SET) {
        mem_buffer_pos[n] = position;
    }
    else if (whence == SEEK_END) {
        mem_buffer_pos[n] = mem_buffer_size[n] + position;
    }
    else if (whence == SEEK_CUR) {
        mem_buffer_pos[n] += position;
    }
    if (mem_buffer_pos[n] > mem_buffer_size[n]) {
	mem_buffer_pos[n] = 0;
	return -1;
    } 
    return 0;
}

/*
 * ftell for memory file n
 */

long ftell_mem(int n) {
    if (n < 0 || n >= N_mem_buffers) return -1;
    return mem_buffer_pos[n];
}

/*
 * fgets for memory file n
 */

char *fgets_mem(char *s, int size, int n) {
    char *p;

    if (n < 0 || n >= N_mem_buffers) return NULL;
    p = s;
    while (size > 1 && (mem_buffer_pos[n] < mem_buffer_size[n]) ) {
	size--;
	if ( (*p++ = mem_buffer[n][mem_buffer_pos[n]++]) == '\n') break;
    }
    *p = '\0';
    return s;
}

size_t wgrib2_get_mem_buffer_size(int n) {
    if (n < 0 || n >= N_mem_buffers) return 0;
    return mem_buffer_size[n];
}

/*
 * int wgrib2_get_mem_buffer
 *  return 0  :  good
 *         1  :  bad memory file number
 *         2  :  wrong size
 */

int wgrib2_get_mem_buffer(unsigned char *my_buffer, size_t size, int n) {
    if (n < 0 || n >= N_mem_buffers) return 1;
    if (size != mem_buffer_size[n]) return 2;
    memcpy(my_buffer, mem_buffer[n], size);
    return 0;
}

/*
 * int wgrib2_set_mem_buffer
 *  return 0  :  good
 *         1  :  bad memory file number
 *         3  :  memory allocation problem
 */
int wgrib2_set_mem_buffer(const unsigned char *my_buffer, size_t size, int n) {
    if (n < 0 || n >= N_mem_buffers) return 1;
    if (size == 0) {
        mem_buffer_size[n] = 0;
    }
    else {
	if (size > mem_buffer_allocated[n]) {
	    if (mem_buffer[n] != NULL) free(mem_buffer[n]);
	    mem_buffer[n] = (unsigned char *) malloc(size);
	    if (mem_buffer[n] == NULL) return 3;
	    mem_buffer_allocated[n] = size;
        }
	memcpy(mem_buffer[n], my_buffer, size);
	mem_buffer_size[n] = size;
    }
    return 0;
}

/*
 * HEADER:100:mem_final:misc:2:write mem file X to file Y at cleanup step
 */

int f_mem_final(ARG2) {
    int i, n;
    i = 0;
    if (mode == -1) {
        n = atoi(arg1);
        if (n < 0 || n >= N_mem_buffers) fatal_error_i("mem_final: n should be 0..%d", N_mem_buffers-1);
        *local = (void *) ffopen(arg2, file_append ? "a+b" : "w+b");
        if (*local == NULL) fatal_error("Could not open %s", arg2);
    }
    else if (mode == -2) {
        n = atoi(arg1);
        if (mem_buffer_size[n] > 0) {
            i = fwrite(mem_buffer[n], sizeof(unsigned char), mem_buffer_size[n], (FILE *) *local) != 
		mem_buffer_size[n];
            ffclose((FILE *) *local);
	}
    }
    return i;
}

/*
 * HEADER:100:mem_init:misc:2:read mem file X from file Y (on initialization)
 */

int f_mem_init(ARG2) {
    FILE *in;
    int n;
    long size;

    if (mode == -1) {
	n = atoi(arg1);
	if (n < 0 || n >= N_mem_buffers) fatal_error_i("mem_init: n should be 0..%d", N_mem_buffers-1);
	in = fopen(arg2,"rb");
	if (in == NULL) fatal_error("Could not open %s", arg2);
	fseek(in, 0L, SEEK_END);
	size = ftell(in);
	fseek(in, 0L, SEEK_SET);
	if (new_mem_buffer(n, (size_t) size) == NULL) {
	    fclose(in);
	    fatal_error("Could not allocate memory mem_init %s", arg2);
	}
	if (fread(mem_buffer[n], sizeof(unsigned char), size, in) != size) {
	    fclose(in);
            mem_buffer_size[n] = 0;
	    fatal_error("Could not read %s", arg2);
	}
	fclose(in);
    }
    return 0;
}
/*
 * HEADER:100:mem_del:misc:1:delete mem file X
 */

int f_mem_del(ARG1) {
    int n;
    if (mode >= 0) {
	n = atoi(arg1);
	if (n < 0 || n >= N_mem_buffers) fatal_error_i("mem_del: illegal memory buffer %d", n);
	/* probably going to reuse memory file, so keep buffers allocated */
	if (mem_buffer[n] == NULL) return 0;
        mem_buffer_size[n] = 0;
        mem_buffer_pos[n] = 0;
    }
    return 0;
}


