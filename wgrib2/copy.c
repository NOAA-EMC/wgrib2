/* simple utility routine to copy sec[], and free it later
 *
 * 8/2008 public domain Wesley Ebisuzaki
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "grb2.h"
#include "wgrib2.h"


int copy_sec(unsigned char **sec, unsigned char **clone_sec) {

    int i;
    unsigned int size[9];

//  get size of each secion

    size[0] = GB2_Sec0_size;
    size[1] = GB2_Sec1_size(sec);
    size[2] = 0;
    if (sec[2]) size[2] = GB2_Sec2_size(sec);
    size[3] = GB2_Sec3_size(sec);
    size[4] = GB2_Sec4_size(sec);
    size[5] = GB2_Sec5_size(sec);
    size[6] = GB2_Sec6_size(sec);
    size[7] = GB2_Sec7_size(sec);
    size[8] = GB2_Sec8_size;

//  allocate and copy memory

    for (i = 0; i < 9; i++) {
	if (size[i] > 0) {
	    if ((clone_sec[i] = (unsigned char *) malloc(size[i])) == NULL) 
	        fatal_error_i("memory allocation failed copy_sec %d",i);
	    memcpy(clone_sec[i], sec[i], size[i]);
	}
	else {
	    clone_sec[i] = NULL;
	}
    }	

    return 0;
}

int free_sec(unsigned char **clone_sec) {
    int i;
    for (i = 0; i < 9; i++) {
	if (clone_sec[i]) free(clone_sec[i]);
	clone_sec[i] = NULL;
    }
    return 0;
}

int init_sec(unsigned char **clone_sec) {
    int i;
    for (i = 0; i < 9; i++) {
	clone_sec[i] = NULL;
    }
    return 0;
}


int copy_data(float *data, unsigned int ndata, float **clone_data) {

    float *fp;
    unsigned int i;

    *clone_data = fp = (float *) malloc(((size_t) ndata) * sizeof(float));
    if (fp == NULL) fatal_error("memory allocation clone_data","");

    for (i = 0; i < ndata; i++) {
        fp[i] = data[i];
    }
    return 0;
}

int free_data(float *clone_data) {
    free(clone_data);
    return 0;
}
