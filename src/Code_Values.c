/** @file
 * @brief This file contains nice to know values
 * @author Public Domain: Wesley Ebisuzaki @date 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:-1:number_of_coordinate_values_after_template:inv:0:
 */

/**
 * Prints the number of values (4 byte) in the "optional list of coordinate values". 
 * 
 * ## Usage
 * -number_of_coordinate_values_after_template
 * 
 * Prints section 4, octets 6-7 as an unsigned integer. It is expected that each value uses 4 
 * octets of storage at the end of Section 4.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @note Vertical coordinates can be easy like "400 hPa", "2 meters above ground" or 
 * "sigma=0.4". However, the situation is more complicated with the vertical coordinates 
 * in the models. In these model coordinates, the vertical are usually an integer. 
 * You need some extra information to locate the level in physical space. This information
 * is stored in the PDT (section 4) as the "optional list of coordinate values" at the end of the
 * product definition template.
 * 
 * @note The WMO documentation only specifies the hybrid coordinate values should be in 
 * pairs of IEEE single precision floats. 
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $  wgrib2 -number_of_coordinate_values_after_template COSMO_EU_1rec.grib2 -get_byte 4 6 2
 * 1:0:number_coordinates_values_in_pdt=45:4-6=0,45
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int f_number_of_coordinate_values_after_template(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,"number_coordinates_values_in_pdt=%d", 
            number_of_coordinate_values_after_template(sec));
    }
    return 0;
}

/**
 * Returns the number of coordinate values (4 byte) after the template in section 4.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of coordinate values after the template.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_coordinate_values_after_template(unsigned char **sec) {
   int n;
   n=0;
   n = uint2(sec[4]+5);
   return n;
}

/*
 * HEADER:-1:pds_fcst_time:inv:0:fcst_time(1) in units given by pds
 */

/**
 * Prints out the value of the forecast time.
 * 
 * Most grib messages include a forecast time which is stored in the Product 
 * Definition Templates (PDT). When the PDT has a forecast time, it stores the number 
 * as a 4 byte integer and the units in Code Table 4.4. For example, a 12 hour forecast 
 * could be stored with the number 12 and the units of 1 which corresponds hour. Note 
 * that the same forecast time can be stored in several ways. For example, 12 hours can 
 * be stored as:
 * 
 *  - value=720 , units = 0 (minutes)
 *  - value=12 , units = 1 (hours)
 *  - value=4 , units = 10 (3-hours)
 *  - value=2 , units = 11 (6-hours)
 *  - value=1 , units = 12 (12-hours)
 *  - value=43299, units = 13 (seconds)
 * 
 * The unusual 3-hour, 6-hour and 12-hour time units are a legacy of the grib version 1 standard. 
 * Note that the value is a signed integer because the grib standard allows negative forecast 
 * hours which can make sense in data assimilation. 
 * 
 * ## Usage
 * -pds_fcst_time
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Example:
 * @code{.sh}
 * $ wgrib2 percentile_precip.grib2 -s -pds_fcst_time -code_table_4.4
 * 1:0:d=2014101012:TPRATE:surface:2@1 hour max(13-14 hour acc fcst)++,missing=0:75% level:pds_fcst_time1=13:code table 4.4=1 (hour)
 * 2:315649:d=2014101012:TPRATE:surface:2@1 hour max(13-14 hour acc fcst)++,missing=0:90% level:pds_fcst_time1=13:code table 4.4=1 (hour)
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int f_pds_fcst_time(ARG0) {
    int p;

    if (mode >= 0 && code_table_4_4_location(sec)) {
        p = forecast_time_in_units(sec);
        sprintf(inv_out,"pds_fcst_time1=%d", p);
    }
    return 0;
}

/**
 * Returns the number of forecasts in the ensemble.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of forecasts in the ensemble. Returns -1 if the PDT does not specify
 * a number of forecasts.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_forecasts_in_the_ensemble(unsigned char **sec) {
    unsigned char *p;
    p = number_of_forecasts_in_the_ensemble_location(sec);
    if (p) return (int) *p;
    return -1;
}

/**
 * Returns the location of the number of forecasts (NP)in the ensemble in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of forecasts in the ensemble.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the perturbation number from the GRIB2 message.
 *
 * @param sec Pointer to the section of the GRIB2 message.
 *
 * @return The perturbation number. Returns -1 if the PDT does not specify a perturbation number.
 *
 * @author Wesley Ebisuzaki @date 2009
 */
int perturbation_number(unsigned char **sec) {
    unsigned char *p;
    p = perturbation_number_location(sec);
    if (p) return (int) *p;
    return -1;
}

/**
 * Returns the location of the perturbation number in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the perturbation number.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the forecast time in units specified by the Product Definition Template (PDT).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2009 | W. Ebisuzaki | Initial
 * 4/2015 | W. Ebisuzaki | Allow forecast time to be a signed quantity
 *                             Old: return unsigned value, ! code_4_4, return 0xffffffff;
 *                             New: return signed value    ! code_4_4, return 0
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The forecast time in the units specified by the PDT. Returns -1 if the PDT does not specify
 * a forecast time.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns the location of the forecast time in units specified by the Product Definition Template (PDT).
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * @param size Pointer to the size of the forecast time value (2 or 4 bytes).
 * 
 * @return Pointer to the byte in section 4 that contains the forecast time in units specified by the PDT.
 * 
 * @note The size of the forecast time value can be either 2 or 4 bytes, depending on the PDT.
 * WMO made a mistake and pdt 4.44 only uses 2 bytes for the forecast length
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
unsigned char *forecast_time_in_units_location(unsigned char **sec, int *size) {

    unsigned char *code_4_4;

    code_4_4 = code_table_4_4_location(sec);
    if (code_4_4 == NULL) return NULL;
    *size = code_table_4_0(sec) == 44 ? 2 : 4;
    return code_4_4 + 1;
} 

/**
 * Returns the values of fixed surfaces from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * @param type1 Pointer to the type of the first surface.
 * @param surface1 Pointer to the value of the first surface.
 * @param undef_val1 Pointer to the undefined value flag of the first surface.
 * @param type2 Pointer to the type of the second surface.
 * @param surface2 Pointer to the value of the second surface.
 * @param undef_val2 Pointer to the undefined value flag of the second surface.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the background generating process identifier from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The background generating process identifier, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int background_generating_process_identifier(unsigned char **sec) {
    unsigned char *p;
    p = background_generating_process_identifier_location(sec);
    if (p) return (int) *p;
    return -1;
}

/**
 * Returns the location of the background generating process identifier in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the background generating process identifier.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the analysis or forecast generating process identifier from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The analysis or forecast generating process identifier, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int analysis_or_forecast_generating_process_identifier(unsigned char **sec) {
    unsigned char *p;
    p = analysis_or_forecast_generating_process_identifier_location(sec);
    if (p) return (int) *p;
    return -1;
}

/**
 * Returns the location of the analysis or forecast generating process identifier in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the analysis or forecast generating process identifier.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the number of hours of observational data cutoff after reference time.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of hours of observational data cutoff after reference time, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int hours_of_observational_data_cutoff_after_reference_time(unsigned char **sec) {
    unsigned char *p;
    p = hours_of_observational_data_cutoff_after_reference_time_location(sec);
    if (p) return int2(p);
    return -1;
}

/**
 * Returns the location of the hours of observational data cutoff after reference time in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the hours of observational data cutoff after reference time.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the number of minutes of observational data cutoff after reference time.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of minutes of observational data cutoff after reference time, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int minutes_of_observational_data_cutoff_after_reference_time(unsigned char **sec) {
    unsigned char *p;
    p = minutes_of_observational_data_cutoff_after_reference_time_location(sec);
    if (p) return int1(p);
    return -1;
}

/**
 * Returns the location of the minutes of observational data cutoff after reference time in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the minutes of observational data cutoff after reference time.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the observation generating process identifier from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The observation generating process identifier, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int observation_generating_process_identifier(unsigned char **sec) {
    unsigned char *p;
    p = observation_generating_process_identifier_location(sec);
    if (p) return (int) *p;
    return -1;
}

/**
 * Returns the location of the observation generating process identifier in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the observation generating process identifier.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
unsigned char *observation_generating_process_identifier_location(unsigned char **sec) {
    int p;
    p = GB2_ProdDefTemplateNo(sec);
    if (p == 30 || p == 31 || p == 35) return sec[4]+12;
    return NULL;
}

/**
 * Retrieves substitute missing values from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * @param missing1 Pointer to store the first missing value.
 * @param missing2 Pointer to store the second missing value (if applicable).
 * 
 * @return The number of missing values (1 or 2), or 0 if not applicable.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns the location of the statistical time processing section in the GRIB2 message. 
 * (ie location of overall time)
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the statistical time processing information.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns index of n - number of time range specifications in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The index of n - number of time range specifications, or -1 if not applicable.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns the statistical time processing section from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * @param year Pointer to store the year of the statistical time processing.
 * @param month Pointer to store the month of the statistical time processing.
 * @param day Pointer to store the day of the statistical time processing.
 * @param hour Pointer to store the hour of the statistical time processing.
 * @param minute Pointer to store the minute of the statistical time processing.
 * @param second Pointer to store the second of the statistical time processing.
 * 
 * @return 0 on success, 1 if no statistical time processing section is found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the location of the year of the model version date in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the year of the model version date, or NULL if not applicable.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Prints the percentile value, if available.
 * 
 * ## Usage:
 * -percent
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Example:
 * 
 * @code{.sh}
 * $ wgrib2 example.grb2 
 * 1:0:d=2026060100:WIND:10 m above ground:1 hour fcst:0% level
 * 2:1594550:d=2026060100:WIND:10 m above ground:1 hour fcst:5% level
 * 3:3416868:d=2026060100:WIND:10 m above ground:1 hour fcst:10% level
 * ...
 * 19:32573956:d=2026060100:WIND:10 m above ground:1 hour fcst:90% level
 * 20:34624043:d=2026060100:WIND:10 m above ground:1 hour fcst:95% level
 * 21:36674130:d=2026060100:WIND:10 m above ground:1 hour fcst:100% level
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 example.grb2 -percent
 * 1:0:0%
 * 2:1594550:5%
 * 3:3416868:10%
 * ...
 * 19:32573956:90%
 * 20:34624043:95%
 * 21:36674130:100%
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int f_percent(ARG0) {
    int percent;
    if (mode >= 0) {
        percent = percentile_value(sec);
        if (percent >= 0) sprintf(inv_out,"%d%%",percent);
    }
    return 0;
}

/**
 * Returns the percentile value from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The percentile value as an integer, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int percentile_value(unsigned char **sec) {
    unsigned char *p;
    p = percentile_value_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Returns the location of the percentile value in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the percentile value, or NULL if not applicable.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns reference value, binary scaling, decimal scaling, and number of bits from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * @param ref_value Pointer to store the reference value.
 * @param decimal_scaling Pointer to store the decimal scaling factor.
 * @param binary_scaling Pointer to store the binary scaling factor.
 * @param nbits Pointer to store the number of bits.
 * 
 * @return 0 on success, 1 if the packing type specified in the PDT section is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns the number of particle size distributions used by templates 4.57, 4.58, 4.67, and 4.68.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of particle size distributions, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_mode(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    if (pdt == 57 || pdt == 58 || pdt == 67 || pdt == 68) return (int) uint2(sec[4]+13);
    return -1;
}

/**
 * Returns particle size distribution (mode) number for templates 4.57, 4.58, 4.67, and 4.68.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The mode number as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int mode_number(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    if (pdt == 57 || pdt == 58 || pdt == 67 || pdt == 68) return (int) uint2(sec[4]+15);
    return -1;
}

/**
 * Returns the number of following function parameters (NP) for templates 4.57, 4.58, 4.67, and 4.68.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of following distribution parameters (NP), or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_following_distribution_parameters_np(unsigned char **sec) {
    unsigned char *p;
    p = number_of_following_distribution_parameters_np_location(sec);
    if (p) return *p;
    return -1;
}

/**
 * Returns the location of the number of following distribution parameters (Np) for templates 4.57, 4.58, 4.67, and 4.68.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of following distribution parameters (Np), or 
 * NULL if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Prints the type of post-processing applied to the data for templates 4.70 to 4.73.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Usage:
 * -post_processing
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int f_post_processing(ARG0) {
    int i;
    
    if (mode >= 0) {
        i = type_of_post_processing(sec);
        if (i >= 0) sprintf(inv_out,"post_processing=%d", i);
    }
    return 0;
}

/**
 * Returns the type of post-processing applied to the data for templates 4.70 to 4.73.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The type of post-processing as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int type_of_post_processing(unsigned char **sec) {
    int pdt;

    pdt = code_table_4_0(sec);
    if (pdt < 70 || pdt > 73) return -1;
    return (int) sec[4][15];
}

/**
 * Returns the value of cluster identifier for templates 4.3, 4.4, 4.13, and 4.14 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message. (assumes unsigned char)
 * 
 * @return The cluster identifier as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int cluster_identifier(unsigned char **sec) {
    unsigned char *p;
    p = cluster_identifier_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Returns the location of the cluster identifier templates 4.3, 4.4, 4.13, and 4.14 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the cluster identifier, or NULL if the 
 * PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
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

/**
 * Returns the number of clusters for templates 4.3, 4.4, 4.13, and 4.14 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of clusters as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_clusters(unsigned char **sec) {
    unsigned char *p;
    p = number_of_clusters_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Returns the location of the number of clusters in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of clusters, or NULL if the 
 * PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the number of forecasts in the cluster for templates 4.3, 4.4, 4.13, and 4.14 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of forecasts in the cluster as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_forecasts_in_the_cluster(unsigned char **sec) {
    unsigned char *p;
    p = number_of_forecasts_in_the_cluster_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Returns the location of the number of forecasts in the cluster in the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of forecasts in the cluster, or NULL if the
 * PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the location of the list of ensemble forecast numbers (NC) in the GRIB2 message.
 *
 * @param sec Pointer to the section of the GRIB2 message.
 *
 * @return Pointer to the byte in section 4 that contains the list of ensemble forecast numbers, or NULL if 
 * the PDT number is not valid.
 *
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the number of contributing spectral bands (NB) for templates 4.31 to 4.35 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of contributing spectral bands as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_contributing_spectral_bands(unsigned char **sec) {
    unsigned char *p;
    p = number_of_contributing_spectral_bands_location(sec);
    return (p != NULL) ? (int) *p : -1;
}

/**
 * Returns the location of the number of contributing spectral bands (NB) for templates 4.31 to 4.35 
 * from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of contributing spectral bands, or NULL 
 * if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

/**
 * Returns the number of categories for template 4.91 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of categories as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
int number_of_categories(unsigned char **sec) {
    unsigned char *p;
    p = number_of_categories_location(sec);
    return (p != NULL) ? (int) *p : -1;
}

/**
 * Returns the location of the number of categories for template 4.91 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of categories, or NULL 
 * if the PDT number is not valid.

 * @author Wesley Ebisuzaki @date 2009
 */
unsigned char *number_of_categories_location(unsigned char **sec) {
    int pdt;
    pdt = code_table_4_0(sec);
    return (pdt == 91) ? sec[4]+34 : NULL;
}

/** 
 * Returns the number of partitions for templates 4.53 and 4.54 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return The number of partitions as an integer, or -1 if the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
*/
int number_of_partitions(unsigned char **sec) {
    unsigned char *p;
    p = number_of_partitions_location(sec);
    return (p != NULL) ? (int) *p : -1;
}

/**
 * Returns the location of the number of partitions for templates 4.53 and 4.54 from the GRIB2 message.
 * 
 * @param sec Pointer to the section of the GRIB2 message.
 * 
 * @return Pointer to the byte in section 4 that contains the number of partitions, or NULL if 
 * the PDT number is not valid.
 * 
 * @author Wesley Ebisuzaki @date 2009
 */
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

