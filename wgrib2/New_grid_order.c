/*
 * New_grid_order
 *
 * put grib file in order for new_grid to work
 *   
 * 10/2019    Public Domain Wesley Ebisuzaki
 *
 * output for new_grid is put into X
 * vectors that do not have corresponding vector are put into Y
 * vector fields are written into one grib message
 *
 * v1.0
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
 * uv_index
 *  returns index in u_v list
 *   even: U
 *   odd: V 
 *     V that corresponds to U has index+1
 *   -1: not found
 */

static int uv_index(const char *name) {
    int i;

    i = 0;
    while (vectors[i] != NULL) {
	if (strcmp(vectors[i], name) == 0) return i;
	if (strcmp(vectors[i+1], name) == 0) return i+1;
	i += 2;
    }
    return -1;
}


struct grib_message {
    size_t size_data;
    int uvindex;
    unsigned char *sec[9];
    unsigned char *data;
    struct grib_message *next;
};

/*
 * creates a new grib_message given sec[]
 */

static struct grib_message *new_grib_message(unsigned char **sec) {
    unsigned int size[9], j;
    int i;
    size_t size_all;
    unsigned char *p;

    struct grib_message *grb_msg;

    size[0] = GB2_Sec0_size;
    size[1] = GB2_Sec1_size(sec);
    size[2] = (sec[2]) ? GB2_Sec2_size(sec) : 0;
    size[3] = GB2_Sec3_size(sec);
    size[4] = GB2_Sec4_size(sec);
    size[5] = GB2_Sec5_size(sec);
    size[6] = GB2_Sec6_size(sec);
    size[7] = GB2_Sec7_size(sec);
    size[8] = GB2_Sec8_size;

    size_all = size[0] + size[1] + size[2] + size[3] + size[4] + size[5] + size[6] + size[7] + size[8];

    grb_msg = (struct grib_message *) malloc( sizeof(struct grib_message) );
    if (grb_msg == NULL) fatal_error("new_grid_order: memory allocation grb_msg","");

    grb_msg->data = (unsigned char *) malloc(size_all);
    if (grb_msg->data == NULL) fatal_error("new_grid_order: memory allocation grb_msg->data","");

    grb_msg->size_data = size_all;
    p = grb_msg->data;

    /* save grib message along with sec[] */
    for (i = 0; i < 9; i++) {
	if (sec[i] == NULL) {
	   grb_msg->sec[i] = NULL;
	}
	else {
	    grb_msg->sec[i] = p;
	    for (j = 0; j < size[i]; j++) {
		*p++ = sec[i][j];
	    }
	}
    }
    return grb_msg;
}

/* write U/V grib message */

static int wrt_uv_sec(unsigned char **sec1, unsigned char **sec2, struct seq_file *file) {
    size_t size;
    unsigned char s[16];
    int i, j;

    size = (size_t) GB2_Sec0_size + GB2_Sec8_size +
            ((sec1[1]) ? uint4(sec1[1]) : 0) +
            ((sec1[2]) ? uint4(sec1[2]) : 0) +
            ((sec1[3]) ? uint4(sec1[3]) : 0) +
            ((sec1[4]) ? uint4(sec1[4]) : 0) +
            ((sec2[4]) ? uint4(sec2[4]) : 0) +
            ((sec1[5]) ? uint4(sec1[5]) : 0) +
            ((sec2[5]) ? uint4(sec2[5]) : 0) +
            ((sec1[6]) ? uint4(sec1[6]) : 0) +
            ((sec2[6]) ? uint4(sec2[6]) : 0) +
            ((sec1[7]) ? uint4(sec1[7]) : 0) +
            ((sec2[7]) ? uint4(sec2[7]) : 0);

    /* write out sec[0] for combined grib message */
    for (i = 0; i < 8; i++) s[i] = sec1[0][i];
    uint8_char(size, s+8);
    if (fwrite_file((void *) s, sizeof(char), 16, file) != 16) {
        return 1;
    }

    /* write out sec[1] .. sec[7] for 1st message */
    for (j = 1; j <= 7; j++) {
        if (sec1[j]) {
            i = uint4(sec1[j]);
            if (fwrite_file((void *)sec1[j], sizeof(char), i, file) != i) {
                return 1;
            }
        }
    }
    /* write out sec[4] .. sec[7] for 2nd message */
    for (j = 4; j <= 7; j++) {
        if (sec2[j]) {
            i = uint4(sec2[j]);
            if (fwrite_file((void *)sec2[j], sizeof(char), i, file) != i) {
                return 1;
            }
        }
    }

    /* write out sec[8] .. end of grib message */
    s[0] = s[1] = s[2] = s[3] = 55; /* s = "7777" */
    if (fwrite_file((void *) s, sizeof(char), 4, file) != 4) {
       return 1;
    }
    if (flush_mode) fflush_file(file);
    return 0;
}

/*
 * HEADER:111:new_grid_order:output:2:put in required order for -new_grid, X=out Y=out2 no matching vector
 */

int f_new_grid_order(ARG2) {

    struct local_struct {
	struct grib_message *next; 	
	struct seq_file out, out_no_match;
    };

    struct local_struct *save;
    struct grib_message *last_grib, *grib, *p;

    int i, uv;
    char name[NAMELEN];

    if (mode == -1) {		// initialization

        // allocate static structure

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct) );
        if (save == NULL) fatal_error("new_grid_order: memory allocation","");
        if (fopen_file(&(save->out), arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        if (fopen_file(&(save->out_no_match), arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
	}
	save->next = NULL;
	return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2)  {		// cleanup
	/* delete grib messages that haven't been written out */
	if (save->next) {
	    grib = save->next;
	    while (grib) {
	        i = wrt_sec(grib->sec[0], grib->sec[1], grib->sec[2], grib->sec[3], grib->sec[4], 
			grib->sec[5], grib->sec[6], grib->sec[7], &(save->out_no_match));
	        if (flush_mode) fflush_file(&(save->out));
		p = grib;
		grib = grib->next;
		free(p->data);
		free(p);
	    }
	    fprintf(stderr,"new_grid_order: some vectors were unmatched, see %s\n", arg2);	
	}
	
	fclose_file(&(save->out));
	fclose_file(&(save->out_no_match));
        free(save);
	return 0;
    }

    if (mode >= 0 )  {					// processing

        i = getName(sec, mode, NULL, name, NULL, NULL);
	uv = uv_index(name);

	/* scalar */
	if (uv == -1) {
	    i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6], sec[7], &(save->out));
	    if (flush_mode) fflush_file(&(save->out));
	    if (i) fatal_error_i("new_grid_order: write problem %i",i);
	    return 0;
	}

	/* vector */

	/* see if u-v  match */

	grib = save->next;
	last_grib = NULL;
	i = uv ^ 1;

	while (grib != NULL) {
	    if (i == grib->uvindex) {
		if ( (same_sec0_not_var(mode, sec,grib->sec) == 1) &&
 	             (same_sec1_not_var(mode, sec,grib->sec) == 1) &&
 	             (same_sec2(sec,grib->sec) == 1) &&
 	             (same_sec3(sec,grib->sec) == 1) &&
 	             (same_sec4_not_var(mode,sec,grib->sec) == 1) ) break;
	    }
	    last_grib = grib;
	    grib = grib->next;
	}

	if (grib) {		// match

	    // write vector pair
	    if (uv & 1) wrt_uv_sec(grib->sec, sec, &(save->out));
	    else wrt_uv_sec(sec, grib->sec, &(save->out));
	    if (flush_mode) fflush_file(&(save->out));

	    // unlink grib
	    if (last_grib == NULL) save->next = grib->next;
	    else last_grib->next = grib->next;

	    // free grib
	    free(grib->data);
	    free(grib);
	    return 0;
	}

	/* save grib_message */
	p = new_grib_message(sec);
	if (p == NULL) fatal_error("new_grid_order: onew_grib_message","");
	p->uvindex = uv;

	/* add to linked list */
	p->next = save->next;
	save->next = p;
    }
    return 0;
}
