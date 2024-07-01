#include <stdio.h>
#include <stdlib.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"


/*
 * HEADER:100:ncpu:setup:1:number of threads, default is environment variable OMP_NUM_THREADS/number of cpus
 */

#ifdef USE_OPENMP
#include <omp.h>
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
