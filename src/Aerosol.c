/** @file
 * @brief Functions for handling aerosol size and wavelength in GRIB files.
 * @author Public Domain: Wesley Ebisuzaki @date 02/2012
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 02/2012 | W. Ebisuzaki | Initial
 * 10/2024 | W. Ebisuzaki | Fix to handle all pdts
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:200:aerosol_size:inv:0:optical properties of an aerosol
 */

/**
 * Prints the size of the aerosol particle if applicable. This product applies to Product Definition 
 * Template 4.44-4.48. This option is part of the standard inventory.
 * 
 * ## Usage
 * -aerosol_size
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, does not return error codes.
 * 
 * ## Example: 
 * @code{.sh}
 * $ wgrib2 ngac.t00z.a3df03  -for 2:6
 * 2:77259:d=2014081000:TMP:1 hybrid level:3 hour fcst:
 * 3:131943:d=2014081000:RH:1 hybrid level:3 hour fcst:
 * 4:205847:d=2014081000:MASSMR:1 hybrid level:3 hour fcst:aerosol=Dust Dry:aerosol_size >=2e-07,<2e-06:
 * 5:243914:d=2014081000:MASSMR:1 hybrid level:3 hour fcst:aerosol=Dust Dry:aerosol_size >=2e-06,<3.6e-06:
 * 6:279272:d=2014081000:MASSMR:1 hybrid level:3 hour fcst:aerosol=Dust Dry:aerosol_size >=3.6e-06,<6e-06:
 * @endcode
 * 
 * Message 4: mass mixing ratio (MASSMR) of Dry Dust, particle size ranges from 2e-7 meters to 2e-6 meters
 * 
 * @code{.sh}
 * $ wgrib2 ngac.t00z.a3df03  -for 2:6 -aerosol_size
 * 2:77259:
 * 3:131943:
 * 4:205847:aerosol_size <=2e-07,>2e-06
 * 5:243914:aerosol_size <=2e-06,>3.6e-06
 * 6:279272:aerosol_size <=3.6e-06,>6e-06
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 02/2012
 */
int f_aerosol_size(ARG0) {
    unsigned char *ptr;
    double size1, size2;

    if (mode >= 0) {
        ptr = code_table_4_91_location(sec);
        if (ptr == NULL) return 0;
        if (*ptr == 255) return 0;
        size1 = scaled2dbl(INT1(ptr[1]), int4(ptr+2));
        size2 = scaled2dbl(INT1(ptr[6]), int4(ptr+7));

        sprintf(inv_out,"aerosol_size ");
        inv_out += strlen(inv_out);
        prt_code_table_4_91(ptr[0], size1, size2, inv_out);
    }
    return 0;
}
/*
 * HEADER:200:aerosol_wavelength:inv:0:optical properties of an aerosol
 */

/**
 * Prints the optical properties of the aerosol particle. This option is part of the standard inventory.
 * 
 * 
 * ## Usage
 * -aerosol_wavelength
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, does not return error codes.
 * 
 * ## Example: 
 * @code{.sh}
 * $ wgrib2 wrib2 ngac.t00z.a2df03  -aerosol_wavelength -d 1 
 * 1:0:aerosol_wavelength <=5.45e-07,>=5.65e-07
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 02/2012
 */
int f_aerosol_wavelength(ARG0) {
    unsigned char *ptr;
    double wave1, wave2;

    if (mode >= 0) {
        ptr = code_table_4_91b_location(sec);
        if (ptr == NULL) return 0;
        if (*ptr == 255) return 0;
        wave1 = scaled2dbl(INT1(ptr[1]), int4(ptr+2));
        wave2 = scaled2dbl(INT1(ptr[6]), int4(ptr+7));

        sprintf(inv_out,"aerosol_wavelength ");
        inv_out += strlen(inv_out);
        prt_code_table_4_91(ptr[0], wave1, wave2, inv_out);

    }
    return 0;
}

