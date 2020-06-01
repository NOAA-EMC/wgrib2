#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"
/*
 * some routines that involve Sec4
 *
 * Public Domain 2006: Wesley Ebisuzaki
 * 1/2007  cleanup M Schwarb
 * 7/2009 bug fix (buffer overflow) Reinoud Bokhorst
 */

static void print_ftime (int unit1, int value1, int unit2, int value2, int format, char *inv_out);

/*
 * HEADER:440:ftime1:inv:0:forecast time
 */

	/*
	Code Table 4.4: Indicator of unit of time range
	Code figure   Meaning
	  0	Minute
	  1	Hour
	  2	Day
	  3	Month
	  4	Year
	  5	Decade (10 years)
	  6	Normal (30 years)
	  7	Century (100 years)
	  8-9	Reserved
	 10	3 hours
	 11	6 hours
	 12	12 hours
	 13	Second
	 14-191	Reserved
	192-254	Reserved for local use
	255	Missing
	*/


	int wrt_time(int unit, int value, char *inv_out) {

	   const char *string;

	   normalize_time_range(&unit, &value);

	   if (unit == 13) {	// second
		if (value == 0 || value % 60 != 0) {
		    sprintf(inv_out,"%d sec", value);
		    return 0;
		}
		value = value / 60;
		unit = 0;
	   }
	   if (unit == 0) {	// minute
		if (value == 0 || value % 60 != 0) {
		    sprintf(inv_out,"%d min", value);
		    return 0;
		}
		value = value / 60;
		unit = 1;
	   }

	   if (unit == 1) {     // hours
		if (value == 0 || value % 24 != 0) {
		    sprintf(inv_out,"%d hour", value);
		    return 0;
		}
		value = value / 24;
		unit = 2;
	   }

	   string = time_range2a(unit);
	   if (string != NULL) {
		sprintf(inv_out,"%d %s", value, string);
		return 0;
	   }
	   sprintf(inv_out,"????");
	   return 1;
	}

	/* 
	  print_ftime
	     format == 1   only print out 1st time
	     format == 2   print out 1st - (1st + 2nd) times
	 */

	static void print_ftime (int unit1, int value1, int unit2, int value2, int format, char *inv_out) {
//	void print_ftime (int unit1, int value1, int unit2, int value2, int format, char *inv_out) {

	    normalize_time_range(&unit1, &value1);
	    if (format != 1) normalize_time_range(&unit2, &value2);

	    if (format == 1) sprintf(inv_out,"%d %s",value1,time_range2a(unit1));
	    else if (format == 2) {
		if (unit1 == unit2) {
		    if (unit1 ==  1 && (value1 % 24 == 0) && (value2 % 24 == 0)) {		// hours
			value1 /= 24;
			value2 /= 24;
			unit1=2;
		    }
		    sprintf(inv_out,"%d-%d %s",value1,value1+value2,time_range2a(unit1));
		}
		else if (value1 == 0) {
		    sprintf(inv_out,"%d %s-%d %s",value1,time_range2a(unit1),value2,time_range2a(unit2));
		} 

		/* if HOURS is a common unit */
	/*
		else if ( (unit1 == 1 || unit1 == 10 || unit1 == 11 || unit1 == 12 || unit1 == 2) &&
			(unit2 == 1 || unit2 == 10 || unit2 == 11 || unit2 == 12 || unit2 == 2) ) {

		    if (unit1 == 12) value1 *= 12;
		    else if (unit1 == 11) value1 *= 6;
		    else if (unit1 == 10) value1 *= 3;
		    else if (unit1 == 2) value1 *= 4;

		    if (unit2 == 12) value2 *= 12;
		    else if (unit2 == 11) value2 *= 6;
		    else if (unit2 == 10) value2 *= 3;
		    else if (unit2 == 2) value2 *= 24;
		    value2 += value1;

		    if (value1 % 24 == 0) { value1 /= 24; unit1 = DAY; }
		    if (value2 % 24 == 0) { value2 /= 24; unit2 = DAY; }
		    sprintf(inv_out,"%d %s-%d %s",value1,time_range2a(unit1),value2,time_range2a(unit2));
		}
	*/
		else {
		    sprintf(inv_out,"%d %s-(%d %s+%d %s)",value1,time_range2a(unit1),
			value1,time_range2a(unit1),value2,time_range2a(unit2));
		}
	    }
	}

	int prt_stat_tr(int mode, unsigned char **sec, char *inv_out, unsigned char *p, int inner) {
	    const char *string;
	    int unit, value, unit2, value2, unit3, value3;

	    // unit2,value2:  time range for which stat processing is done
	    unit2 = p[2];
	    value2 = uint4(p+3);

	    // unit3,value3:  time range between sucessive fields (only for n > 1)
	    unit3 = p[7];
	    value3 = uint4(p+8);

	    if (mode == 99) {
		fprintf(stderr,"prt_stat_tr: code_4.10=p[0]=%d inner=%d\n", (int) p[0], inner);
		fprintf(stderr,"prt_stat_tr: code_4.11=p[1]=%d\n",(int) p[1]);
		fprintf(stderr,"prt_stat_tr: unit3=%d value3=%d\n", unit3, value3);
	    }

	    if (p[1] == 0) {
		sprintf(inv_out,"Code Table 4.11=reserved");
		inv_out += strlen(inv_out);
		return 0;
	    }
	    if (p[1] == 255) {
		sprintf(inv_out,"CodeTable 4.11=missing");
		inv_out += strlen(inv_out);
		return 0;
	    }
	    if (p[1] > 5) {
		sprintf(inv_out,"CodeTable 4.11=%d",p[1]);
		inv_out += strlen(inv_out);
		return 0;
	    }

	    if (inner) {
		if ((unit = code_table_4_4(sec)) < 0) return -1;
		// value = GB2_ForecastTime(sec);
		value = forecast_time_in_units(sec);

		if (mode == 99) fprintf(stderr,"prt_stat_tr: inner time_unit=%d fcst time=%d\n",unit,value);

		if (p[1] == 1) {

		    // initial time incremented
		    // forecast length is the same

		    if (p[0] == 51) {
			print_ftime(unit,value,unit2,value2,2, inv_out);
			inv_out += strlen(inv_out);
			sprintf(inv_out," climo");
			inv_out += strlen(inv_out);
			return 0;
		    }

		    if (mode == 99) fprintf(stderr,"loc:1234 p[1]=0 p[0]=%d, p[1]=%d\n", p[0],p[1]);

		    

		    /* average or accumulation or */
		    string = "";
		    switch(*p) {
	#include           "CodeTable_4.10.dat"
		    }
		    if (strcmp(string,"") == 0) string="???";


	//old          ave valid 12-16 hours
	//new       12-15 hour ave anl
		    if (unit3 == 255  || value3 == 0) { // ie n=1
	//	        sprintf(inv_out,"%s valid ",string);
	//       		inv_out += strlen(inv_out);
	//	        print_ftime(unit,value,unit2,value2,2, inv_out);
	//	        inv_out += strlen(inv_out);
			print_ftime(unit,value,unit2,value2,2, inv_out);
			inv_out += strlen(inv_out);
			sprintf(inv_out," %s anl",string);

// 7/2015 WNE		if (value == 0) sprintf(inv_out," %s anl",string);
//			else sprintf(inv_out," %s fcst,anl++",string);
			inv_out += strlen(inv_out);
			return 0;
		    }

	//	    5@1 day ave
		    if (unit2 == unit3 && value3 != 0 && value2 % value3 == 0) {
			sprintf(inv_out,"%d@",value2/value3+1);
			inv_out += strlen(inv_out);

	//	        sprintf(inv_out,"[%d@]",value2);
			inv_out += strlen(inv_out);

			wrt_time(unit3,value3,inv_out);
			inv_out += strlen(inv_out);
			sprintf(inv_out," %s",string);
			inv_out += strlen(inv_out);
		    }
	//	    (time) @ (dt) ave
		    else {
			wrt_time(unit2,value2,inv_out);
			inv_out += strlen(inv_out);
			sprintf(inv_out," %s",string);
			inv_out += strlen(inv_out);
			if (value3 != 0 && unit3 != 255) {
			    sprintf(inv_out," @");
			    inv_out += strlen(inv_out);

			    wrt_time(unit3,value3,inv_out);
			    inv_out += strlen(inv_out);
			}
		    }

		    // value = GB2_ForecastTime(sec);
		    value = forecast_time_in_units(sec);
		    if ((unit = code_table_4_4(sec)) < 0) return -1;

		    if (value == 0) {
			// sprintf(inv_out," anl");
			sprintf(inv_out,"(anl)");
			inv_out += strlen(inv_out);
		    }
		    else {
			// wne sprintf(inv_out," (");
			sprintf(inv_out,"(");
			inv_out += strlen(inv_out);
			wrt_time(unit,value,inv_out);
			inv_out += strlen(inv_out);
			sprintf(inv_out," fcst)");
			inv_out += strlen(inv_out);
		    }

		    return 0;
		}

		if (p[1] == 2) {

		    if (mode == 99) fprintf(stderr,"loc:12356 p[0]=%d, p[1]=%d\n", p[0],p[1]);
		    if (mode == 99) fprintf(stderr,"p[1]=2 unit=%d value=%d unit2=%d value2=%d\n",unit,value,unit2,value2);
		    print_ftime(unit,value,unit2,value2,2, inv_out);
		    inv_out += strlen(inv_out);
		    /* average or accumulation or */
		    string = "";
		    switch(*p) {
	#include           "CodeTable_4.10.dat"
		    }
		    if (strcmp(string,"") == 0) string="???";
		    if (mode == 99) fprintf(stderr,"codetable_4.10 is (%s)\n", string);

		    // NCEP TMIN and TMAX set codetable 4.10 to missing
		    // if TMIN/TMAX, do not print stat processing operator if missing

		    if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 0
			&& (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 4 || GB2_ParmNum(sec) == 5)
			&& strcmp(string,"missing") == 0) {
			string="";
		    }
		    else {
			sprintf(inv_out," %s",string);
		    }
		    inv_out += strlen(inv_out);

		    if (value3 != 0 && unit3 != 255) {  // n == 1
			sprintf(inv_out,"@(fcst,dt=");
			inv_out += strlen(inv_out);
			wrt_time(unit3,value3,inv_out);
			inv_out += strlen(inv_out);
			sprintf(inv_out,")");
		    }
		    else sprintf(inv_out," fcst");
		    inv_out += strlen(inv_out);
		    return 0;
		}

		if (p[1] == 3 || p[1] == 4) {
		    if (mode == 99) fprintf(stderr,"p[1]=3/4 value2=%d unit2=%d\n", value2,unit2);
		   /* average or accumulation or */
		    string = "";
		    switch(*p) {
	#include           "CodeTable_4.10.dat"
		    }
		    if (strcmp(string,"") == 0) string="???";


	//          ensemble ave valid 12 hours
		    sprintf(inv_out,"ensemble %s-%d valid ",string, p[1]);
		    inv_out += strlen(inv_out);
		    print_ftime(unit,value,unit2,value2,1, inv_out);
		    inv_out += strlen(inv_out);

		    if (mode > 0) {
			sprintf(inv_out," code4.10=%d unit2=%d value2=%d unit3=%d value3=%d",
			   p[1], unit2, value2, unit3, value3);
		    }
		    return 0;
		}
		wrt_time(unit,value,inv_out);
		inv_out += strlen(inv_out);
		if (p[1] == 3) {
		    sprintf(inv_out," t0+ ft- ");
		}
		else if (p[1] == 4) {
		    sprintf(inv_out," t0- ft+ ");
		}
		else if (p[1] == 5) {
		    sprintf(inv_out," dt ");
		}
		else {
		    sprintf(inv_out," (%d) ", p[1]);
		}
		inv_out += strlen(inv_out);

		wrt_time(unit2,value2,inv_out);
		inv_out += strlen(inv_out);
	    }

	//  outer loop

    /* average or accumulation or */
    string = "";
    switch(*p) {
#include           "CodeTable_4.10.dat"
    }
    if (strcmp(string,"") == 0) string="???";


//   10@3 hour ave

if (mode == 99) fprintf(stderr,"ftime: value2 %d value3 %d\n", value2, value3);
if (mode == 99) fprintf(stderr,"ftime: unit2 %d unit3 %d\n", unit2, unit3);
    if (unit2 == unit3 && value3 != 0 && value2 % value3 == 0 && p[0] != 51 ) {
        sprintf(inv_out,"%d@",value2/value3+1);
        inv_out += strlen(inv_out);
	wrt_time(unit3,value3,inv_out);
	inv_out += strlen(inv_out);
        sprintf(inv_out," %s",string);
        inv_out += strlen(inv_out);
	return 0;
    }

if (mode == 99) fprintf(stderr,"ftime: more code");

    wrt_time(unit2,value2,inv_out);
    inv_out += strlen(inv_out);

    sprintf(inv_out," %s",string);
    inv_out += strlen(inv_out);

    if (value3 != 0 && p[0] != 51) {	// not if climo or value == 0 
        sprintf(inv_out,"@");
        inv_out += strlen(inv_out);
	wrt_time(unit3,value3,inv_out);
	inv_out += strlen(inv_out);
    }

    return 0;
}

int f_ftime1(ARG0) {
    int unit, value, n,i,code_4_11, code_4_10;
    int loc_n;

    if (mode < 0) return 0;

    if (mode == 99) fprintf(stdout,"ProdDefTemplateNo=%d\n", GB2_ProdDefTemplateNo(sec));

    /* if not a forecast .. no code 4.4 */
    if (code_table_4_4_not_used(sec)) {
        sprintf(inv_out,"anl");
	return 0;
    }

    loc_n = stat_proc_n_time_ranges_index(sec);

    if (loc_n == -1) {
	if ((unit = code_table_4_4(sec)) < 0) return -1;
        // value = GB2_ForecastTime(sec);
	value = forecast_time_in_units(sec);
        if (value == 0) {
            sprintf(inv_out,"anl");
        }
        else {
    	    print_ftime(unit, value, 0, 0, 1, inv_out);
	    inv_out += strlen(inv_out);
            sprintf(inv_out," fcst");
	    inv_out += strlen(inv_out);
        }
	return 0;
    }
    else {
	if ((unit = code_table_4_4(sec)) < 0) return -1;
        n = (int) sec[4][loc_n];
	/* strictly n should be unsigned but should never get n > 127 */
 	// if (n <= 0 || loc_n+5+n*12 > GB2_Sec4_size(sec)) fatal_error_i("Statistical processing bad n=%d\n", n);
 	if (n <= 0) {
	    fprintf(stderr,"** WARNING bad grib message: Statistical processing bad n=%d **\n", n);
	    return 0;
	}
 	if (loc_n+5+n*12 > GB2_Sec4_size(sec)) {
            fprintf(stderr,"** WARNING bad grib message: sec4 len (Statistical Processing): found %u .. needs at least %d **\n",
 	        GB2_Sec4_size(sec), loc_n+5+n*12);
	    return 0;
	}

	if (mode == 99) fprintf(stdout,"f_ftime:  n=sec[4][loc_n]=%d\n", n);
	code_4_10 = sec[4][5+loc_n];
	code_4_11 = sec[4][6+loc_n];
	if (mode == 99) printf("f_ftime: stat_proc code_4.10=%d increment code_4.11=%d\n", code_4_10, code_4_11);
	if (mode == 99) printf("f_ftime: code_4.4=%d ntime=%d\n", sec[4][7+loc_n],uint4(sec[4]+8+loc_n));

	// code table 4.11
	//
	// 1:        (..)			ref_time++
	// 2:        (..)++			fcst_time++
	// 3:        LAF[..]--                  fcst_time--, ref_time++
	// 4:        LAF[..]++                  fcst_time++, ref_time--


	for (i = 1; i <= n; i++) {
	    if (i > 1) {
	        switch (sec[4][loc_n+i*12-18]) {
		    case 1: // ref time++
		    case 2: // ref time
		        sprintf(inv_out,"("); break;
		    case 3: // ref time--
		    case 4: // ref time++
		        sprintf(inv_out," LAF["); break;
		    default: 
		        sprintf(inv_out," ?["); break;
		}
	        inv_out += strlen(inv_out);
	    }
	    prt_stat_tr(mode, sec, inv_out, sec[4]+5+loc_n+i*12-12, n == i);
	    inv_out += strlen(inv_out);
	    if (i > 1) {
	    	// check code table 4.11
	        switch (sec[4][loc_n+i*12-18]) {
		    case 1: // ref time++
		    sprintf(inv_out,")"); break;
		    case 2: // fcst time++ ref=constant
			    sprintf(inv_out,")++"); break;
		    case 3: // fcst time--, ref_time++
			    sprintf(inv_out,"]--"); break;
		    case 4: // fcst time++, ref_time--
			    sprintf(inv_out,"]++"); break;
		    default: sprintf(inv_out,"]"); break;
	    	}
		inv_out += strlen(inv_out);
	    }
        }

	if (uint4(sec[4]+13+loc_n) != 0) {	// not a continous process -- print no. missing
	    sprintf(inv_out,",missing=%d",uint4(sec[4]+1+loc_n));
	    inv_out += strlen(inv_out);
	}
	return 0;
    }
    return 0;
}
