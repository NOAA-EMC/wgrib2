#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"
/*
 * Fix_CFSv2_fcst.c
 *
 * sets flag controlling whether DX and DY are defined
 *
 * 11/2011: Public Domain: Wesley Ebisuzaki
 *
 */



/*
 * HEADER:100:fix_CFSv2_fcst:misc:3:fixes CFSv2 monthly fcst  X=daily or 00/06/12/18 Y=pert no. Z=number ens fcsts  v1.0
 */
int f_fix_CFSv2_fcst(ARG3) {

    int subcenter, n_time_ranges, fcst_time_0, fcst_time_1, i;
    int dtime, unit, id;

    int ref_year, ref_month, ref_day, ref_hour, ref_minute, ref_second;
    int fcst0_year, fcst0_month, fcst0_day, fcst0_hour, fcst0_minute, fcst0_second;
    int fcst1_year, fcst1_month, fcst1_day, fcst1_hour, fcst1_minute, fcst1_second;

    static unsigned char new_sec4[61];

    struct local_struct {
        int fixed;			// number of fields processed
	int dt;				// dt either 6 or 24
	int hour0;			// initial hour
        int pert;			// perturbation no, 0 = ctl
        int num_ensemble_members;
    };
    struct local_struct *save;

    if (mode == -1) {
	*local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
	save->fixed = 0;
	save->dt = 24;
	if (strcmp(arg1,"daily") == 0) {
	    save->dt = 6;
	    save->hour0 = 0;
	}
	else if (strcmp(arg1,"00") == 0) {
	    save->hour0 = 0;
	}
	else if (strcmp(arg1,"06") == 0) {
	    save->hour0 = 6;
	}
	else if (strcmp(arg1,"12") == 0) {
	    save->hour0 = 12;
	}
	else if (strcmp(arg1,"18") == 0) {
	    save->hour0 = 1;
	}
	else fatal_error("fix_CFSv2_fcst: bad arg %s, wanted daily,00,06,12 or 18",arg1);

	save->pert = atoi(arg2);
	save->num_ensemble_members = atoi(arg2);

    }
    save = *local;

    if (mode == -2) {
	sprintf(inv_out,"fix_CFSRv2_fcst %d fields fixed" , save->fixed);
	free(*local);
    }

    if (mode < 0) return 0;

    // only process NCEP CFSv2 monthly forecasts

    // must be NCEP generated
    if (GB2_Center(sec) != NCEP) return 0;

    // subcenter should be 0, 1 (reanalysis), 3 (EMC) or 4 (NCO)
    subcenter = GB2_Subcenter(sec);
    if (subcenter != 0 && subcenter != 1 && subcenter != 3 && subcenter != 4) return 0;

    // must have process id = 82 or 98
    id = analysis_or_forecast_generating_process_identifier(sec);
    if (id != 82 && id != 98) return 0;

    // product defn table must be 8 (fcst average)
    if ( code_table_4_0(sec) != 8) return 0;

    // n_time_ranges should be 1
    if ( (n_time_ranges = sec[4][41]) != 1) {
        fprintf(stderr,"unexected CFSv2 type forecast field .. pdt=8, n=%d\n", n_time_ranges);
	return 0;
    }

    // forecast time units should be months
    if (sec[4][17] != 3 || sec[4][48] != 3) return 0;

    fcst_time_0  = int4(sec[4]+18);
    fcst_time_1  = int4(sec[4]+49) + fcst_time_0;

    // get reference time
    reftime(sec, &ref_year, &ref_month, &ref_day, &ref_hour, &ref_minute, &ref_second);

    if (ref_minute != 0 || ref_second != 0) {
        fprintf(stderr,"unexected CFSv2 minute/second value != 0\n");
	return 0;
    }

    if (fcst_time_0 != 0) {	// start at day=1 hour=save->hour0 of proper month
	fcst0_year = ref_year;
	fcst0_month = ref_month + fcst_time_0;
        if (fcst0_month > 12) {
	    i = (fcst0_month - 1) / 12;
	    fcst0_year += i;
	    fcst0_month -= (i*12);
	}
	fcst0_day = 1;
	fcst0_hour = save->hour0;
	fcst0_minute = 0;
	fcst0_second= 0;
   }
   else {			// start at current month 
	fcst0_year = ref_year;
	fcst0_month = ref_month;
	fcst0_day = ref_day;
	fcst0_hour = ref_hour;
	fcst0_minute = ref_minute;
	fcst0_second= ref_second;
	if (save->dt == 24) {
	    i = save->hour0 - fcst0_hour;
	    if (i < 0) i += 24;
	    add_time(&fcst0_year, &fcst0_month, &fcst0_day, &fcst0_hour, &fcst0_minute, &fcst0_second, i, HOUR);
	}
    }

    sub_time(fcst0_year, fcst0_month, fcst0_day, fcst0_hour, fcst0_minute, fcst0_second, 
		ref_year, ref_month, ref_day, ref_hour, ref_minute, ref_second,
		&dtime, &unit);
    // set forecast time.
    if (dtime == 0) unit = MONTH;
    sec[4][17] = unit;
    int_char(dtime, sec[4]+18);

    if (fcst_time_1 == 0) fatal_error("fix-CFSv2_fcst: unexpected end_ft","");

    if (save->dt == 6) {		// ends at 18Z of last day of month
	fcst1_year = ref_year;
	fcst1_month = ref_month + fcst_time_1-1;
	if (fcst1_month > 12) {
            i = (fcst1_month - 1) / 12;
            fcst1_year += i;
            fcst1_month -= (i*12);
        }
        fcst1_day = num_days_in_month(fcst1_year, fcst1_month);
        fcst1_hour = 18;
        fcst1_minute = 0;
        fcst1_second= 0;

	// time increment = 6 hours
	sec[4][53] = (unsigned char) HOUR;
	int_char(6, sec[4]+54);
    }
    else {
	fcst1_year = ref_year;
	fcst1_month = ref_month + fcst_time_1-1;
	if (fcst1_month > 12) {
            i = (fcst1_month - 1) / 12;
            fcst1_year += i;
            fcst1_month -= (i*12);
        }
        fcst1_day = num_days_in_month(fcst1_year, fcst1_month);
        fcst1_hour = save->hour0;
        fcst1_minute = 0;
        fcst1_second= 0;

	// time increment = 24 hours
	sec[4][53] = (unsigned char)  HOUR;
	int_char(24, sec[4]+54);
    }

    // find stat processing time
    sub_time(fcst1_year, fcst1_month, fcst1_day, fcst1_hour, fcst1_minute, fcst1_second, 
		fcst0_year, fcst0_month, fcst0_day, fcst0_hour, fcst0_minute, fcst0_second, 
		&dtime, &unit);

    // change length of processing
    sec[4][48] = (unsigned char) unit;
    int_char(dtime, sec[4]+49);

// fprintf(stderr,"length of processing %d unit (%d)\n", dtime, unit);

    // save end-of-processing time
    save_time(fcst1_year, fcst1_month, fcst1_day, fcst1_hour, fcst1_minute, fcst1_second, sec[4]+34);

    // make sure that some basic info is correct
    sec[4][47] = 2;	// fcst_time++


    // now to add ensemble information
    for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];
    for (i = 34; i <= 57; i++) new_sec4[i+3] = sec[4][i];

    uint_char(61, new_sec4);			// length of new sec[4[
    new_sec4[7] = 0;				// pdt = 11
    new_sec4[8] = 11;

    // add perturbation info
    if (save->pert == 0) {			// pert == 0 means control forecast
        new_sec4[34] = 0;
        new_sec4[35] = 0;
    }
    else { 
        new_sec4[34] = 3;
        new_sec4[35] = save->pert;
    }
    new_sec4[35] = save->num_ensemble_members;
   
    sec[4] = new_sec4;
 
    save->fixed = save->fixed + 1;
    return 0;
}
