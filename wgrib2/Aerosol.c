#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 2/2012 Public Domain: Wesley Ebisuzaki
 */

/*
 * HEADER:200:aerosol_size:inv:0:optical properties of an aerosol
 */

int f_aerosol_size(ARG0) {
    int pdt, type;
    unsigned char *sec4;
    double size1, size2;

    if (mode >= 0) {
	pdt = code_table_4_0(sec);
	if (pdt < 44 || pdt > 48) return 0;
	type = code_table_4_91(sec);
	if (type == 255) return 0;
        sec4 = sec[4];

	size1 = scaled2dbl(INT1(sec4[14]), int4(sec4+15));
	size2 = scaled2dbl(INT1(sec4[19]), int4(sec4+20));

	sprintf(inv_out,"aerosol_size ");
	inv_out += strlen(inv_out);
	type = code_table_4_91(sec);
        prt_code_table_4_91(type, size1, size2, inv_out);

    }
    return 0;
}
/*
 * HEADER:200:aerosol_wavelength:inv:0:optical properties of an aerosol
 */

int f_aerosol_wavelength(ARG0) {
    int pdt, type;
    unsigned char *sec4;
    double wave1, wave2;

    if (mode >= 0) {
        pdt = code_table_4_0(sec);
        if (pdt != 48) return 0;
	type = code_table_4_91b(sec);
	if (type == 255) return 0;
        sec4 = sec[4];

        wave1 = scaled2dbl(INT1(sec4[25]), int4(sec4+26));
        wave2 = scaled2dbl(INT1(sec4[30]), int4(sec4+31));

        sprintf(inv_out,"aerosol_wavelength ");
        inv_out += strlen(inv_out);
        prt_code_table_4_91(type, wave1, wave2, inv_out);

    }
    return 0;
}

