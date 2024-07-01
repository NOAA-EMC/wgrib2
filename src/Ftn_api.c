#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "grid_id.h"

/* Ftn_api.c   5/2015 Public Domain Wesley Ebisuzaki
 *
 * this routine is called by grb2_inq - returns often needed information
 *
 */

extern enum output_order_type output_order;
extern unsigned int npnts, nx_, ny_;
extern int submsg, msg_no, inv_no;
/*
 * HEADER:100:ftn_api_fn0:inv:0:n npnts nx ny msg_no submsg i11,5(1x,i11)
 */
int f_ftn_api_fn0(ARG0) {
    /* ndata data may be undefined when data is not decoded */
    if (mode >= 0) sprintf(inv_out, "%11d %11u %11u %11u %11d %11d",inv_no,npnts,nx_,ny_,msg_no, submsg);
    return 0;   
}

