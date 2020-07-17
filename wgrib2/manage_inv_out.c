#include <stdio.h>
#include "wgrib2.h"

char *inv_out;
char *last_inv_out;
static char inv_buf1[INV_BUFFER];  
static char inv_buf2[INV_BUFFER];  

/* initialize once */

void init_inv_out(void) {
    inv_buf1[0] = inv_buf2[0] = '\0';
    inv_out = &(inv_buf1[0]);
    last_inv_out = &(inv_buf2[0]);
}

void new_inv_out(void) {
    if (inv_out == &(inv_buf1[0])) {
        inv_out = &(inv_buf2[0]);
        last_inv_out = &(inv_buf1[0]);
        inv_buf2[0] = '\0';
    }
    else if (inv_out == &(inv_buf2[0])) {
        inv_out = &(inv_buf1[0]);
        last_inv_out = &(inv_buf2[0]);
        inv_buf1[0] = '\0';
    }
    else
	fatal_error("new_inv_bufr: programming error","");
    return;
}
