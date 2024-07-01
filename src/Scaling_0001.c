#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * 8/2013 Public Domain by Wesley Ebisuzaki
 *
 * grid values can be saved in one of two different styles.
 *
 * ECMWF-style:
 *    all variables use N-bits of precision to store grid values.
 *    for ERA-5, the 16 bits are used and grid values can be represented
 *    by ref+i*2**N   where i=0..2**16-1 and N varies by the field
 *
 *    The ECMWF-style is very convenient and is the default as
 *    used by wgrib2.  Wgrib2's default precision is 12 bits.
 *
 * NCEP-style:
 *    In NCEP-style, there are decimal and binary scaling factors.
 *    A grid value and be represented as ref+i*(2**BIN)*(10**DEC).
 *    where the decimal and binary scaling factors will vary
 *    by variable (ex. HGT, TMP) and perhaps other factors
 *    (example, specific humidity).  The advantage of the
 *    NCEP-style you can have smaller files.  For example,
 *    storing temperature to the nearest 0.001 degree is
 *    unnecessary when a thermometer will be lucky to get
 *    the temperature to the nearest 0.1 degree.
 *
 *    The disadvantage of the NCEP-style is that you have to
 *    have tables.  In practise, different products have
 *    different tables.
 * 
 * Scaling_0001.c is a table to use NCEP-style grib precision.
 *   this table will be changed with no regard to users.
 *
 * You should use Scaling_0001.c as a example on how to write
 * your own NCEP-style grib precission.  I suggest that you 
 * call your routine Scaling_(something unique).c  The
 * first letter of the C code must start with a capital
 * in order for the scipts to recognize it as source code
 * with an option.
 *
 * consider Scaling_0001.c to be an example of a set scaling option.
 */

extern int use_scale, dec_scale, bin_scale, max_bits, wanted_bits, decode;

/* how to set precision table

  "XXX"   variable name
  DEC,
  BIN,
  BITS

For global branch EMC style
  use decimal scale only
   DEC = integer
   BIN =  0
   BITS = 0

  i*10**DEC

for mesoscale branch EMC style
  use decimal and binary scale
   DEC = integer
   BIN =  integer
   BITS = 0

For ECMWF style (number of bit of precision)
   DEC = don't car
   BIN = don't car
   BITS > 0
   
*/

struct prec {
   const char *name;
   int dec_prec;
   int bin_prec;
   int wanted_bits;
};

/* if you put the table in alphabetical order, you could rewrite the search to be faster */

static struct prec prec_table[] = {
    {"HGT", 0, 0, 0},
    {"5WAVH", 0, 0, 0},
    {"RH", 0, 0, 0},
    {"TMP", -1, 0, 0},
    {"TSOIL", -1, 0, 0},
    {"VTMP", -1, 0, 0},
    {"APTMP", -1, 0, 0},
    {"TMIN", -1, 0, 0},
    {"TMAX", -1, 0, 0},

    {"CICEP", 0, 0, 0},
    {"CFRZR", 0, 0, 0},
    {"CRAIN", 0, 0, 0},
    {"CSNOW", 0, 0, 0},

    {"POT", -1, 0, 0},
    {"GUST", -1, 0, 0},
    {"UGRD", -1, 0, 0},
    {"VGRD", -1, 0, 0},
    {"USTM", -1, 0, 0},
    {"VSTM", -1, 0, 0},

    {"UFLX", -3, 0, 0},
    {"VFLX", -3, 0, 0},
    {"U-GRD", -3, 0, 0},
    {"V-GRD", -3, 0, 0},
    {"PRES", 1, 0, 0},
    {"PRMSL", 1, 0, 0},
    {"CWORK", 0, 0, 0},

    {"PRATE", -6, 0, 0},
    {"CPRAT", -6, 0, 0},

    {"LHTFL", 0, 0, 0},
    {"SHTFL", 0, 0, 0},
    {"ULWRF", 0, 0, 0},
    {"USWRF", 0, 0, 0},
    {"DLWRF", 0, 0, 0},
    {"DSWRF", 0, 0, 0},
    {"VBDSF", 0, 0, 0},
    {"VDDSF", 0, 0, 0},
    {"CFNLF", 0, 0, 0},
    {"CFNSF", 0, 0, 0},
    {"CSDSF", 0, 0, 0},
    {"CSUSF", 0, 0, 0},
    {"CSDLF", 0, 0, 0},
    {"CSULF", 0, 0, 0},

    {"TCDC", 0, 0, 0},
    {"CDCON", 0, 0, 0},

    {"PWAT", -1, 0, 0},
    {"SPFH",  0, 0, 16},
    {"SNOD", -2, 0, 0},
    {"ABSV", -7, 0, 0},
    {"RELV", -8, 0, 0},
    {"VPOT", 2, 0, 0},
    {"STRM", 2, 0, 0},
    {"VVEL", -3, 0, 0},
    {"VSSH", -4, 0, 0},
    {"LFTX", -1, 0, 0},
    {"4LFTX", -1, 0, 0},

    {"LAND", 0, 0, 0},
    {"ICEC", -2, 0, 0},
};
#define N_prec_table	(sizeof(prec_table) / sizeof(prec_table[0]))

/*
 * HEADER:100:scaling_0001:misc:0:changes scaling testing (sample)
 */

int f_scaling_0001(ARG0) {
    char name[STRING_SIZE];
    int i, old_mode;

    if (mode == -1) decode = 1;
    if (mode < 0) return 0;

    max_bits = 25;  /* this routine overides default max_bits */

    old_mode = mode;
    mode = 0;
    getName(sec, mode, name, NULL, NULL, NULL);
    mode = old_mode;

    for (i = 0; i < N_prec_table; i++) {
        if (strcmp(name,prec_table[i].name) == 0) {
	    if (prec_table[i].wanted_bits > 0) {
	        use_scale = 0;
		wanted_bits = prec_table[i].wanted_bits;
	    }
	    else {
	        use_scale = 1;
	        dec_scale = prec_table[i].dec_prec;
                bin_scale = prec_table[i].bin_prec;
	    }
            return 0;
	}
    }
    /* didn't find entry .. do not change scaling */
    if (mode > 0) fprintf(stderr,"scaling_0001: did not find %s\n",name);
    return 0;
}
