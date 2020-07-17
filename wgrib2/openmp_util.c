#include <stdio.h>
#include <limits.h>
#include "wgrib2.h"

#ifdef USE_OPENMP
#include <omp.h>
#else
#define omp_get_num_threads()           1
#define omp_get_thread_num()		0
#endif


/* openmp compatible routines, does not require OpenMP 3.1
 *
 * Public Domain 8/2015 Wesley Ebisuzaki  
 *               6/2016 Wesley Ebisuzaki
 */

/*
 * min_max_array()  returns min/max of the array
 *
 * return 0:  if min max found
 * return 1:  if min max not found, min = max = 0
 */

int min_max_array(float *data, unsigned int n, float *min, float *max) {

    unsigned int first, i;
    float mn, mx, min_val, max_val;

    if (n == 0) {
	*min = *max = 0.0;
	return 1;
    }

    for (first = 0; first < n; first++) {
        if (DEFINED_VAL(data[first])) break;
    }
    if (first >= n) {
	*min = *max = 0.0;
	return 1;
    }

    mn = mx = data[first];

#pragma omp parallel private(min_val, max_val)
    {
	min_val = max_val = data[first];

#pragma omp for private(i) schedule(static) nowait
	for (i = first+1; i < n; i++) {
	    if (DEFINED_VAL(data[i])) {
                min_val = (min_val > data[i]) ? data[i] : min_val;
                max_val = (max_val < data[i]) ? data[i] : max_val;
            }
	}

#pragma omp critical
	{
	    if (min_val < mn) mn = min_val;
	    if (max_val > mx) mx = max_val;
	}
    }

    *min = mn;
    *max = mx;
    return 0;
}

/*
 * min_max_array_all_defined()  returns min/max of the array which has all values defined
 *
 * return 0:  if min max found
 * return 1:  if min max not found, min = max = 0
 */

int min_max_array_all_defined(float *data, unsigned int n, float *min, float *max) {

    float mx, mn, min_val, max_val;
    unsigned int i;

    if (n == 0) {
	*min = *max = 0.0;
	return 1;
    }

    min_val = max_val = data[0];
#pragma omp parallel private(mn, mx, i)
    {
        mn = mx = data[0];
#pragma omp for nowait
        for (i = 1; i < n; i++) {
            mn = (mn > data[i]) ? data[i] : mn;
            mx = (mx < data[i]) ? data[i] : mx;
        }
#pragma omp critical
        {
            if (min_val > mn) min_val = mn;
            if (max_val < mx) max_val = mx;
        }
    }
    *min = min_val;
    *max = max_val;
    return 0;
} 

/*
 * find min/max of an integer array
 * return 0:  if min max found
 * return 1:  if min max not found, min = max = 0
 */
int int_min_max_array(int *data, unsigned int n, int *min, int *max) {

    unsigned int first, i;
    int  mn, mx, min_val, max_val;

    if (n == 0) {
        return 1;
    }

    for (first = 0; first < n; first++) {
        if (data[first] != INT_MAX) {
            mx = mn = data[first];
            break;
        }
    }
    if (first >= n) return 1;

    mn = mx = data[first];

#pragma omp parallel private(min_val, max_val)
    {
        min_val = max_val = data[first];

#pragma omp for private(i) schedule(static) nowait
        for (i = first+1; i < n; i++) {
            if (data[i] != INT_MAX) {
                min_val = (min_val > data[i]) ? data[i] : min_val;
                max_val = (max_val < data[i]) ? data[i] : max_val;
            }
        }

#pragma omp critical
        {
            if (min_val < mn) mn = min_val;
            if (max_val > mx) mx = max_val;
        }
    }

    *min = mn;
    *max = mx;
    return 0;
}

/*
 * compute delta complex packing (c2)
 *  to make it harder, do not allocate memory
 */
int delta(int *data, unsigned int n, int *min, int *max, int *first_val) {
    int nthreads, thread_id, tmp, last_val, mn, mx, mn_thread, mx_thread, mx_set, mx_set_thread;
    unsigned int first, j, last, istart, iend;

    for (first = 0; first < n; first++) {
        if (data[first] != INT_MAX) {
            break;
        }
    }
    if (first >= n) return 1;
    data += first;
    n -= first;
    *first_val = data[0];

    mx_set = mn = mx = 0;

    // to reproduce 1cpu version, same grid values, inefficient for monotonic fields
    mx_set = 1;

#pragma omp parallel private(nthreads, thread_id, istart, iend, last_val, last, j, tmp, mn_thread, mx_thread, mx_set_thread)
    {
	nthreads = omp_get_num_threads();
	thread_id = omp_get_thread_num();
	istart = (thread_id*n)/nthreads;
	iend = ((thread_id+1)*n)/nthreads;
	if (thread_id == nthreads - 1) iend = n;
        mx_set_thread = mn_thread = mx_thread = 0;

	/* find first defined for sub-section */
	for (last = istart; last < iend; last++) {
            if (data[last] != INT_MAX) break;
	}
        if (last != iend) last_val = data[last];
	// need to wait until all threads have read data[last] from memory
#pragma omp barrier

        // threads can now compute the deltas and write to data[]
	// threads can write to data[last] of other threads

        if (last != iend) {
	    j = last + 1;
	    while (last < iend && j < n) {
		if (data[j] != INT_MAX) {
		    tmp = data[j];
		    data[j] -= last_val;
		    if (mx_set_thread) {
		        mn_thread = mn_thread <= data[j] ? mn_thread : data[j];
		        mx_thread = mx_thread >= data[j] ? mx_thread : data[j];
		    }
		    else {
			mx_set_thread = 1;
			mn_thread = mx_thread = data[j];
		    }
		    last_val = tmp;
		    last = j;
		}
		j++;
	    }
	}
	if (mx_set_thread) {
#pragma omp critical
	    {
		if (mx_set) {
	            mn = mn <= mn_thread ? mn : mn_thread;
	            mx = mx >= mx_thread ? mx : mx_thread;
		}
		else {
		    mx_set = 1;
		    mn = mn_thread;
		    mx = mx_thread;
		}
	    }
	}

    }
    data[0] = 0;
    if (mx_set) {
        *min = mn;
        *max = mx;
    }
    else *min = *max = 0;

    return 0;
}

/*
 * compute delta of the deltas (complex packing c3)
 *  to make it harder, use openmp and do not allocate memory
 */

int delta_delta(int *data, unsigned int n, int *min, int *max, int *first_val, int *second_val) {

    int nthreads, thread_id, tmp, mn, mx, mn_thread, mx_thread, mx_set, mx_set_thread, penultimate_val, last_val;
    unsigned int first, second, j, penultimate, last, istart, iend;

    for (first = 0; first < n; first++) {
        if (data[first] != INT_MAX) {
            break;
        }
    }
    if (first >= n) return 0;
    *first_val = data[first];

    for (second = first+1; second < n; second++) {
        if (data[second] != INT_MAX) {
            break;
        }
    }
    if (second >= n) {
	data[first] = 0;
	return 0;
    }
    *second_val = data[second];
    mx_set = mn = mx = 0;

    n -= first;
    data += first;

#pragma omp parallel private(nthreads, thread_id, istart, iend, penultimate_val, penultimate, last_val, last, j, tmp, mn_thread, mx_thread, mx_set_thread)
    {
        nthreads = omp_get_num_threads();
        thread_id = omp_get_thread_num();
        istart = (thread_id*n)/nthreads;
        iend = ((thread_id+1)*n)/nthreads;
        if (thread_id == nthreads - 1) iend = n;
        mx_set_thread = mx_thread = mn_thread = 0;

	/* find penultimate for sub-section */
	for (penultimate = istart; penultimate < iend; penultimate++) {
            if (data[penultimate] != INT_MAX) break;
	}
        if (penultimate != iend) {
	    penultimate_val = data[penultimate];

	    // find last for sub-section */
	    for (last = penultimate + 1; last < iend; last++) {
                if (data[last] != INT_MAX) break;
	    }
            if (last != iend) last_val = data[last];
        }
        else {
	    last = iend;
	}

	// need to wait until all threads have read data[last] and data[penultimate]
#pragma omp barrier

        // threads can now compute the deltas and write to data[]

        if (last != iend) {
	    j = last + 1;
	    while (penultimate < iend && j < n) {
		if (data[j] != INT_MAX) {
		    tmp = data[j];
		    data[j] = data[j] - last_val - last_val + penultimate_val;
		    if (mx_set_thread) {
		        mn_thread = mn_thread <= data[j] ? mn_thread : data[j];
		        mx_thread = mx_thread >= data[j] ? mx_thread : data[j];
		    }
		    else {
			mx_set_thread = 1;
			mn_thread = mx_thread = data[j];
		    }
		    penultimate = last;
		    last = j;
		    penultimate_val = last_val;
		    last_val = tmp;
		}
		j++;
	    }
	}
	if (mx_set_thread) {
#pragma omp critical
	    {
		if (mx_set) {
	            mn = mn <= mn_thread ? mn : mn_thread;
	            mx = mx >= mx_thread ? mx : mx_thread;
		}
		else {
		    mx_set = 1;
		    mn = mn_thread;
		    mx = mx_thread;
		}
	    }
	}

    }
    data[0] = 0;
    data[second-first] = 0;
    if (mx_set) {
        *min = mn;
        *max = mx;
    }
    else *min = *max = 0;
    return 0;
}

