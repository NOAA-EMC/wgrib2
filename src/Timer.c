/** @file
 * @brief Functions for using OpenMP timers.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_OPENMP

#include <omp.h>

/** Start time for OpenMP timer */
static double start_time;

/** Net time for OpenMP timer */
static double net_time;

/** Count of OpenMP timer calls */
static int n_time;

/*
 * HEADER:100:start_timer:misc:0:starts OpenMP timer
 */

/**
 * Starts OpenMP timer.
 * 
 * ## Usage
 * -start_timer
 *
 * Sets timer_time = 0
 * 
 * Note: There is only one instance of timer_time. timer_time is set to 0 in the initialization 
 * of wgrib2.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 3/2019
 */
int f_start_timer(ARG0) {
    if (mode >= 0) start_time = omp_get_wtime();
    return 0;
}

/*
 * HEADER:100:timer:inv:0:reads OpenMP timer
 */

/**
 * Reads OpenMP timer.
 * 
 * The -start_timer and -timer options were designed for developers and people who want to 
 * know how fast wgrib2 is running. Most users can ignore these options. 
 * 
 * The -timer option prints out the time in seconds since the last -start_timer or -timer 
 * option was executed. The -timer option is executed in the initialization phase (no output) 
 * and the finalization phase (extra output). The timer is active in the finalization phase 
 * in order to time the finialization of options. 
 * 
 * This option uses the OpenMP funcion omp_get_wtime(), and require compilation with OpenMP.
 * 
 * ## Usage
 *  -timer              
 * 
 * Prints timer in seconds then sets timer_time = 0.
 * 
 * At end of processing, prints average of these results.
 * 
 * Note: There is only one instance of timer_time. timer_time is set to 0 in the initialization 
 * of wgrib2.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 3/2019
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

