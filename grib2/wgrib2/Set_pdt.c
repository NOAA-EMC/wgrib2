#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * the set options
 *
 * routines make a generic PDT
 *
 * 9/2008 Public Domain by Wesley Ebisuzaki
 * 10/2013 use update_sec4()
 * 7/2014 added copy more metadata when making new pdt
 *        if copying metadata, check for number of time ranges, make larger if needed
 * 5/2017 update for better handling of statistical processing
 * 5/2017 update for
 */

/*
 * HEADER:100:set_pdt:misc:1:makes new pdt, X=(+)PDT_number or X=(+)PDT_number:size of PDT in octets, +=copy metadata
 */


int f_set_pdt(ARG1) {

    unsigned char new_sec4[SET_PDT_SIZE];
    int i, pdt,len,copy_metadata;

    if (mode < 0) return 0;

    i = sscanf(arg1,"%d:%d", &pdt, &len);
    if (i == 0) fatal_error("set_pdt: X=PDT_number[:PDT_SIZE]","");
    if (i == 1) len = -1;
    copy_metadata = arg1[0] == '+';

    i = new_pdt(sec, new_sec4, pdt,len,copy_metadata);
    if (i == 0) update_sec4(sec, new_sec4);
    return i;
}

/*
 * new_pdt
 * sec4[] -> new_sec4[]
 * pdt = new pdt
 * len = -1 or len of sec4
 * copy_metadata = 1 (yes) 0 (no)
 */


int new_pdt(unsigned char **sec, unsigned char *new_sec4, int pdt, int len, int copy_metadata) {

    int i, k;
    int n_time_range_old, n_time_range_new;
    unsigned char *new_sec[9];
    unsigned char *p_old, *p_new;

//fprintf(stderr,"::: new_pdt: pdt=%d\n", pdt);

    for (i = 0; i < 9; i++) new_sec[i] = sec[i];
    new_sec[4] = new_sec4;

    k = stat_proc_n_time_ranges_index(sec);
//fprintf(stderr,"::: new_pdt: k=%d\n",k);
    n_time_range_old = k >= 0 ? sec[4][k] : 0;
    if (len <= 0) {
        len = smallest_pdt_len(pdt);
        if (copy_metadata && n_time_range_old > 0) {	// -set_pdt +X, size may vary
	    len += (n_time_range_old - 1) * 12;
        }
    }

//fprintf(stderr,"set_pdt: n_time_range_old %d\n", n_time_range_old);
// fprintf(stderr,"set_pdt: len %d\n",len);

    if (len > SET_PDT_SIZE) fatal_error_ii("set_pdt: maximum pdt len is %d wanted %d", SET_PDT_SIZE, len);
    for (i = 9; i < len; i++) new_sec4[i] = 255;

    uint_char(len, new_sec4);		    // size of sec[4];
    new_sec4[4] = 4;                        // section number
    uint2_char(0, new_sec4+5);              // no extra coordinates
    uint2_char(pdt, new_sec4+7);            // pdt

    if (copy_metadata) {

        new_sec[4] = new_sec4;

	/* copy statistical processing terms */
	/* only copy all or none */

        if ((k = stat_proc_n_time_ranges_index(sec)) >= 0) {		// input has time_ranges
            k = stat_proc_n_time_ranges_index(new_sec);
	    n_time_range_new = k >= 0 ?  (len - smallest_pdt_len(pdt)) / 12 + 1 : 0;
//fprintf(stderr,"n_time_range_old = %d\n", n_time_range_old);
//fprintf(stderr,"n_time_range_new = %d\n", n_time_range_new);
	    if (n_time_range_old <= n_time_range_new) {
		p_old = stat_proc_verf_time_location(sec);
		p_new = stat_proc_verf_time_location(new_sec);
		k = (58-34) + 12*(n_time_range_old-1);
//fprintf(stderr,"set_pdt copied %d bytes for stat processing\n", k);
	        for (i = 0; i < k; i++) p_new[i] = p_old[i];
	    }
	}
//	else {
//fprintf(stderr,"n_time_range_old = 0\n");
//	}

	new_sec4[9] = sec[4][9];		// parmmeter category 4.1
	new_sec4[10] = sec[4][10];		// parameter number   4.2

	p_old = code_table_4_3_location(sec);
	p_new = code_table_4_3_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = code_table_4_4_location(sec);
	p_new = code_table_4_4_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    for (i = 0; i < 5; i++) p_new[i] = p_old[i];
	}

	p_old = code_table_4_5a_location(sec);
	p_new = code_table_4_5a_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    for (i = 0; i < 6; i++) p_new[i] = p_old[i];
	}

	p_old = code_table_4_5b_location(sec);
	p_new = code_table_4_5b_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    for (i = 0; i < 6; i++) p_new[i] = p_old[i];
	}

	p_old = code_table_4_6_location(sec);
	p_new = code_table_4_6_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = code_table_4_7_location(sec);
	p_new = code_table_4_7_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = code_table_4_8_location(sec);
	p_new = code_table_4_8_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = perturbation_number_location(sec);
	p_new = perturbation_number_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = number_of_forecasts_in_the_ensemble_location(sec);
	p_new = number_of_forecasts_in_the_ensemble_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	/* probabilty info */
	p_old = code_table_4_9_location(sec);
	p_new = code_table_4_9_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    for (i = -2; i < 11; i++) p_new[i] = p_old[i];
	}

	p_old = background_generating_process_identifier_location(sec);
	p_new = background_generating_process_identifier_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = analysis_or_forecast_generating_process_identifier_location(sec);
	p_new = analysis_or_forecast_generating_process_identifier_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = hours_of_observational_data_cutoff_after_reference_time_location(sec);
	p_new = hours_of_observational_data_cutoff_after_reference_time_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    *p_new++ = *p_old++;
	    *p_new = *p_old;
	}

	p_old = minutes_of_observational_data_cutoff_after_reference_time_location(sec);
	p_new = minutes_of_observational_data_cutoff_after_reference_time_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	p_old = observation_generating_process_identifier_location(sec);
	p_new = observation_generating_process_identifier_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

	/* year of model version */
	p_old = year_of_model_version_date_location(sec);
	p_new = year_of_model_version_date_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    for (i = 0; i < 7; i++) p_new[i] = p_old[i];
	}

	/* percentile values */
	p_old = percentile_value_location(sec);
	p_new = percentile_value_location(new_sec);
	if (p_old != NULL && p_new != NULL) *p_new = *p_old;

        /* chemical type */
	p_old = code_table_4_230_location(sec);
	p_new = code_table_4_230_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    *p_new++ = *p_old++;
	    *p_new = *p_old;
	}

        /* aerosol type */
	p_old = code_table_4_233_location(sec);
	p_new = code_table_4_233_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    *p_new++ = *p_old++;
	    *p_new = *p_old;
	}

	/* 1st aerosol size */
	p_old = code_table_4_91_location(sec);
	p_new = code_table_4_91_location(new_sec);
	if (p_old != NULL && p_new != NULL) {
	    for (i = 0; i < 11; i++) p_new[i] = p_old[i];
	}

    }
    return 0;
}

