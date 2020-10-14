#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * set_ts_dates
 *
 * for changing date in a time series - used in converting to grib
 *
 * 3/2011 Public Domain by Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:set_ts_dates:misc:3:changes date code for time series X=YYYYMMDDHH(mmss) Y=dtime Z=#msgs/date
 */

int f_set_ts_dates(ARG3) {

    int year, month, day, hour, minute, second, i, j, units, n;
    int dtime;
    struct local_struct {
        int year, month, day, hour, minute, second;
        int unit;
	int dtime;
	int count, block_size;
    } *save;

    if (mode == -1) {
        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("set_ts_dates: memory allocation ","");

	save->month = save->day = 1;
	save->hour = save->minute = save->second = 0;

	i = sscanf(arg1,"%4d%2d%2d%2d%2d%2d" , &(save->year), &(save->month), 
		&(save->day), &(save->hour), &(save->minute), &(save->second));

	save->dtime = 0;
	save->unit = -1;
	save->count = -1;

        if (sscanf(arg2,"%i%n", &i, &n) != 1) fatal_error("set_ts_date: bad dt=%s",arg2);
	if (i < 0) fatal_error("set_ts_date: dt has to be positive %s",arg2);
	save->dtime =  i;
	arg2 += n;

	if (strcmp(arg2,"second") == 0) save->unit = 13;
	else if (strcmp(arg2,"minute") == 0) save->unit = 0;
	else if (strcmp(arg2,"hour") == 0) save->unit = 1;
	else if (strcmp(arg2,"day") == 0) save->unit = 2;
	else if (strcmp(arg2,"month") == 0) save->unit = 3;
	else if (strcmp(arg2,"year") == 0) save->unit = 4;
	else fatal_error("set_ts_date: could not understand time unit %s", arg2);

	save->block_size = atoi(arg3);
	if (save->block_size <= 0) fatal_error("set_ts_dates: #msgs/date code must be >= 1","");

	return 0;
    }

    if (mode == -2) { free(*local); return 0; }

    if (mode < 0) return 0;

    /* set date code */

    save = (struct local_struct *) *local;
    year = save->year;
    month = save->month;
    day = save->day;
    hour = save->hour;
    minute = save->minute;
    second = save->second;
    dtime = save->dtime;
    units = save->unit;
    save->count++;

    if (save->unit >= 0 && save->dtime != 0) {
       dtime *= (save->count / save->block_size);
       add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);
    }

    /* set reference time */
    save_time(year,month,day,hour,minute,second, sec[1]+12);

    j = stat_proc_n_time_ranges_index(sec);
    if (j == -1) return 0;	// return if not stat proc
    n = sec[4][j];              // number of stat proc elements
    j = j - 7;			// j is now the index to the start of end date code

    // add forecast time to time

    units = code_table_4_4(sec);
    dtime = forecast_time_in_units(sec);
    add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);

    for (i = 0; i < n; i++) {
        // add statistical processing time to time
        units = (int) sec[4][48-34+j+i*12];
        dtime = uint4(sec[4]+49-34+j+i*12);
        add_time(&year, &month, &day, &hour, &minute, &second, (unsigned int) dtime, units);
    }

    save_time(year,month,day,hour,minute,second, sec[4]+j);
    return 0;
}
