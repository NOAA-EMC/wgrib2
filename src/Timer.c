#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 *  Timer.c  3/2019   Public Domain Wesley Ebisuzaki
 */
 

#ifdef USE_OPENMP

#include <omp.h>


static double start_time, net_time;
static int n_time;

/*
 * HEADER:100:start_timer:misc:0:starts OpenMP timer
 */
 
int f_start_timer(ARG0) {
    if (mode >= 0) start_time = omp_get_wtime();
    return 0;
}

/*
 * HEADER:100:timer:inv:0:reads OpenMP timer
 */

int f_timer(ARG0) {
    double  delta_time, time;
    if (mode >= 0) {			// normal processing
	delta_time = (time = omp_get_wtime()) - start_time;
	n_time++;
	net_time +=  delta_time;
	start_time = time;
	sprintf(inv_out,"time=%lf", delta_time);
    }
    else if (mode == -1) {		// init
	delta_time = (time = omp_get_wtime()) - start_time;
        n_time = 0;
        net_time = 0.0;
        start_time = time;
//	sprintf(inv_out,"init-time=%lf", delta_time);
    }
    else if (mode == -2) {		// finalize
	if (n_time) {
	    delta_time = (time = omp_get_wtime()) - start_time;
	    sprintf(inv_out,"finalize-time=%lf:ave_time=%lf count=%d", delta_time, net_time/n_time,n_time);
	    n_time = 0;
	}
    }

    return 0;
}

#else

int f_start_timer(ARG0) {
    fprintf(stderr,"timer available, requires OpenMP\n");
    return 1;
}
int f_timer(ARG0) {
    fprintf(stderr,"timer available, requires OpenMP\n");
    return 1;
}

#endif

