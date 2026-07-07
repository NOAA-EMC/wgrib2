#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "pds4.h"
#include "grib.h"

/*
 * PDStimes.c   v1.2 wesley ebisuzaki
 *
 * prints something readable for time code in grib file
 *
 * not all cases decoded
 * for NCEP/NCAR Reanalysis
 *
 * v1.2.1 1/99 fixed forecast time unit table
 * v1.2.2 10/01 add time_range = 11 (at DWD)  Helmut P. Frank
 * v1.2.3 10/05 add time units 13 = 15 min, 14 = 30 min, and
 *              time range 13 = nudging analysis, 14 = relabeled forecast
 *              (at DWD), Helmut P. Frank
 */

static char *units[] = {
	"min", "hr", "d", "mon", "yr",
	"decade", "normal", "century", "??", "??", " x3 hours", " x6 hours",
        " x12 hours",
        "x15 min", "x30 min", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", " sec"}; 

void PDStimes(int time_range, int p1, int p2, int time_unit) {

	char *unit;
	enum {anal, fcst, unknown} type;
	int fcst_len = 0;

	if (time_unit >= 0 && time_unit <= sizeof(units)/sizeof(char *))
             unit = units[time_unit];
	else unit = "";

        /* change x3/x6/x12 to hours */

        if (time_unit == HOURS3) {
	    p1 *= 3; p2 *= 3;
	    time_unit = HOUR;
        }
        else if (time_unit == HOURS6) {
	    p1 *= 6; p2 *= 6;
	    time_unit = HOUR;
        }
        else if (time_unit == HOURS12) {
	    p1 *= 12; p2 *= 12;
	    time_unit = HOUR;
        }
	else if (time_unit == MINUTES30) {
	    p1 *= 30; p2 *= 30;
	    time_unit = MINUTE;
        }
	else if (time_unit == MINUTES15) {
	    p1 *= 15; p2 *= 15;
	    time_unit = MINUTE;
        }
	/* turn off 5/13/2010 
	if (time_unit == MINUTE && p1 % 60 == 0 && p2 % 60 == 0) {
	    p1 /= 60; p2 /= 60;
	    time_unit = HOUR;
        }
	*/

	if (time_unit >= 0 && time_unit <= sizeof(units)/sizeof(char *))
             unit = units[time_unit];
	else unit = "";

	/* figure out if analysis or forecast */
	/* in GRIB, there is a difference between init and uninit analyses */
	/* not case at NMC .. no longer run initialization */
	/* ignore diff between init an uninit analyses */

	switch (time_range) {

	case 0:
	case 1:
	case 113:
	case 114:
	case 118:
		if (p1 == 0) type = anal;
		else {
			type = fcst;
			fcst_len = p1;
		}
		break;
	case 10: /* way NMC uses it, should be unknown? */
		type = fcst;
		fcst_len = p1*256 + p2;
		if (fcst_len == 0) type = anal;
		break;

	case 51:
		type = unknown;
		break;
	case 123:
	case 124:
		type = anal;
		break;

	case 135:
		type = anal;
		break;

	default: type = unknown;
		break;
	}

	/* ----------------------------------------------- */

	if (type == anal) printf("anl:");
	else if (type == fcst) printf("%d%s fcst:",fcst_len,unit);


	if (time_range == 123 || time_range == 124) {
		if (p1 != 0) printf("start@%d%s:",p1,unit);
	}


	/* print time range */


	switch (time_range) {

	case 0:
	case 1:
	case 10:
		break;
	case 2: printf("valid %d-%d%s:",p1,p2,unit);
		break;
	case 3: printf("%d-%d%s ave:",p1,p2,unit);
		break;
	case 4: printf("%d-%d%s acc:",p1,p2,unit);
		break;
	case 5: printf("%d-%d%s diff:",p1,p2,unit);
		break;
        case 6: printf("-%d to -%d %s ave:", p1,p2,unit);
                break;
        case 7: printf("-%d to %d %s ave:", p1,p2,unit);
                break;
	case 11: if (p1 > 0) {
		    printf("init fcst %d%s:",p1,unit);
		}
		else {
	            printf("time?:");
		}
		break;
	case 13: printf("nudge ana %d%s:",p1,unit);
		break;
	case 14: printf("rel. fcst %d%s:",p1,unit);
		break;
	case 51: if (p1 == 0) {
		    /* printf("clim %d%s:",p2,unit); */
		    printf("0-%d%s product:ave@1yr:",p2,unit);
		}
		else if (p1 == 1) {
		    /* printf("clim (diurnal) %d%s:",p2,unit); */
		    printf("0-%d%s product:same-hour,ave@1yr:",p2,unit);
		}
		else {
		    printf("clim? p1=%d? %d%s?:",p1,p2,unit);
		}
		break;
	case 113:
	case 123:
		printf("ave@%d%s:",p2,unit);
		break;
	case 114:
	case 124:
		printf("acc@%d%s:",p2,unit);
		break;
	case 115:
		printf("ave of fcst:%d to %d%s:",p1,p2,unit);
		break;
	case 116:
		printf("acc of fcst:%d to %d%s:",p1,p2,unit);
		break;
	case 118: 
		printf("var@%d%s:",p2,unit);
		break;
	case 128:
		printf("%d-%d%s fcst acc:ave@24hr:", p1, p2, unit);
		break;
	case 129:
		printf("%d-%d%s fcst acc:ave@%d%s:", p1, p2, unit, p2-p1,unit);
		break;
	case 130:
		printf("%d-%d%s fcst ave:ave@24hr:", p1, p2, unit);
		break;
	case 131:
		printf("%d-%d%s fcst ave:ave@%d%s:", p1, p2, unit,p2-p1,unit);
		break;
		/* for CFS */
	case 132:
		printf("%d-%d%s anl:ave@1yr:", p1, p2, unit);
		break;
	case 133:
		printf("%d-%d%s fcst:ave@1yr:", p1, p2, unit);
		break;
	case 134:
		printf("%d-%d%s fcst-anl:rms@1yr:", p1, p2, unit);
		break;
	case 135:
		printf("%d-%d%s fcst-fcst_mean:rms@1yr:", p1, p2, unit);
		break;
	case 136:
		printf("%d-%d%s anl-anl_mean:rms@1yr:", p1, p2, unit);
		break;
	case 137:
		printf("%d-%d%s fcst acc:ave@6hr:", p1, p2, unit);
		break;
	case 138:
		printf("%d-%d%s fcst ave:ave@6hr:", p1, p2, unit);
		break;
	case 139:
		printf("%d-%d%s fcst acc:ave@12hr:", p1, p2, unit);
		break;
	case 140:
		printf("%d-%d%s fcst ave:ave@12hr:", p1, p2, unit);
		break;
		
	default: printf("time?:");
	}
}
