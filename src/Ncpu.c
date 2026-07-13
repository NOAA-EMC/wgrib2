/** @file
 * @brief Set the maximum number of threads, overriding OMP_NUM_THREADS.
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */
#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:ncpu:setup:1:number of threads, default is environment variable OMP_NUM_THREADS/number of cpus
 */

#ifdef USE_OPENMP
#include <omp.h>
/**
 * Set the maximum number of threads, overriding OMP_NUM_THREADS.
 * 
 * Wgrib2 can be compiled to use multiple threads. By default, an OpenMP-enabled version of 
 * wgrib2 will use the environement variable OMP_NUM_THREADS to control the number of threads. 
 * If the environment variable OMP_NUM_THREADS is not defined, the number of threads created 
 * is usually the same as the number of cores. The -ncpu option controls the number of threads 
 * that will be used by wgrib2. If wgrib2 is not compiled to use OpenMP, this option is quietly 
 * ignored. The -ncpu option overrides the OMP_NUM_THREADS environment variable. 
 * 
 * When you run an OpenMP-enabled wgrib2 version on a shared system, the performance may be 
 * suboptimal if you use too many threads. For example, on one of our linux systems, we have 
 * 32 cores. The system is shared by many users and during a busy time, there may only be a few 
 * cores free at any one time. When using 32 threads, there is often a thread that doesn't get 
 * much time in the beginning and is slow to finish. 
 * 
 * The main usage of the -ncpu option is to allocate threads/cpus when you are running multiple 
 * copies of wgrib2. 
 * 
 * ## Usage
 * -ncpu N
 * 
 * N = number of threads
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_ncpu(ARG1) {
    int nthreads;
    if (mode == -1) {
        nthreads = atoi(arg1);
        if (nthreads <= 0) fatal_error_i("NCPU (num threads) set to %d", nthreads);
        omp_set_num_threads(nthreads);
    }
    return 0;
}

#else
int f_ncpu(ARG1) {
    return 0;
}
#endif
