/******************************************************************************************

 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 Edition 2008.02.18

 Sergey Varlamov
 Kristian Nilssen
 Wesley Ebisuzaki
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

static const char *months = "janfebmaraprmayjunjulaugsepoctnovdec";

/*
 * 9/2006  w.ebisuzaki
 * 1/2007  check error code on verftime
 * 2/2008  wne for product definition template 8 (ave/acc/etc), return end of averaging interval -- like grads
 * 3/2008  wne add verf time for ensemble products, add vt=
 */

/*
 * HEADER:400:vt:inv:0:verf time = reference_time + forecast_time, -v2 for alt format
 */
int f_vt(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
	    if (mode != 2) {
	        sprintf(inv_out,"vt=%4.4d%2.2d%2.2d%2.2d", year,month,day,hour);
	    }
	    else {
               sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour,day,months[month*3-3],
		months[month*3-2], months[month*3-1], year);
	    }
        }
        else {
           sprintf(inv_out,"vt=?");
        }
    }
    return 0;
}

/*
 * HEADER:400:VT:inv:0:verf time = reference_time + forecast_time (YYYYMMDDHHMMSS)
 */
int f_VT(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
	    if (mode != 2) {
                sprintf(inv_out,"vt=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year,month,day,hour,minute,second);
	    }
	    else {
               sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour,minute,day,months[month*3-3],
		months[month*3-2], months[month*3-1], year);
	    }
        }
        else {
            sprintf(inv_out,"vt=?");
       }
    }
    return 0;
}


/*
 * Returns the verification time: reference_time + forecast_time + statistical processing time (if any)
 * 9/2006  w. ebisuzaki
 * 1/2007  w. ebisuzaki return error code
 * 11/2007  w. ebisuzaki fixed code for non forecasts
 * 3/2008 w. ebisuzaki added code for ensemble processing
 * 4/2009 w. ebisuzaki test table 1.2 sign of reference time
 */

int verftime(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second) {

    int units, i, j;
    int dtime;
    static int error_count = 0;
    static int warning_count = 0;

    i = code_table_4_0(sec);

    // if statistically processed field, verftime is in header
    j = stat_proc_verf_time(sec, year, month, day, hour, minute, second);
    if (j == 0) return 0;

    get_time(sec[1]+12, year, month, day, hour, minute, second);

    // some products have no forecast time

    if (code_table_4_4(sec) == -1) return 0;

    // check the significance of the refernce time
    i = code_table_1_2(sec);

    if (i == 2) {
	// rt=verifying time of forecast
	// unclear what it means for time averages/accumulations
	if (warning_count == 0) {
	    fprintf(stderr,"Warning: rt == vt (CodeTable 1.2)\n");
	    warning_count++;
	}
	return 0;
    }
    if (i == 3) {
	// rt = observing time 
	return 0;
    }

    if (i != 0 && i != 1) {
	if (error_count == 0) {
	    fprintf(stderr,"verifying time: Table 1.2=%d not supported "
		" using RT=analysis/start of forecast\n", i);
	    error_count++;
	}
    }

    units = code_table_4_4(sec);
    dtime = forecast_time_in_units(sec);
    if (dtime >= 0) 
        return add_time(year, month, day, hour, minute, second, (unsigned int) dtime, units);
    return sub_dt(year, month, day, hour, minute, second, (unsigned int) (unsigned int) -dtime, units);
}

/*
 * HEADER:400:start_ft:inv:0:verf time = reference_time + forecast_time (YYYYMMDDHH) : no stat. proc time
 */
int f_start_ft(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (start_ft(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"start_ft=%4.4d%2.2d%2.2d%2.2d", year,month,day,hour);
            }
            else {
               sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"start_ft=?");
       }
    }
    return 0;
}

/*
 * HEADER:400:start_FT:inv:0:verf time = reference_time + forecast_time (YYYYMMDDHHMMSS) - no stat. proc time
 */
int f_start_FT(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (start_ft(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"start_FT=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year,month,day,hour,minute,second);
            }
            else {
               sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour,minute,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"start_FT=?");
       }
    }
    return 0;
}

/*
 * HEADER:400:end_ft:inv:0:verf time = reference_time + forecast_time + stat. proc time (YYYYMMDDHH) (same as -vt)
 */

int f_end_ft(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"end_ft=%4.4d%2.2d%2.2d%2.2d", year,month,day,hour);
            }
            else {
               sprintf(inv_out,"%2.2dZ%2.2d%c%c%c%4.4d", hour,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"end_ft=?");
       }
    }
    return 0;
}

/*
 * HEADER:400:end_FT:inv:0:verf time = reference_time + forecast_time + stat. proc time (YYYYMMDDHHMMSS) (same as -VT)
 */
int f_end_FT(ARG0) {

    int year, month, day, hour, minute, second;

    if (mode >= 0) {
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            if (mode != 2) {
                sprintf(inv_out,"end_FT=%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", year,month,day,hour,minute,second);
            }
            else {
               sprintf(inv_out,"%2.2d_%2.2dZ%2.2d%c%c%c%4.4d", hour,minute,day,months[month*3-3],
                months[month*3-2], months[month*3-1], year);
            }
        }
        else {
            sprintf(inv_out,"end_FT=?");
       }
    }
    return 0;
}

int start_ft(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second) {

    int units, i;
    int dtime;
    static int error_count = 0;
    static int warning_count = 0;

    i = code_table_4_0(sec);

    // get reference time
    get_time(sec[1]+12, year, month, day, hour, minute, second);
    // some products have no forecast time .. done

    if ( (units = code_table_4_4(sec)) == -1) return 0;

    // check the significance of the refernce time
    i = code_table_1_2(sec);

    if (i == 2) {
        // rt=verifying time of forecast
        // unclear what it means for time averages/accumulations
        if (warning_count == 0) {
            fprintf(stderr,"Warning: rt == vt (CodeTable 1.2)\n");
            warning_count++;
        }
        return 0;
    }

    if (i != 0 && i != 1) {
        if (error_count == 0) {
            fprintf(stderr,"verifyingtime: Table 1.2=%d not supported "
                " using RT=analysis/start of forecast\n", i);
            error_count++;
        }
    }

    dtime = forecast_time_in_units(sec);
    add_time(year, month, day, hour, minute, second, dtime, units);
    return 0;
}

int reftime(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second)
{

    unsigned char *p;

    p = sec[1];
    *year = (p[12] << 8) | p[13];
    *month = p[14];
    *day = p[15];
    *hour = p[16];
    *minute = p[17];
    *second = p[18];

    return 0;
}

int Ref_time(unsigned char **sec, struct full_date *date) {

    return Get_time(sec[1]+12, date);

}

int Verf_time(unsigned char **sec, struct full_date *date) {
    return verftime(sec, &date->year, &date->month, &date->day, &date->hour, &date->minute, &date->second);
}

