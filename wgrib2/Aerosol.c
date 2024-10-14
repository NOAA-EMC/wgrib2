#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 2/2012 Public Domain: Wesley Ebisuzaki
 * 10/2024               fix to handle all pdts Wesley Ebisuzaki
 */

/*
 * HEADER:200:aerosol_size:inv:0:optical properties of an aerosol
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

