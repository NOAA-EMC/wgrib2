#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "pds4.h"
#include "grib.h"

/*
 * EC_ext	v1.0 wesley ebisuzaki
 *
 * prints something readable from the EC stream parameter
 *
 * prefix and suffix are only printed if EC_ext has text
 */

void EC_ext(unsigned char *pds, char *prefix, char *suffix, int verbose) {

    int local_id, ec_type, ec_class, ec_stream;
    char string[200];

    if (PDS_Center(pds) != ECMWF) return;

    local_id = PDS_EcLocalId(pds);
    if (local_id  == 0) return;
    ec_class = PDS_EcClass(pds);
    ec_type = PDS_EcType(pds);
    ec_stream = PDS_EcStream(pds);

    if (verbose == 2) printf("%sECext=%d%s", prefix, local_id, suffix);

    if (verbose == 2) {
	switch(ec_class) {
	    case 1: strcpy(string, "operations"); break;
	    case 2: strcpy(string, "research"); break;
	    case 3: strcpy(string, "ERA-15"); break;
	    case 4: strcpy(string, "Euro clim support network"); break;
	    case 5: strcpy(string, "ERA-40"); break;
	    case 6: strcpy(string, "DEMETER"); break;
	    case 7: strcpy(string, "PROVOST"); break;
	    case 8: strcpy(string, "ELDAS"); break;
            default: sprintf(string, "%d", ec_class); break;
	}
        printf("%sclass=%s%s",prefix,string,suffix);
    }
    /*
     10/03/2000: R.Rudsar : subroutine changed.
                 Tests for EcType and extra test for EcStream 1035
    */


/*    if (verbose == 2) { */
        switch(ec_type) {
            case 1: strcpy(string, "first guess"); break;
            case 2: strcpy(string, "analysis"); break;
            case 3: strcpy(string, "init analysis"); break;
            case 4: strcpy(string, "OI analysis"); break;
            /* case 10: strcpy(string, "Control forecast"); break; */
            case 10: sprintf(string, "Control forecast %d",PDS_EcFcstNo(pds)); 
		break;
            case 11:
		/* 5/23/2023 added ec_stream == 1122, thanks S. Villaume (ECMWF) **/
		/* 6/2023 added ec_stream 1120 */
		/* 9/2023 added ec_stream 1081 */
		if (ec_stream == 1035 || ec_stream == 1033 || ec_stream == 1122 || ec_stream == 1120 || ec_stream == 1081) 
              	   sprintf(string, "Perturbed forecast %d",
                   PDS_EcFcstNo(pds)); 
		else
		    strcpy(string, "Perturbed forecasts");
		break;
            case 14: strcpy(string, "Cluster means"); break;
            case 15: strcpy(string, "Cluster std. dev."); break;
            case 16: strcpy(string, "Forecast probability"); break;
            case 17: strcpy(string, "Ensemble means"); break;
            case 18: strcpy(string, "Ensemble std. dev."); break;
    	    case 20: strcpy(string, "Climatology"); break;
            case 21: strcpy(string, "Climatology simulation"); break;
            case 80: strcpy(string, "Fcst seasonal mean"); break;
            default: sprintf(string, "%d", ec_type); break;
        }
        printf("%stype=%s%s",prefix,string,suffix);
/*    }  */
    if (verbose == 2) {
        switch(ec_stream) {
	    case 1033: strcpy(string, "ensemble hindcasts"); break;
	    case 1035: strcpy(string, "ensemble forecasts"); break;
	    case 1043: strcpy(string, "mon mean"); break;
	    case 1070: strcpy(string, "mon (co)var"); break;
	    case 1071: strcpy(string, "mon mean from daily"); break;
	    case 1090: strcpy(string, "EC ensemble fcsts"); break;
	    case 1091: strcpy(string, "EC seasonal fcst mon means"); break;
	    default:   sprintf(string, "%d", ec_stream); break;
        }
        printf("%sstream=%s%s",prefix,string,suffix);
    }
    if (verbose == 2) {
        printf("%sVersion=%c%c%c%c%s", prefix, *(PDS_Ec16Version(pds)), *(PDS_Ec16Version(pds)+1),
		*(PDS_Ec16Version(pds)+2), *(PDS_Ec16Version(pds)+3), suffix);
        if (local_id == 16) {
	    printf("%sSysVersion=%d%s", prefix, PDS_Ec16SysNum(pds), suffix);
	    printf("%sAvgPeriod=%d%s", prefix, PDS_Ec16AvePeriod(pds), suffix);
	    printf("%sFcstMon=%d%s", prefix, PDS_Ec16FcstMon(pds), suffix);

        }
    }

        if (local_id == 16) {
	    printf("%sEnsem_mem=%d%s", prefix, PDS_Ec16Number(pds), suffix);
	    printf("%sVerfDate=%d%s", prefix, PDS_Ec16VerfMon(pds), suffix);
        }

}
