#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "wgrib2.h"

/*
 * a simple extension to fopen
 *
 * if file is already open, just return the pointer to the file handler
 *
 * public domain 2/2008 Wesley Ebisuzaki
 *  v1.1 WNE: upgrade: add read/write detection
 *		- can be stdin and stdout (in previous version)
 *		if pipe - set flush_mode, error checking
 *  v1.2 10/2014 John Howard .. add ffclose
 *               needs to keep track of number of ffopens of a file
 *               when number ffopens and ffclose are the same .. close file
 *
 *               adding ffcloses is needed to make wgrib2 a callable subroutine
 *                otherwise run out of file handles
 *
 *  v1.3 4/2015 WNE added @tmp:XXXX  temporary files, added for callable wgrib2
 *                        @mem:N
 */

extern int flush_mode;
struct opened_file
{
    char *name;
    FILE *handle;
    int is_read_file;
    int usage_count;
    int do_not_close_flag;
    struct opened_file *next;
};

/* memory buffers */
extern size_t mem_buffer_pos[N_mem_buffers];

/* do not initialize opened_file_start in init_globals */
static struct opened_file *opened_file_start = NULL;
/* do not initialize opened_file_start in init_globals */

FILE *ffopen(const char *filename, const char *mode)
{
    struct opened_file *ptr;
    struct stat stat_buf;  /* test for pipes */
    int is_read_file, i;
    const char *p;

    /* see if is a read/write file */
    is_read_file = 0;
    p = mode;
    while (*p) {
	if (*p++ == 'r') is_read_file = 1;
    }

    if (strcmp(filename,"-") == 0) {
	if (is_read_file) return stdin;
	flush_mode = 1;
	return stdout;
    }

    if (strncmp(filename,"@mem:",5) == 0) {
	fatal_error("option does not suport memory files %s", filename);
    }

    /* see if file has already been opened */
	
    ptr = opened_file_start;
    while (ptr != NULL) {
	if (strcmp(filename,ptr->name) == 0) {
	    if (ptr->usage_count == 0) {			/* reuse of opened file */
		if (is_read_file && !ptr->is_read_file) {	/* was write, now read */
		     i = fflush(ptr->handle);
		     if (i) fprintf(stderr,"WARNING: wgrib2 could not flush %s in write->read\n", ptr->name);
                     fseek(ptr->handle, 0L, SEEK_SET);		/* rewind file */
		}
		ptr->is_read_file = is_read_file;
	    }
	    if (is_read_file != ptr->is_read_file)  {
		fatal_error("ffopen: file can only read or write not both: %s", ptr->name);
	    }
	    (ptr->usage_count)++;
	    return ptr->handle;
	}
	ptr = ptr->next;
    }

    ptr = (struct opened_file *) malloc( sizeof(struct opened_file) );
    if (ptr == NULL) fatal_error("ffopen: memory allocation problem (%s)", filename);
    ptr->name = (char *) malloc(strlen(filename) + 1);
    if (ptr->name == NULL) fatal_error("ffopen: memory allocation problem (%s)", filename);

    strncpy(ptr->name, filename, strlen(filename)+1);
    ptr->is_read_file = is_read_file;
    ptr->usage_count = 1;
    ptr->do_not_close_flag = 1;		// default is not to close file file

    /* check if filename is @tmp:XXX or just XXX */
    if (strlen(filename) > 5 && strncmp(filename, "@tmp:", 5) == 0) {
	if (is_read_file) {
	    /* remove ptr */
	    free(ptr->name);
	    free(ptr);
	    fatal_error("ffopen: creating temporary file for read is not a good idea (%s)", filename);
	}
        ptr->handle = tmpfile();	/* ISO C to generate temporary file */
        if (ptr->handle == NULL) {
	    /* remove ptr */
	    free(ptr->name);
	    free(ptr);
	    fatal_error("ffopen: could not open tmpfile %s", filename);
	}
        /* add ptr to linked list */
        ptr->next = opened_file_start;
        opened_file_start = ptr;
        return ptr->handle;
    }

    /* disk file or pipe */
    ptr->handle = fopen(filename,mode);
    if (ptr->handle == NULL) {
	free(ptr->name);
	free(ptr);
	// fatal_error("ffopen: could not open %s", filename);
	return NULL;
    }

    /* check if output is to a pipe */
    if (is_read_file == 0) {
	if (stat(filename, &stat_buf) == -1) {
	    /* remove ptr */
	    free(ptr->name);
	    free(ptr);
	    fatal_error("ffopen: could not stat file: %s", filename);
	}
        if (S_ISFIFO(stat_buf.st_mode)) {
	    flush_mode  = 1;
	}
    }

    /* add ptr to linked list */
    ptr->next = opened_file_start;
    opened_file_start = ptr;
    return ptr->handle;
}

int ffclose(FILE *flocal)
{
    struct opened_file *ptr, *last_ptr;

    if (flocal == stdin || flocal == stdout) return 0;		// do not close stdin or stdout

    last_ptr = NULL;
    ptr = opened_file_start;
    while (ptr != NULL) {
	if (ptr->handle == flocal) {
	    if (ptr->usage_count <= 0) fatal_error_i("ffclose: usage_count = %d <= 0, %d", ptr->usage_count);
	    if (--ptr->usage_count == 0) {   // usage_count is zero, can close file name
	        if (ptr->do_not_close_flag == 1) return 0;
		if (last_ptr == NULL)
		    opened_file_start = ptr->next;
		else {
		    last_ptr->next = ptr->next;
		}
		fclose(ptr->handle);
		free(ptr->name);
		free(ptr);
	    }
	    return 0;
	}
	last_ptr = ptr;
	ptr = ptr->next;
    }
    return 1;	/* not found */
}

/*
 * make a file that has been opened with ffopen persistent
 *
 * with a callable wgrib2, may not want to close files for performance
 *
 */
int mk_file_persistent(const char *filename) {
    struct opened_file *ptr;

    ptr = opened_file_start;
    while (ptr != NULL) {
	if (strcmp(filename,ptr->name) == 0) {
	    ptr->do_not_close_flag = 1;
	    return 0;
	}
	ptr = ptr->next;
    }
    fprintf(stderr,"Warning persistant flag not set %s\n", filename);
    return 0;
}
int mk_file_transient(const char *filename) {
    struct opened_file *ptr;

    ptr = opened_file_start;
    while (ptr != NULL) {
	if (strcmp(filename,ptr->name) == 0) {
	    ptr->do_not_close_flag = 0;
	    return 0;
	}
	ptr = ptr->next;
    }
    fprintf(stderr,"Warning transient flag not set %s\n", filename);
    return 0;
}

/*
 * remove file from ffopen() linked lists
 */

int wgrib2_free_file(const char *filename) {
    struct opened_file *ptr, *last_ptr;

    // status_ffopen();
    ptr = opened_file_start;
    last_ptr = NULL;
    while (ptr != NULL) {
	if (strcmp(filename,ptr->name) == 0) {
	    if (ptr->usage_count == 0) {		/* can close file */
		if (last_ptr == NULL)
		    opened_file_start = ptr->next;
		else {
		    last_ptr->next = ptr->next;
		}
		fclose(ptr->handle);
		free(ptr->name);
		free(ptr);
	    }
	    else {
    		fprintf(stderr,"Warning: wgrib2_free_file %s usage count %d\n", filename, ptr->usage_count);
	    }
	    return 0;
	}
	last_ptr = ptr;
	ptr = ptr->next;
    }
    fprintf(stderr,"Warning wgrib2_free_file: %s not closed\n", filename);
    return 1;
}

/*
 * rewind filename
 */

int rewind_file(const char *filename) {
    struct opened_file *ptr;
    int i;

    if (strncmp(filename,"@mem:",5) == 0) {
        i = atoi(filename+5);
        if (i < 0 || i >= N_mem_buffers || (i == 0 && filename[5] != '0')) {
            fatal_error_i("rewind_file: @mem:N  N from 0..%d", N_mem_buffers-1);
        }
        mem_buffer_pos[i] = 0;
        return 0;
    }

    ptr = opened_file_start;
    while (ptr != NULL) {
	if (strcmp(filename,ptr->name) == 0) {
            if (fseek(ptr->handle, 0L, SEEK_SET))
		fatal_error("rewind_file: failed (%s)", filename);
	    return 0;
	}
	ptr = ptr->next;
    }
    return 1;
}


/*
 * status of ffopen/ffclose
 */

void status_ffopen(void) {
    struct opened_file *ptr;
    ptr = opened_file_start;
    while (ptr != NULL) {
        fprintf(stderr, "file: %s %c:%s file_offset=%ld usage=%d\n", ptr->name, ptr->is_read_file ? 'r' : 'w', 
		ptr->do_not_close_flag ? "perm":"tran", ftell(ptr->handle), ptr->usage_count);
	ptr = ptr->next;
    }
    return;
}
