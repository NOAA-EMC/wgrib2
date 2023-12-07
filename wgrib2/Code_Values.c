#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 2016 Public Domain Wesley Ebisuzaki
 *
 * this file contains nice to know values 
 */

/*
 * HEADER:-1:number_of_coordinate_values_after_template:inv:0:
 */
int f_number_of_coordinate_values_after_template(ARG0) {
    if (mode >= 0) {
	sprintf(inv_out,"number_coordinates_values_in_pdt=%d", 
		number_of_coordinate_values_after_template(sec));
    }
    return 0;
}
int number_of_coordinate_values_after_template(unsigned char **sec) {
   int n;
   n=0;
   n = uint2(sec[4]+5);
   return n;
}

/*
 * HEADER:-1:pds_fcst_time:inv:0:fcst_time(1) in units given by pds
 */
int f_pds_fcst_time(ARG0) {
    int p;

    if (mode >= 0 && code_table_4_4_location(sec)) {
        p = forecast_time_in_units(sec);
	sprintf(inv_out,"pds_fcst_time1=%d", p);
    }
    return 0;
}

int number_of_forecasts_in_the_ensemble(unsigned char **sec) {
    unsigned char *p;
    p = number_of_forecasts_in_the_ensemble_location(sec);
    if (p) return (int) *p;
    return -1;
}

unsigned char *number_of_forecasts_in_the_ensemble_location(unsigned char **sec) {
    int pdt, nb, np;
    unsigned char *p;

    pdt = code_table_4_0(sec);
    switch(pdt) {
	case 1:
	case 11:
		p = sec[4]+36; break;
	case 2:
	case 3:
	case 4:
	case 12:
	case 13:
	case 14:
		p = sec[4]+35; break;
	case 33:
	case 34:
		nb = sec[4][22];
		p = sec[4] + 25 + 11*nb; break;
	case 41:
	case 43:
		p = sec[4]+38; break;
	case 45:
	case 47:
		p = sec[4]+49; break;
	case 49:
		p = sec[4]+60; break;
	case 54:
		np = sec[4][12];
		p = sec[4]+40+2*np; break;
	case 56:
	case 59:
	case 71:
	case 73:
		p = sec[4]+41; break;
	case 58:
	case 68:
		np = sec[4][19];
		p = sec[4]+45+5*np; break;
	case 60:
	case 61:
		p = sec[4]+36; break;
	case 63:
		p = sec[4] + 42; break;
	default: p=NULL; break;
    }
    return p;
}

int perturbation_number(unsigned char **sec) {
    unsigned char *p;
    p = perturbation_number_location(sec);
    if (p) return (int) *p;
    return -1;
}

unsigned char *perturbation_number_location(unsigned char **sec) {
    int pdt, nb, np;
    unsigned char *p;

    pdt = code_table_4_0(sec);
    switch(pdt) {
	case 1:
	case 11:
	case 60:
	case 61:
		p = sec[4]+35; break;
	case 33:
	case 34:
		nb = sec[4][22];
		p = sec[4] + 24 + 11*nb; break;
	case 41:
	case 43:
		p = sec[4]+37; break;
	case 45:
	case 47:
		p = sec[4]+48; break;
	case 49:
		p = sec[4]+59; break;
	case 54:
		np = sec[4][12];
		p = sec[4]+39+2*np; break;
	case 56:
	case 71:
	case 73:
		p = sec[4]+40; break;
	case 58:
	case 68:
		np = sec[4][19];
		p = sec[4]+44+5*np; break;

	case 59:
	case 63:
		p = sec[4]+41; break;
	default: p = NULL; break;
    }
    return p;
}

/*
 * forecast_time_in_units
 *
 * v1.1 4/2015:  allow forecast time to be a signed quantity
 *      old: return unsigned value, ! code_4_4, return 0xffffffff;
 *      new: return signed value    ! code_4_4, return 0
 */

int forecast_time_in_units(unsigned char **sec) {

    unsigned char *p;
    int size;

    if ((p = forecast_time_in_units_location(sec, &size)) == NULL) return 0;
    if (size == 4) {
	return int4(p);
    }
    else if (size == 2) {
	return int4(p);
    }
    else fatal_error_i("forecast_time_in_units size=%d?", size);
    return 0;
}

/*
 * WMO made a mistake and pdt 4.44 only uses 2 bytes for the forecast length
 */

unsigned char *forecast_time_in_units_location(unsigned char **sec, int *size) {

    unsigned char *code_4_4;

    code_4_4 = code_table_4_4_location(sec);
    if (code_4_4 == NULL) return NULL;
    *size = code_table_4_0(sec) == 44 ? 2 : 4;
    return code_4_4 + 1;
} 

/* returns the values of fixed surfaces */

void fixed_surfaces(unsigned char **sec, int *type1, float *surface1, 
	int *undef_val1, int *type2, float *surface2, int *undef_val2) {

    unsigned char *p1, *p2;
    *undef_val1 = *undef_val2 = 1;
    *surface1 = *surface2 = UNDEFINED;
    *type1 = *type2 = 255;

    p1 = code_table_4_5a_location(sec);
    p2 = code_table_4_5b_location(sec);

    if (p1 != NULL && p1[0] != 255) {
	*type1 = *p1;
        if (p1[1] != 255) {
	    if (p1[2] != 255 || p1[3] != 255 || p1[4] != 255 || p1[5] != 255) {
		*undef_val1 = 0;
		*surface1 = scaled2flt(INT1(p1[1]), int4(p1+2));
	    }
	}
    }
    if (p2 != NULL && *p2 != 255) {
	*type2 = *p2;
        if (p2[1] != 255) {
	    if (p2[2] != 255 || p2[3] != 255 || p2[4] != 255 || p2[5] != 255) {
		*undef_val2 = 0;
                *surface2 = scaled2flt(INT1(p2[1]), int4(p2+2));
	    }
	}
    }
    return;
}

int background_generating_process_identifier(unsigned char **sec) {
    unsigned char *p;
    p = background_generating_process_identifier_location(sec);
    if (p) return (int) *p;
    return -1;
}
unsigned char *background_generating_process_identifier_location(unsigned char **sec) {
    int p, np, center;
    p = GB2_ProdDefTemplateNo(sec);
    center = GB2_Center(sec);

    if (p <= 15 || (p >= 32 && p <= 34) || p == 51 || p == 60 || p == 61 || p == 91 || p == 1000 || p == 1001 || p == 1002 || p == 1100 || p == 1101)
        return sec[4]+12;
    if (p >= 40 && p <= 43) return sec[4]+14;
    if (p >= 44 && p <= 47) return sec[4]+25;
    if (p == 48 || p == 49) return sec[4]+36;
    if (p == 52) return sec[4]+15;
    if (p == 53 || p == 54) { np = sec[4][12]; return sec[4]+16+2*np; }
    if (p == 55 || p == 56 || p == 59 || p == 62 || p == 63) return sec[4]+18;
    if (p == 57 || p == 58 || p == 67 || p == 68) { np = sec[4][19]; return sec[4]+21+5*np; }
    if (p >= 70 && p <= 73) return sec[4]+17;

    if ((center == JMA1) || (center == JMA2)) {
        switch(p) {
            case 50000:
            case 50002:
            case 50008:
            case 50009:
            case 50010:
            case 50011:
            case 50012:
            case 50020:
            case 51020:
            case 51021:
            case 51022:
            case 51122:
            case 51123:
            case 52020:
                return sec[4]+12;
        }
    }

    return NULL;
}


int analysis_or_forecast_generating_process_identifier(unsigned char **sec) {
    unsigned char *p;
    p = analysis_or_forecast_generating_process_identifier_location(sec);
    if (p) return (int) *p;
    return -1;
}
unsigned char *analysis_or_forecast_generating_process_identifier_location(unsigned char **sec) {
    int p, np, center;

    p = GB2_ProdDefTemplateNo(sec);

    if (p <= 15 || (p >= 32 && p <= 34) || p == 51 || p == 60 || p == 61 || p == 91 || p == 1000 || p == 1001 || 
		    p == 1002 || p == 1100 || p == 1101)
        return sec[4]+13;
    if (p >= 40 && p <= 43) return sec[4]+15;
    if (p >= 44 && p <= 47) return sec[4]+26;
    if (p == 48 || p == 49) return sec[4]+37;
    if (p == 52) return sec[4]+16;
    if (p == 53 || p == 54) { np = sec[4][12]; return sec[4]+17+2*np; }
    if (p == 55 || p == 56 || p == 59 || p == 62 || p == 63) return sec[4]+19;
    if (p == 57 || p == 58 || p == 67 || p == 68) { np = sec[4][19]; return sec[4]+22+5*np; }
    if (p >= 70 && p <= 73) return sec[4]+18;


    center = GB2_Center(sec);
    if ((center == JMA1) || (center == JMA2)) {
	switch(p) {
	case 50000:
	case 50008:
	case 50009:
	case 50010:
	case 50011:
	case 50012:
		return sec[4]+13;
	default: break;
	}
    }

    return NULL;
}

int hours_of_observational_data_cutoff_after_reference_time(unsigned char **sec) {
    unsigned char *p;
    p = hours_of_observational_data_cutoff_after_reference_time_location(sec);
    if (p) return int2(p);
    return -1;
}

unsigned char *hours_of_observational_data_cutoff_after_reference_time_location(unsigned char **sec) {
    int p, np;
    p = GB2_ProdDefTemplateNo(sec);
    if (p <= 15 || (p >= 32 && p <= 34) || p == 51 || p == 60 || p == 61 || p == 91 || p == 1000 || p == 1001 || p == 1002 || p == 1100 || p == 1101)
        return sec[4]+14;
    if (p >= 40 && p <= 43) return sec[4]+16;
    if (p >= 44 && p <= 47) return sec[4]+27;
    if (p == 48 || p == 49) return sec[4]+38;
    if (p == 52) return sec[4]+17;
    if (p == 53 || p == 54) { np = sec[4][12]; return sec[4]+18+2*np; }
    if (p == 55 || p == 56 || p == 59 || p == 62 || p == 63) return sec[4]+20;
    if (p == 57 || p == 58 || p == 67 || p == 68) { np = sec[4][19]; return sec[4]+23+5*np; }
    if (p >= 70 && p <= 73) return sec[4]+19;

    return NULL;
}

int minutes_of_observational_data_cutoff_after_reference_time(unsigned char **sec) {
    unsigned char *p;
    p = minutes_of_observational_data_cutoff_after_reference_time_location(sec);
    if (p) return int1(p);
    return -1;
}

unsigned char *minutes_of_observational_data_cutoff_after_reference_time_location(unsigned char **sec) {
    int p, np;
    p = GB2_ProdDefTemplateNo(sec);
    if (p <= 15 || (p >= 32 && p <= 34) || p == 51 || p == 60 || p == 61 || p == 91 || p == 1000 || p == 1001 || p == 1002 || p == 1100 || p == 1101)
        return sec[4]+16;
    if (p >= 40 && p <= 43) return sec[4]+18;
    if (p >= 44 && p <= 47) return sec[4]+29;
    if (p == 48 || p == 49) return sec[4]+40;
    if (p == 52) return sec[4]+19;
    if (p == 53 || p == 54) { np = sec[4][12]; return sec[4]+20+2*np; }
    if (p == 55 || p == 56 || p == 59 || p == 62 || p == 63) return sec[4]+22;
    if (p == 57 || p == 58 || p == 67 || p == 68) { np = sec[4][19]; return sec[4]+25+5*np; }
    if (p >= 70 && p <= 73) return sec[4]+21;
    return NULL;
}


int observation_generating_process_identifier(unsigned char **sec) {
    unsigned char *p;
    p = observation_generating_process_identifier_location(sec);
    if (p) return (int) *p;
    return -1;
}

unsigned char *observation_generating_process_identifier_location(unsigned char **sec) {
    int p;
    p = GB2_ProdDefTemplateNo(sec);
    if (p == 30 || p == 31 || p == 35) return sec[4]+12;
    return NULL;
}


/*
 * get substitute missing value
 *
 * returns number of missing values
 */

int sub_missing_values(unsigned char **sec, float *missing1, float *missing2) {
    int i, j;
    unsigned char *p;

    i = code_table_5_5(sec);
    if (i < 1 || i > 2) return 0;
    j = code_table_5_1(sec);
    p = sec[5];
    if (j == 0) {		// ieee
	if (p[23] == 255 && p[24] == 255 && p[25] == 255 && p[26] == 255) *missing1 = UNDEFINED;
	else *missing1 = ieee2flt(p+23);
	if (i == 2) {
	    if (p[27] == 255 && p[28] == 255 && p[29] == 255 && p[30] == 255) *missing1 = UNDEFINED;
	    else *missing2 = ieee2flt(p+27);
	}
    }
    else if (j == 1) {		// integer
	if (p[23] == 255 && p[24] == 255 && p[25] == 255 && p[26] == 255) *missing1 = UNDEFINED;
	else *missing1 = (float) int4(p+23);
	if (i == 2) {
	    if (p[27] == 255 && p[28] == 255 && p[29] == 255 && p[30] == 255) *missing1 = UNDEFINED;
	    else *missing2 = (float) int4(p+27);
	}
    }
    return i;
}

/*
 * returns location of statistical time processing section
 *  ie location of overall time
 *
 */

unsigned char *stat_proc_verf_time_location(unsigned char **sec) {
    int pdt, center, nb, np, nc;
    pdt = code_table_4_0(sec);

    if (pdt <= 32767) {		/* WMO defined */
        switch(pdt) {
	case 8:
	    return sec[4] + 34;
	case 9:
	    return sec[4] + 47;
	case 10:
	    return sec[4] + 35;
	case 11:
	    return sec[4] + 37;
	case 12:
	case 42:
	    return sec[4] + 36;
	case 13:
	    return sec[4] + 68;
	case 14:
	    return sec[4] + 64;
	case 34:
    	    nb = sec[4][22];
	    return sec[4] + 26+11*nb;
	case 43:
	case 72:
	    return sec[4] + 39;
	case 46:
	    return sec[4] + 47;		
	case 47:
	    return sec[4] + 50;
	case 61:
	    return sec[4] + 44;
	case 62:
	    return sec[4] + 40;
	case 63:
	    return sec[4] + 43;
	case 67:
	case 68:
	    np = sec[4][19];
	    return sec[4] + 43 + 5*np;
	case 73:
	    return sec[4] + 42;
	case 91:
    	    nc = sec[4][34];
	    return sec[4] + 47 + 12*(nc-1);
	default: return NULL;
	}
    }
    if (pdt == 65535) return NULL; /* missing */
    /* 32768..65534 local use */
    center = GB2_Center(sec);
    if (center == JMA1 || center == JMA2) {
        switch(pdt) {
	case 50008:
	case 50009:
	case 50010:
	case 50011:
	case 50012:
		return sec[4] + 34;
	}
    }
    return NULL;
}

/*
 * index of n ― number of time range specifications
 *
 * if none, then return -1
 */

int stat_proc_n_time_ranges_index(unsigned char **sec) {
    unsigned char *ptr;
    int center, pdt;

    ptr = stat_proc_verf_time_location(sec);
    if (ptr == NULL) return -1;
    /* some JMA pdts have only 1 level of statistical processing */
    center = GB2_Center(sec);
    pdt = code_table_4_0(sec);
    if (center == JMA1 || center == JMA2) {
        if (pdt >= 50008 && pdt <= 50012) return -1;
    }
    return ptr - sec[4] + 7;
}


int stat_proc_verf_time(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second)
{
    unsigned char *stat_proc_time;

    stat_proc_time = stat_proc_verf_time_location(sec);

    if (stat_proc_time) {
	get_time(stat_proc_time, year, month, day, hour, minute, second);
	return 0;
    }
    else {
	*year = *month = *day = *hour = *minute = *second = 0;
    }
    return 1;
}

/*
 * returns the location of the year of the model version date
 */
unsigned char *year_of_model_version_date_location(unsigned char **sec) {
    int pdt;
    unsigned char *p;

    pdt = code_table_4_0(sec);

    switch (pdt) {
      case 60: 
      case 61: p = sec[4] + 37; break;
      default: p = NULL; break;
    }
    return p;
}

/*
 * HEADER:-1:percent:inv:0:percentage probability
 */
int f_percent(ARG0) {
    int percent;
    if (mode >= 0) {
        percent = percentile_value(sec);
        if (percent >= 0) sprintf(inv_out,"%d%%",percent);
    }
    return 0;
}


/*
 * returns the percentile value
 */
int percentile_value(unsigned char **sec) {
	unsigned char *p;
	p = percentile_value_location(sec);
	if (p == NULL) return -1;
	return (int) *p;
}
/*
 * returns location of the percentile value
 */
unsigned char *percentile_value_location(unsigned char **sec) {

    int pdt;
    unsigned char *p;
    pdt = code_table_4_0(sec);
    switch (pdt) {
	case 6:
	case 10: p = sec[4] + 34; break;
	default: p = NULL; break;
    }
    return p;
}

/*
 *  returns reference value, binary and decimal scaling and number of bits
 */

int scaling(unsigned char **sec, double *ref_value, int *decimal_scaling, int *binary_scaling, int *nbits) {
    int pack;
    unsigned char *p;

    pack = (int) code_table_5_0(sec);
    p = sec[5];
    if (pack == 0 || pack == 1 || pack == 2 || pack == 3 || pack == 40 || pack == 41 || pack == 42 ||
                pack == 50 || pack == 51 || pack == 61 || pack == 40000 || pack == 40010) {
       *ref_value = ieee2flt(p+11);
       *binary_scaling = int2(p+15);
       *decimal_scaling = -int2(p+17);
       *nbits = p[19];
    }
    else {
      return 1;
    }
    return 0;
}

/*
 * returns number of particle size distributions used by template 4.57
 */
int number_of_mode(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    if (pdt == 57 || pdt == 58 || pdt == 67 || pdt == 68) return (int) uint2(sec[4]+13);
    return -1;
}

/*
 * returns partical size distribution (mode)  1..number_of_mode for template 4.57
 */

int mode_number(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    if (pdt == 57 || pdt == 58 || pdt == 67 || pdt == 68) return (int) uint2(sec[4]+15);
    return -1;
}

int number_of_following_distribution_parameters_np(unsigned char **sec) {
    unsigned char *p;
    p = number_of_following_distribution_parameters_np_location(sec);
    if (p) return *p;
    return -1;
}

unsigned char *number_of_following_distribution_parameters_np_location(unsigned char **sec) {
    int pdt;

    pdt = code_table_4_0(sec);
    switch(pdt) {
	case 57:   
	case 58:   
	case 67:   
	case 68:   
		return sec[4]+19;
    }
    return NULL;
}

/*
 * HEADER:-1:post_processing:inv:0:type of post-processing
 */
int f_post_processing(ARG0) {
    int i;
    
    if (mode >= 0) {
        i = type_of_post_processing(sec);
        if (i >= 0) sprintf(inv_out,"post_processing=%d", i);
    }
    return 0;
}

int type_of_post_processing(unsigned char **sec) {
    int pdt;

    pdt = code_table_4_0(sec);
    if (pdt < 70 || pdt > 73) return -1;
    return (int) sec[4][15];
}

/* 
 * returns value of cluster identifier (assume unsigned char)
 */
int cluster_identifier(unsigned char **sec) {
        unsigned char *p;
        p = cluster_identifier_location(sec);
        if (p == NULL) return -1;
        return (int) *p;
}
/*
 * returns location of cluster identifier
 */
unsigned char *cluster_identifier_location(unsigned char **sec) {

    int pdt;
    unsigned char *p;
    pdt = code_table_4_0(sec);
    switch (pdt) {
	case 3:
	case 4:
	case 13:
	case 14:
	    p = sec[4] + 36; break;
        default: p = NULL; break;
    }
    return p;
}


int number_of_clusters(unsigned char **sec) {
        unsigned char *p;
        p = number_of_clusters_location(sec);
        if (p == NULL) return -1;
        return (int) *p;
}

unsigned char *number_of_clusters_location(unsigned char **sec) {
    int pdt;
    unsigned char *p;
    pdt = code_table_4_0(sec);
    switch (pdt) {
        case 3:
        case 4:
        case 13:
        case 14:
            p = sec[4] + 39; break;
        default: p = NULL; break;
    }
    return p;
}

int number_of_forecasts_in_the_cluster(unsigned char **sec) {
        unsigned char *p;
        p = number_of_forecasts_in_the_cluster_location(sec);
        if (p == NULL) return -1;
        return (int) *p;
}

unsigned char *number_of_forecasts_in_the_cluster_location(unsigned char **sec) {
    int pdt;
    unsigned char *p;
    pdt = code_table_4_0(sec);
    switch (pdt) {
        case 3:
        case 13:
            p = sec[4] + 57; break;
        case 4:
        case 14:
            p = sec[4] + 53; break;
        default: p = NULL; break;
    }
    return p;
}

unsigned char *list_of_nc_ensemble_forecast_numbers_location(unsigned char **sec) {
    int pdt, n;
    unsigned char *p;
    pdt = code_table_4_0(sec);
    switch (pdt) {
        case 3:
            p = sec[4] + 68; break;
        case 4:
            p = sec[4] + 64; break;
        case 13:
	    n = sec[4][75];
            p = sec[4] + 80 + 12*n; break;
        case 14:
	    n = sec[4][71];
            p = sec[4] + 76 + 12*n; break;
        default: p = NULL; break;
    }
    return p;
}




/* also known as nb (NB) */
int number_of_contributing_spectral_bands(unsigned char **sec) {
        unsigned char *p;
        p = number_of_contributing_spectral_bands_location(sec);
        return (p != NULL) ? (int) *p : -1;
}

unsigned char *number_of_contributing_spectral_bands_location(unsigned char **sec) {
    int pdt;
    unsigned char *p;
    pdt = code_table_4_0(sec);
    switch (pdt) {
	/* ignore 30 because different  case 30: */
	case 31: p = sec[4] + 13; break;
	case 32:
	case 33:
	case 34: p = sec[4] + 22; break;
	case 35: p = sec[4] + 14; break;
        default: p = NULL; break;
    }
    return p;
}

int number_of_categories(unsigned char **sec) {
   unsigned char *p;
   p = number_of_categories_location(sec);
   return (p != NULL) ? (int) *p : -1;
}

unsigned char *number_of_categories_location(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    return (pdt == 91) ? sec[4]+34 : NULL;
}

int number_of_partitions(unsigned char **sec) {
   unsigned char *p;
   p = number_of_partitions_location(sec);
   return (p != NULL) ? (int) *p : -1;
}

unsigned char *number_of_partitions_location(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    switch(pdt) {
	case 53:
	case 54:
		return sec[4] + 12;
    }
    return NULL;
}	

