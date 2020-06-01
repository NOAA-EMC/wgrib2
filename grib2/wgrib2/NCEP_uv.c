/*
 * NCEP_uv
 *
 * duplicate ncep files by puting U and V into one grib messsage
 *  
 * U and V must be adjacent
 *
 * if (cleanup) {
 *    if (saved_U) write saved_U
 *    return
 *  } 
 *  if (saved_U) {
 *     if (current_message != V) {
 *        write saved_U
 *        saved_U = NULL
 *        write current_message
 *     }
 *     else {
 *         merge saved_U, current message
 *         write merged message
 *         saved_U = NULL
 *     }
 *  }
 *  else { write current_message);
 *
 * 9/2010 Public Domain Wesley Ebisuzaki
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern int file_append;

/*
 * HEADER:111:ncep_uv:output:1:combine U and V fields into one message like NCEP operations
 */

int f_ncep_uv(ARG1) {

    struct local_struct {
	int has_U;
        unsigned char *sec[9];
	struct seq_file out;
    };

    struct local_struct *save;
    int i, is_u, is_v;
    unsigned int sec_size;
    size_t size;
    char name[NAMELEN];
    unsigned char s[16];

    if (mode == -1) {		// initialization

        // allocate static structure

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct) );
        if (save == NULL) fatal_error("NCEP_uv: memory allocation","");
        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
	save->has_U = 0;
        init_sec(save->sec);
	return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2)  {		// cleanup
	if (save->has_U) {
	   i = wrt_sec(save->sec[0], save->sec[1], save->sec[2], save->sec[3], 
		save->sec[4], save->sec[5], save->sec[6], save->sec[7], &(save->out));
	   if (i) fatal_error_i("NCEP_uv: last record problem %i",i);
           free_sec(save->sec);
	}
	fclose_file(&(save->out));
        free(save);
	return 0;
    }

    if (mode >= 0 )  {		// processing
        i = getName(sec, mode, NULL, name, NULL, NULL);
        is_u = strcmp(name,"UGRD") == 0;
	is_v = 1;

	// is_u and has_U are defined
	//   is_v is true if same metadata as saved U field and but VGRD
	if (save->has_U && is_v) {
            if (same_sec0(sec,save->sec) == 0) is_v = 0;
            if (same_sec1(sec,save->sec) == 0) is_v = 0;
            if (same_sec2(sec,save->sec) == 0) is_v = 0;
            if (same_sec3(sec,save->sec) == 0) is_v = 0;
	    i = GB2_ParmNum(sec);
	    if (i != 3) is_v = 0;
	    GB2_ParmNum(sec) = 2;
            // 1/2015 if (same_sec3(sec,save->sec) == 0) is_v = 0;
            if (same_sec4(sec,save->sec) == 0) is_v = 0;
	    GB2_ParmNum(sec) = i;
	}

	// merge U and V
	if (save->has_U && is_v) {
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

	    // section 0
	    for (i = 0; i < 8; i++) s[i] = sec[0][i];
            uint8_char(size, s+8);
            fwrite_file((void *) s, sizeof(char), 16, &(save->out));

	    for (i = 1; i <= 3; i++) {
		if (sec[i]) {
		   sec_size = uint4(sec[i]);
	           if (fwrite_file((void *)sec[i], sizeof(char), sec_size, &(save->out)) != sec_size) return 1;
		}
	    }
	    for (i = 4; i <= 7; i++) {
		if (save->sec[i]) {
		   sec_size = uint4(save->sec[i]);
	           if (fwrite_file((void *)save->sec[i], sizeof(char), sec_size, &(save->out)) != sec_size) return 1;
		}
	    }
	    for (i = 4; i <= 7; i++) {
		if (sec[i]) {
		   sec_size = uint4(sec[i]);
	           if (fwrite_file((void *)sec[i], sizeof(char), sec_size, &(save->out)) != sec_size) return 1;
		}
	    }

	    // section 8
            s[0] = s[1] = s[2] = s[3] = 55; /* s = "7777" */
            if (fwrite_file((void *) s, sizeof(char), 4, &(save->out)) != 4) return 1;

	    save->has_U = 0;
	    free_sec(save->sec);
	    return 0;
	}

	// U and not V
	if (save->has_U) {
	   i = wrt_sec(save->sec[0], save->sec[1], save->sec[2], save->sec[3], 
		save->sec[4], save->sec[5], save->sec[6], save->sec[7], &(save->out));
	   if (i) fatal_error_i("NCEP_uv: last field problem %i",i);
	   free_sec(save->sec);
	   save->has_U = 0;
	}

	// if U, save it
	if (is_u) {
	    copy_sec(sec,save->sec);
	    save->has_U = 1;
	    return 0;
	}

	i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
	if (i) fatal_error_i("NCEP_uv: write problem %i",i);
	return 0;
    }
    return 0;
}
