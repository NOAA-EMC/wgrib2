/*
 * Submsg_uv
 *
 * Put vector fields into same submessage .. for multiprocessing
 *  based on NCEP_uv.c
 *  
 * to alter the U/V list, use -new_grid_vectors
 *
 * U and V must be adjacent
 *
 * 1/2015 Public Domain Wesley Ebisuzaki
 * 10/2019    Wesley Ebisuzaki, added fflush_file()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int file_append, flush_mode;
extern const char **vectors;

/*
   is_u:
   returns name of v field or NULL
 */

static const char *is_u(const char *uname) {
    int i;

    i = 0;
    while (vectors[i] != NULL) {
	if (strcmp(vectors[i], uname) == 0) {
	    return vectors[i + 1];
	}
	i += 2;
    }
    return NULL;
}

/*
 * HEADER:111:submsg_uv:output:1:combine vector fields into one message
 */

int f_submsg_uv(ARG1) {

    struct local_struct {
        unsigned char *sec[9];
	const char *vname;
	struct seq_file out;
    };

    struct local_struct *save;
    int i, j, is_v;
    size_t size;
    char name[NAMELEN];
    unsigned char s[16];

    if (mode == -1) {		// initialization

        // allocate static structure

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct) );
        if (save == NULL) fatal_error("submsg_uv: memory allocation","");
        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
	save->vname = NULL;
        init_sec(save->sec);
	return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2)  {		// cleanup
	if (save->vname != NULL) {		// write out cached field
	   i = wrt_sec(save->sec[0], save->sec[1], save->sec[2], save->sec[3], 
		save->sec[4], save->sec[5], save->sec[6], save->sec[7], &(save->out));
	   if (flush_mode) fflush_file(&(save->out));
	   if (i) fatal_error_i("submsg_uv: last record problem %i",i);
           free_sec(save->sec);
	}
	fclose_file(&(save->out));
        free(save);
	return 0;
    }

    if (mode >= 0 )  {					// processing
        i = getName(sec, mode, NULL, name, NULL, NULL);

	/* see if name == expected vname */
	is_v = 0;
	if (save->vname != NULL && strcmp(name, save->vname) == 0) {
	    is_v = 1;
            if (same_sec0(sec,save->sec) == 0) is_v = 0;
            if (same_sec1(sec,save->sec) == 0) is_v = 0;
            if (same_sec2(sec,save->sec) == 0) is_v = 0;
            if (same_sec3(sec,save->sec) == 0) is_v = 0;

	    /* PDT must be same except of name parameters */

	    i = GB2_ParmNum(sec);
	    j = GB2_ParmCat(sec);
	    GB2_ParmNum(sec) = GB2_ParmNum(save->sec);
	    GB2_ParmCat(sec) = GB2_ParmCat(save->sec);
            if (same_sec4(sec,save->sec) == 0) is_v = 0;
	    GB2_ParmNum(sec) = i;
	    GB2_ParmCat(sec) = j;
	}

	/* finished tests for U/V sequence */

	/* is U/V sequence */
	if (is_v) {
	    size = (size_t) GB2_Sec0_size + GB2_Sec8_size +
	    (sec[1] ? uint4(sec[1]) : 0) +
	    (sec[2] ? uint4(sec[2]) : 0) +
	    (sec[3] ? uint4(sec[3]) : 0) +
	    (sec[4] ? uint4(sec[4]) : 0) +
	    (sec[5] ? uint4(sec[5]) : 0) +
	    (sec[6] ? uint4(sec[6]) : 0) +
	    (sec[7] ? uint4(sec[7]) : 0) +
	    (save->sec[4] ? uint4(save->sec[4]) : 0) +
	    (save->sec[5] ? uint4(save->sec[5]) : 0) +
	    (save->sec[6] ? uint4(save->sec[6]) : 0) +
	    (save->sec[7] ? uint4(save->sec[7]) : 0);

	    /* write out sec[0] for combined grib message */
	    for (i = 0; i < 8; i++) s[i] = sec[0][i];
            uint8_char(size, s+8);
            if (fwrite_file((void *) s, sizeof(char), 16, &(save->out)) != 16) {
		if (flush_mode) fflush_file(&(save->out));
		return 1;
	    }

	    /* write out sec[1] .. sec[7] for 1st message */
	    for (j = 1; j <= 7; j++) {
                if (save->sec[j]) {
                    i = uint4(save->sec[j]);
                    if (fwrite_file((void *)save->sec[j], sizeof(char), i, &(save->out)) != i) {
			if (flush_mode) fflush_file(&(save->out));
			return 1;
		    }
                }
	    }

	    /* write out sec[4] .. sec[7] for 2nd message */
	    for (j = 4; j <= 7; j++) {
                if (sec[j]) {
                    i = uint4(sec[j]);
                    if (fwrite_file((void *)sec[j], sizeof(char), i, &(save->out)) != i) {
			if (flush_mode) fflush_file(&(save->out));
			return 1;
		    }
                }
	    }

	    /* write out sec[8] .. end of grib message */
            s[0] = s[1] = s[2] = s[3] = 55; /* s = "7777" */
            if (fwrite_file((void *) s, sizeof(char), 4, &(save->out)) != 4) {
		   if (flush_mode) fflush_file(&(save->out));
	           fatal_error("submsg_uv: write record problem","");
	    }
	    if (flush_mode) fflush_file(&(save->out));

	    save->vname = NULL;
            free_sec(save->sec);
            return 0;
	}

	/* has U but not V, write U */    

	if (save->vname != NULL) {
	   i = wrt_sec(save->sec[0], save->sec[1], save->sec[2], save->sec[3], 
		save->sec[4], save->sec[5], save->sec[6], save->sec[7], &(save->out));
	   if (flush_mode) fflush_file(&(save->out));
	   if (i) fatal_error_i("submsg_uv: write record problem %d",i);
	   free_sec(save->sec);
	   fprintf(stderr,"submsg_uv: not paired, missing %s\n", save->vname);
	   save->vname = NULL;
	}

	/* at this point, U is empty and data in sec[] */

	/* check to see if new field is a U */

	save->vname = is_u(name);

	/* if U, cache it */
	if (save->vname != NULL) {
	    copy_sec(sec,save->sec);
	    return 0;
	}

	/* not U, write it out */
	i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
	if (flush_mode) fflush_file(&(save->out));
	if (i) fatal_error_i("submsg_uv: write problem %i",i);
	return 0;
    }
    return 0;
}
