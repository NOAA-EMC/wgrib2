/** @file
 * @brief Memory buffer(s) to store GRIB messages.
 * @author Public Domain: Wesley Ebisuzaki @date 5/2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "wgrib2.h"
#include "wgrib2_api.h"
#include "fnlist.h"

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

/** Data for \@mem:N */
unsigned char *mem_buffer[N_mem_buffers] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/**  File size for \@mem:N */
size_t mem_buffer_size[N_mem_buffers] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/** Allocated memory for \@mem:N >= buffer size */
size_t mem_buffer_allocated[N_mem_buffers] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/**  Buffer position. */
size_t mem_buffer_pos[N_mem_buffers] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/** Append grib file flag. */
extern int file_append;

/* 
   When we allocate or reallocate memory, want to request more than the 
   minimal needed memory so that we can avoid some reallocs. Also
   try to make it a multiple of because that is the page size on many machines.

   Probably better to make size slightly smaller than a N*4096 to account
   for malloc using some of page for its internal use.  Maybe later.
 */

/**
 * Calculate new size for memory buffer.
 * 
 * @param size The requested size.
 *
 * @return The new size for the memory buffer.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
 */
static size_t calc_new_size(size_t size)
{
    size_t n = size / 4096;
    return (n + 1 + n/10)*4096 + 16*4096;
}

/**
 * Initialize memory buffers.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
 */
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

/**
 * Free memory buffer n.
 * 
 * @param n The memory buffer number to free.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
 */
void free_mem_buffer(int n) {
    if (n < 0 || n >= N_mem_buffers) return;
    if (mem_buffer[n] != NULL) {
        free(mem_buffer[n]);
        mem_buffer[n] = NULL;
        mem_buffer_pos[n] = mem_buffer_allocated[n] = mem_buffer_size[n] = 0;
    }
    return;
}

/**
 * Create a new memory buffer.
 * 
 * @param n The memory buffer number to create.
 * @param size The size of the memory buffer to create.
 *
 * @return A pointer to the new memory buffer, or NULL on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
 */
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

/**
 * Reallocate memory buffer n.
 * 
 * Changes size of mem_buffer[n], but keep contents.
 *
 * @param n The memory buffer number to reallocate.
 * @param size The new size of the memory buffer.
 *
 * @return A pointer to the reallocated memory buffer, or NULL on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * fwrite for memory file n.
 * 
 * @param ptr The pointer to the data to write.
 * @param size The size of each element to write.
 * @param nmemb The number of elements to write.
 * @param n The memory buffer number to write to.
 *
 * @return The number of elements written, or 0 on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * fread for memory file n.
 *
 * @param ptr The pointer to the buffer to read data into.
 * @param size The size of each element to read.
 * @param nmemb The number of elements to read.
 * @param n The memory buffer number to read from.
 *
 * @return The number of elements read, or 0 on failure.
 *
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * fseek for memory file n.
 *
 * @param n The memory buffer number to seek in.
 * @param position The new position to seek to.
 * @param whence The reference point for the new position (SEEK_SET, SEEK_CUR, SEEK_END).
 *
 * @return 0 on success, -1 on failure.
 *
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * ftell for memory file n.
 *
 * @param n The memory buffer number to tell the position of.
 *
 * @return The current position in the memory buffer, or -1 on failure.
 *
 * @author Wesley Ebisuzaki @date 5/2015
 */
long ftell_mem(int n) {
    if (n < 0 || n >= N_mem_buffers) return -1;
    return mem_buffer_pos[n];
}

/**
 * fgets for memory file n.
 * 
 * @param s The buffer to read the line into.
 * @param size The size of the buffer.
 * @param n The memory buffer number to read from.
 *
 * @return A pointer to the buffer, or NULL on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * Get the size of the memory buffer n.
 *
 * @param n The memory buffer number to get the size of.
 * 
 * @return The size of the memory buffer, or 0 on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2015
 */
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

/**
 * Get the memory buffer n.
 *
 * @param my_buffer Pointer to the buffer to copy the memory into.
 * @param size The size of the buffer.
 * @param n The memory buffer number to get.
 * 
 * @return 
 * - 0 :: success
 * - 1 :: bad memory file number
 * - 2 :: wrong size
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * Set the memory buffer n.
 *
 * @param my_buffer Pointer to the buffer to copy the memory from.
 * @param size The size of the buffer.
 * @param n The memory buffer number to set.
 *
 * @return
 * - 0 :: success
 * - 1 :: bad memory file number
 * - 3 :: memory allocation problem
 *
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * Write memory file to specified output file at cleanup step.
 *
 * Wgrib2 supports memory files. Memory files are transient and only exist while wgrib2 is 
 * running. Memory files can be loaded prior to grib processing by the -mem_init option and 
 * written to disk after grib processing by the -mem_final option. Memory files are rarely 
 * used in interactive wgrib2 use.
 * 
 * ## HPC and CW2
 * Memory files was designed to support the reading and writing of grib files using CW2 in a 
 * HPC environment. Suppose you want to write a grib file with a 1000 grib messages. You let 
 * 1000 CPUS encode one grib message each. The encoding CPU may set the RPN register with the 
 * grid values. The calling wgrib2 to encode data and write the grib2 message to a memory file. 
 * After the call, the memory file can be read and sent to the cpus that are tasked with writing 
 * the 1000 grib messages to disk. 
 * 
 * ## Usage
 * -mem_final N FILE
 * 
 * N = 0 to 19
 * FILE = output file name
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * Read memory file X from file Y (on initialization).
 * 
 * Wgrib2 supports memory files. Memory files are transient and only exist while wgrib2 is 
 * running. Memory files can be loaded prior to grib processing by the -mem_init option and 
 * written to disk after grib processing by the -mem_final option. Memory files are rarely 
 * used in interactive wgrib2 use.
 * 
 * ## HPC and CW2
 * Memory files was designed to support the reading and writing of grib files using CW2 in a 
 * HPC environment. Suppose you want to write a grib file with a 1000 grib messages. You let 
 * 1000 CPUS encode one grib message each. The encoding CPU may set the RPN register with the 
 * grid values. The calling wgrib2 to encode data and write the grib2 message to a memory file. 
 * After the call, the memory file can be read and sent to the cpus that are tasked with writing 
 * the 1000 grib messages to disk. 
 * 
 * ## Usage
 * -mem_init N FILE
 * 
 * N = 0 to 19
 * FILE = input file name
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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

/**
 * Delete a memory file.
 * 
 * The -mem_del option is intended for the use by callable wgrib2 so that memory files can be deleted 
 * and the memory freed. 
 * 
 * Wgrib2 supports memory files. Memory files are transient and only exist while wgrib2 is running. 
 * Memory files can be loaded prior to grib processing by the -mem_init option, writen to disk after 
 * grib processing by the -mem_final option and deleted during the processing phase by the -mem_del 
 * option. Memory files can be used with the wgrib2 utility but were designed for use by callable 
 * wgrib2 (CW2). 
 * 
 * ## HPC and CW2
 * Memory files was designed to support the reading and writing of grib files using CW2 in a 
 * HPC environment. Suppose you want to write a grib file with a 1000 grib messages. You let 
 * 1000 CPUS encode one grib message each. The encoding CPU may set the RPN register with the 
 * grid values. The calling wgrib2 to encode data and write the grib2 message to a memory file. 
 * After the call, the memory file can be read and sent to the cpus that are tasked with writing 
 * the 1000 grib messages to disk. 
 * 
 * ## Usage
 * -mem_del N
 * 
 * N = 0 to 19
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 5/2015
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


