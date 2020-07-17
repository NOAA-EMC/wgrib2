#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * Fix_ncep.c
 *
 * 7/2008: Public Domain: Wesley Ebisuzaki
 *
 * need to fix up code -- hours may be assumed rather than given
 */

extern int decode, latlon;
extern double *lat, *lon;
extern int nx, ny;

/*
 * HEADER:100:fix_ncep:misc:0:fix ncep PDT=8 headers produced by cnvgrib
 */

int f_fix_ncep(ARG0) {
    unsigned char *p;
    int n, type;
    unsigned int u, NAve;
    int year, month, day, hour, minute, second;
    unsigned int units, p1_units;
    unsigned int dtime;

    int stat_proc1, stat_proc2, p1, p2, p2_m_p1, factor;

    if (mode < 0) return 0;

    if (GB2_Center(sec) != NCEP) return 0;		// only NCEP
    if (GB2_ProdDefTemplateNo(sec) != 8) return 0;	// pdt == 4.8 only
    if (code_table_4_4(sec) < 0) return -1;		// time unit must be defined

    p = sec[4];
    n = (int) p[41];					// number of loops
  
    if (n != 2) return 0;				// always have n == 2

    stat_proc1 = p[46];
    stat_proc2 = (n == 2) ? p[58] : -1;
    if (mode == 99) fprintf(stdout,"\nfix_ncep: stat_proc=(%d,%d) n=%d\n", stat_proc1, stat_proc2, n);


    if (n == 2) {
	p2 = p[57];
	p2_m_p1 = p[64];
	p1 = p2 - p2_m_p1;
        if (mode == 99) fprintf(stdout,"p2=%d p2-p1=%d p1=%d \n", p2, p2_m_p1, p1);
        p1_units = p[17];
     }

    type = p[46];
    NAve = uint4(p+49);

    if (mode == 99) {
	fprintf(stdout,"nloops=%d\n", n);
        wrt_time(p1_units, uint4(p+18),inv_out);
	fprintf(stdout,"P1=%s  ", inv_out);
	fprintf(stdout,"Nave=%u  type=%d\n", uint4(p+42), type);
        wrt_time(p[48], uint4(p+49),inv_out);
	fprintf(stdout,"time1=%s", inv_out);

        wrt_time(p[53], uint4(p+54),inv_out);
	fprintf(stdout," time2=%s\n",inv_out);

	if (n >= 2) {
        wrt_time(p[48+12], uint4(p+49+12),inv_out);
	fprintf(stdout,"time1=%s", inv_out);
        wrt_time(p[53+12], uint4(p+54+12),inv_out);
        }
	fprintf(stdout," time2=%s\n",inv_out);
	fprintf(stdout," time: %d-%d-%d-%d\n", int2(p+34), p[36], p[37], p[38]);
	fprintf(stdout," Nave=%u\n",NAve);
    }

//    if (stat_proc1 == 0 && stat_proc2 == 204) stat_proc1 = 204;
//    if (stat_proc1 == 0 && stat_proc2 == 205) stat_proc1 = 205;
//    if (stat_proc1 == 0 && stat_proc2 == 206) stat_proc1 = 206;
//    if (stat_proc1 == 0 && stat_proc2 == 207) stat_proc1 = 207;

    if (stat_proc1 == 255 && stat_proc2 == 204) stat_proc1 = 204;
    if (stat_proc1 == 255 && stat_proc2 == 205) stat_proc1 = 205;
    if (stat_proc1 == 255 && stat_proc2 == 206) stat_proc1 = 206;
    if (stat_proc1 == 255 && stat_proc2 == 207) stat_proc1 = 207;

    if ( (stat_proc1 >= 204 &&  stat_proc1 <= 207) ) {

	factor = 6;
        if (stat_proc1 == 204) {
            p[46] = 0;              // average
            p[58] = 1;              // accumulation
	    // factor = 6;
        }
        else if (stat_proc1 == 205) {
            p[46] = p[58] = 0;              // average
	    // factor = 6;
	}
        if (stat_proc1 == 206) {
            p[46] = 0;              // average
            p[58] = 1;              // accumulation
	    factor = 12;
        }
        else if (stat_proc1 == 207) {
            p[46] = p[58] = 0;              // average
	    factor = 12;
	}

	p[47] = 1;
	p[59] = 2;

	// get reference time
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

	// add in (NAve-1) * factor hours
        add_time(&year, &month, &day, &hour, &minute, &second, (NAve-1)*factor, 1);

	// add in P2 hours
        add_time(&year, &month, &day, &hour, &minute, &second, p2, 1);

	// save end of end of period time
        save_time(year,month,day,hour,minute,second, sec[4]+34);

	p[48] = 1;
	uint_char((NAve-1)*factor,p+49);

	p[53] = 1;
	uint_char(factor,p+54);

	p[48+12] = 1;
	uint_char(p2-p1,p+49+12);

	p[53+12] = 1;
	uint_char(0,p+54+12);
	return 0;
    }

    if (stat_proc1 == 193 || stat_proc1 == 194) {

        if (stat_proc1 == 194) p1 = 0;

	p[17] = p1_units;		// forecast time units
	uint_char(p1,p+18);		// forecast time

        // get reference time
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

        // add in (NAve-1) * p2 hours
        add_time(&year, &month, &day, &hour, &minute, &second, (NAve-1)*p2, p1_units);

        // add in P1 hours
        add_time(&year, &month, &day, &hour, &minute, &second, p1, p1_units);

        // save end of end of period time
        save_time(year,month,day,hour,minute,second, sec[4]+34);

    	p[41] = 1;				// n = 1
	uint_char(58,p);			// n=1 -> new size of sec4
	p[46] = 0;				// average
	p[47] = 1;
	p[48] = p1_units;			// hours
	uint_char((NAve-1)*p2,p+49);		// time period
	p[53] = p1_units;			// days
	uint_char(p2,p+54);			// dt = 1 day
	return 0;
    }

    if (stat_proc1 == 195 || (stat_proc1 == 255 && stat_proc2 == 195) ||
        stat_proc1 == 197 || (stat_proc1 == 255 && stat_proc2 == 197)) {

	// specification 1 - average at 24 hour intervals

	p[46] = 0;			// average
        if (stat_proc1 == 195 || (stat_proc1 == 255 && stat_proc2 == 195) )
		p[46] = 1;			// accumulation

	p[47] = 1;			// update forecast time
	p[48] = 2;			// time unit = days
	uint_char((NAve-1),p+49);	// time period
	p[53] = 2;			// day
	uint_char(1,p+54);		// time period

	// specification 2

	p[58] = 0;			// average
        if (stat_proc1 == 195 || (stat_proc1 == 255 && stat_proc2 == 195) )
		p[58] = 1;			// accumulation

	// TMAX and TMIN
        if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 0
                && (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 4))
		p[58] = 2;			// max
        if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 0
                && (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 5))
		p[58] = 3;			// min

	// QMAX and QMIN
        if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 1
                && (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 219))
		p[58] = 2;			// max
        if (GB2_Discipline(sec) == 0 && GB2_Center(sec) == NCEP && GB2_ParmCat(sec) == 1
                && (GB2_MasterTable(sec) <= 5) && (GB2_ParmNum(sec) == 220))
		p[58] = 3;			// min

	p[59] = 2;			// same forecast
	// p[60]  time units is unchanged
	uint_char(p2_m_p1,p+61);	// time period
	p[65] = p[60];			// time units
	uint_char(0,p+66);		// time period

	p[17] = p[60];			// forecast time units
	uint_char(p1,p+18);		// forecast hour

        // get reference time
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

        // add in (NAve-1) * 24 hours
        add_time(&year, &month, &day, &hour, &minute, &second, (NAve-1)*24, 1);

        // add in P2 hours
        add_time(&year, &month, &day, &hour, &minute, &second, p2, 1);

        // save end of end of period time
        save_time(year,month,day,hour,minute,second, sec[4]+34);

	return 0;

    }

    return 0;

//  old code -- not great fix up end of interval time

    if (type == 193 || type == 194 || type == 195 || type == 196 || type == 197
	|| type == 203 || type  == 204 || type == 205 || type == 207) {

	// get reference time
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

	// add in forecast time

        units = sec[4][17];
        dtime = uint4(sec[4]+18);
        add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);

	// add (Nave -1)*dt
	if (type == 194 || type == 196) {
	    units = 2;			// dt = 1 day
	    dtime = 1;
	}
	else if (type == 203 || type == 204) {
	    units = 1;			// dt = 6 hours
	    dtime = 6;
	}
	else if (type == 205 || type == 206) {
	    units = 1;			// dt = 12 hours
	    dtime = 12;
	}
	else {
            units = p[53];
            dtime = uint4(p+54);
	}

        dtime *= (NAve-1);
        add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);

	// if n == 2, add to time
	if (n == 2) {
            if (stat_proc2 == 194 || stat_proc2 == 196) {
                units = 2;                  // dt = 1 day
                dtime = 1;
            }
            else if (stat_proc2 == 203 || stat_proc2 == 204) {
                units = 1;                  // dt = 6 hours
                dtime = 6;
            }
            else if (stat_proc2 == 205 || stat_proc2 == 206) {
                units = 1;                  // dt = 12 hours
                dtime = 12;
            }
            else {
	        units = sec[4][60];
	        dtime = uint4(p+61);
            }
            add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);
	}
	if (sec[4][41] > 2) {
	    fprintf(stderr,"Help fix_ncep n = %d\n", (int) sec[4][41] );
	}

        save_time(year,month,day,hour,minute,second, sec[4]+34);
    }

    
    if (type == 192 && n == 2) {		// climo

        // get reference time
        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);

        // add in forecast time

        units = sec[4][17];
        dtime = uint4(sec[4]+18);
        add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);

        if (sec[4][41] == 2) {
            units = sec[4][60];
            dtime = uint4(p+61);
            add_time(&year, &month, &day, &hour, &minute, &second, dtime, units);
        }
        if (sec[4][41] > 2) {
            fprintf(stderr,"Help fix_ncep n = %d\n", (int) sec[4][41] );
        }

        save_time(year,month,day,hour,minute,second, sec[4]+34);



	p[46] = 51;
	p[47] = 1;

	p[48] = 4;		// outer loop NAve years long
	u = NAve;
	uint_char(u,p+49);

	p[53] = 4;		// 1 year between fields
	u = 1;
	uint_char(u,p+54);

    }

    if (type == 193) {
	p[46] = 0;		// average
	p[47] = 1;
	u = uint4(p+49) * uint4(p+54);
	uint_char(u, p+49);
    }

    if (type == 194) {		// ref cnvgrib
        p[46] = 0;	// average
	p[48] = 2;	// units=day
	p[53] = 2;	// units=day
	u = 1;
	uint_char(u, p+54);
        p[58] = 1;	// accumulation
    }

    if (type == 195) {
	p[58] =  1; // accumulation
	p[59] = 2;

	p[60] = p[53];
	p[61] = p[54];
	p[62] = p[55];
	p[63] = p[56];
	p[64] = p[57];

	p[46] = 0;
	u = uint4(p+49) * uint4(p+54);
	uint_char(u, p+49);
    }

    if (type == 196) {	// cnvgrib
        p[46] = 0;	// average
	p[48] = 2;	// day
	p[53] = 1;	// hour
        p[58] = 0;	// average
    }

    if (type == 197) {
        p[58] =  0; // average
        p[59] = 2;

        p[60] = p[53];
        p[61] = p[54];
        p[62] = p[55];
        p[63] = p[56];
        p[64] = p[57];

        p[46] = 1;		// accumulation
        p[47] = 2;		// ft+
        u = uint4(p+49) * uint4(p+54);
        uint_char(u, p+49);

    }

    return 0;
}

