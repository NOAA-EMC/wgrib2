#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"


#if USE_IPOLATES > 0

/*
 * ncep_grids:
 *
 * part of the new_grid package
 *
 * this routine converts the calls with "ncep grid I" to a regular call
 *
 * understands t382, various ncep grid numbers
 *
 * (arg1, arg2, arg3) -> new (arg1, arg2, arg3) if ncep grid
 */


void ncep_grids(const char **arg1, const char **arg2, const char **arg3) {

   int grid_no;
   if (strcmp(*arg1,"ncep") == 0 && strcmp(*arg2,"grid") == 0) {


	if (strcmp(*arg3,"t1534") == 0) {
            *arg1 = "gaussian";
            *arg2 = "0:3072:0.1171875";
            *arg3 = "89.9103245346627:1536";
	    return;
	}
	if (strcmp(*arg3,"t1148") == 0) {
            *arg1 = "gaussian";
            *arg2 = "0:2304:0.15625";
            *arg3 = "89.8804456827785:1152";
	    return;
	}
	if (strcmp(*arg3,"t190") == 0) {
	    *arg1 = "gaussian";
	    *arg2 = "0:576:0.625";
	    *arg3 = "89.5224045547860:288";
	    return;
	}
	if (strcmp(*arg3,"nam188") == 0) {
	    *arg1 = "lambert:265:25:25";
	    *arg2 = "244.723:2345:2540";
	    *arg3 = "19.299:1597:2540";
	    return;
	}

	if (strcmp(*arg3,"t62") == 0) grid_no = 98;
	else if (strcmp(*arg3,"t126") == 0) grid_no = 126;
	else if (strcmp(*arg3,"t254") == 0) grid_no = 127;
	else if (strcmp(*arg3,"t382") == 0) grid_no = 128;
	else if (strcmp(*arg3,"t574") == 0) grid_no = 129;
	else if (strcmp(*arg3,"t170") == 0) grid_no = 170;
	else grid_no = atoi(*arg3);

	switch(grid_no) {
	    case 2:
		*arg1="latlon";
		*arg2="0:144:2.5";
		*arg3="90:73:-2.5";
		break;

	    case 3: 
		*arg1="latlon";
		*arg2="0:360:1";
		*arg3="90:181:-1";
		break;

	    case 4: 
		*arg1="latlon";
		*arg2="0:720:0.5";
		*arg3="90:361:-0.5";
		break;

	    case 45: 
		*arg1="latlon";
		*arg2="0:288:1.25";
		*arg3="90:145:-1.25";
		break;

	    case 98:
		*arg1 = "gaussian";
		*arg2 = "0:192:1.875";
		*arg3 = "88.5419501372975:94";
		break;

	    case 126:
		*arg1 = "gaussian";
		*arg2 = "0:384:0.9375";
		*arg3 = "89.2767128781058:190";
		break;

	    case 127:	/* T254 grid */
		*arg1 = "gaussian";
		*arg2 = "0:768:0.46875";
		*arg3 = "89.6416480725934:384";
		break;

	    case 128:  /* t382 gaussian grid */
		*arg1 = "gaussian";
		*arg2 = "0:1152:0.3125";
		*arg3 = "89.7609950778017:576";
		break;

	    case 129:  /* t574 gaussian grid */
		*arg1 = "gaussian";
		*arg2 = "0:1760:0.20454545454545";
		*arg3 = "89.8435135178685:880";
		break;

	    case 170:  /* t170 gaussian grid */
		*arg1 = "gaussian";
		*arg2 = "0:512:0.703125";
		*arg3 = "89.4628215685774:256";
		break;

	    case 173:
		*arg1 = "latlon";
		*arg2 = "0.041666666667:4320:0.083333333333";
		*arg3 = "89.95833333333:2160:-0.083333333333";
		break;

            case 184:
                *arg1 = "lambert:265:25";
                *arg2 = "238.446:2145:2540";
                *arg3 = "20.192:1377:2540";
                break;

	    case 194:
		*arg1 = "mercator:20";
		*arg2 = "284.5:544:2500:297.491";
		*arg3 = "15:310:2500:22.005";
		break;

	    case 221:
		*arg1="lambert:253:50";
		*arg2="214.5:349:32463.41";
		*arg3="1:277:32463.41";
		break;

	    case 230:
		*arg1="latlon";
		*arg2="0:720:0.5";
		*arg3="90:361:-0.5";
		break;

	    case 242:
		*arg1="nps:225:60";
		*arg2="187:553:11250";
		*arg3="30:425:11250";
		break;

	    case 249:
		*arg1="nps:210.0:60.0";
		*arg2="188.4:367:9867.89";
		*arg3="45.4:343:9867.89";
		break;
	}
    }
   return;
}
#endif
