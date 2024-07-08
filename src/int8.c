#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include "grb2.h"
#include "wgrib2.h"

/*
 * various conversion routines
 *
 * 2006: Public Domain Wesley Ebisuzaki
 * 1/2007: uint8 fix Wesley Ebisuzaki
 */

/* routines to return various sized integers from GRIB file */

unsigned int uint2(unsigned char const *p) {
	return (p[0] << 8) + p[1];
}

unsigned int uint4(unsigned const char *p) {
    return ((p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]);
}

/*
 * if len character is 255 -> return 1 (missing)
 */
int is_missing(unsigned const char *p, int len) {
    int i;
    for (i = 0; i < len; i++) {
	if (p[i] != 255) return 0;
    }
    return 1;
}
/*
 * uint4_missing
 * if missing return 0
 * uint4_missing is only used in Sec3.c where an undefined nx/ny == 0 is a good responce
 */
unsigned int uint4_missing(unsigned const char *p) {
    int t;

    t = p[0];
    t = t << 8 | p[1];
    t = t << 8 | p[2];
    t = t << 8 | p[3];

    if (t == 0xffffffff) return 0;
    return t;
}

unsigned long int uint8(unsigned const char *p) {

#if (ULONG_MAX == 4294967295UL) 
	if (p[0] || p[1] || p[2] || p[3]) {
		fatal_error("unsigned value (8 byte integer) too large for machine\n" 
		   "fatal error .. run on 64-bit machine","");
	}
	return  ((unsigned long int)p[4] << 24) + ((unsigned long int)p[5] << 16) + 
                ((unsigned long int)p[6] << 8) + (unsigned long int)p[7];
#else
	return  ((unsigned long int)p[0] << 56) + ((unsigned long int)p[1] << 48) + 
                ((unsigned long int)p[2] << 40) + ((unsigned long int)p[3] << 32) + 
                ((unsigned long int)p[4] << 24) + ((unsigned long int)p[5] << 16) +
		((unsigned long int)p[6] << 8) + (unsigned long int)p[7];
#endif
}

/*  uint_n: converts n bytes to unsigned int */

unsigned  int uint_n(unsigned const char *p, int n) {
    unsigned int i;
    i = 0;
    while (n-- > 0) {
	i = (i << 8) + *p++;
    }
    return i;
}

int int1(unsigned const char *p) {
	int i;
	if (*p & 0x80) {
		i = -(*p & 0x7f);
	}
	else {
		i = (int) *p;
	}
	return i;
}

int int2(unsigned const char *p) {
	int i;
	if (p[0] & 0x80) {
		i = -(((p[0] & 0x7f) << 8) + p[1]);
	}
	else {
		i = (p[0] << 8) + p[1];
	}
	return i;
}

int int4(unsigned const char *p) {
	int i;
	if (p[0] & 0x80) {
		i = -(((p[0] & 0x7f) << 24) + (p[1] << 16) + (p[2] << 8) + p[3]);
	}
	else {
		i = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	}
	return i;
}

/*  int_n: converts n bytes to int */

int int_n(unsigned const char *p, int n) {
    int i, sign;

    if (n == 0) return 0;
    sign = *p;
    i = *p++ & 127;
    while (n-- > 1) {
	i = i * 256 + (int) *p++;
    }
    if (sign & 0x80) i = -i;
    return i;
}

//
// 2's complement integer4 -- normal storage
//
int int4_comp(unsigned const char *p) {
    int i;
    unsigned int j;

    if (p[0] & 0x80) {
        j = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	j = (j ^ 0xffffffff) + 1;
	i = 0 - j;
    }
    else {
	i = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
    }
    return i;
}

//
// floating point values are often represented as int * power of 10
//
float scaled2flt(int scale_factor, int scale_value) {
   if (scale_factor == 0) return (float) scale_value;
   return scale_value * Int_Power(10.0, -scale_factor);
}

double scaled2dbl(int scale_factor, int scale_value) {
   if (scale_factor == 0) return (double) scale_value;
   return scale_value * Int_Power(10.0, -scale_factor);
}

// scaled value is stored as p[0] = INT1(scale_factor), INT4(scale_value)
void scaled_char(int scale_factor, int scale_value, unsigned char *p) {
    // undefined value
    if (scale_factor == 255) {
        p[0] = 255;
	p[1] = p[2] = p[3] = p[4] = 255;
    }
    else {
        int1_char(scale_factor, p);
        int_char(scale_value, p+1);
    }
    return;
}
//
// inverse of scaled2flt
//
int flt2scaled(int scale_factor, float value) {
	if (scale_factor == 0) return (int) value;
	return (int) (value * Int_Power(10.0,scale_factor));
}
//
// best scaled values
//
int best_scaled_value(double val, int *scale_factor, int *scale_value) {

    int n, sign_val;
    double abs_val;

    if (isinf(val)) {
	fatal_error("best_scaled_value: encountered an infinite value","");
    }

    if (val == 0.0) {
	*scale_factor = *scale_value = 0;
	return 0;
    }

    n = 0;
    abs_val = fabs(val);
    sign_val = val < 0.0 ? -1 : 1;

    // negative scale for large numbers
    if (floor(abs_val+0.5) > INT_MAX) {
        while (floor(abs_val + 0.5) > INT_MAX && n > -126) {
	    // -126 is used because -127 has a binary respresentation of 0xffffffff
	    // as a signed integer, could be confused with undefined
	    abs_val *= 0.1;
	    n--;
	}
	*scale_factor = n;
        *scale_value = sign_val * floor(abs_val + 0.5);
	return 0;
    }

    // positive scaling, no loss of data when converting to integer
    while (floor(abs_val*10.0 + 0.5) < INT_MAX && (abs_val-floor(abs_val)) != 0.0 && n < 127) {
	n++;
	abs_val *= 10.0;
    }
    *scale_factor = n;
    *scale_value = sign_val * floor(abs_val + 0.5);
    return 0;
}

void uint8_char(unsigned long int i, unsigned char *p) {
    int j;
    for (j = 0; j < 8; j++) {
	p[7-j] = i & 255;
        i = i >> 8;
    }
}

void uint_char(unsigned int i, unsigned char *p) {
    p[0] = (i >> 24) & 255;
    p[1] = (i >> 16) & 255;
    p[2] = (i >>  8) & 255;
    p[3] = (i      ) & 255;
}

void int_char(int i, unsigned char *p) {
    int sign = 0;
    if (i < 0) {
	sign = 128;
	i = -i;
    }
    p[0] = ((i >> 24) & 127) | sign;
    p[1] = (i >> 16) & 255;
    p[2] = (i >>  8) & 255;
    p[3] = (i      ) & 255;
    return;
}

void uint2_char(unsigned int i, unsigned char *p) {
    p[0] = (i >>  8) & 255;
    p[1] = (i      ) & 255;
    return;
}

void int2_char(int i, unsigned char *p) {
    int sign = 0;
    if (i < 0) {
	sign = 128;
	i = -i;
    }
    p[0] = ((i >> 8) & 127) | sign;
    p[1] = i & 255;
    return;
}

void int1_char(int i, unsigned char *p) {
    if (i > 127 || i < -127) fatal_error("int1_char: abs(exponent) > 127","");
    *p = (i >= 0) ? (unsigned char) i : (unsigned char) -i | (unsigned char) 128;
}


/*
 * originally nx and ny were int with -1 == variable
 * with the large grib conversions, nx and ny became unsigned int
 * and zero became indicator of a variable size
 *
 * to keep the output the same .. have a function that returns a string variable
 * non-threaded!
 */

char *nx_str(unsigned int nx) {
   static char string[30];
   if (nx == 0) return "-1";
   sprintf(string,"%u", nx);
   return string;
}
char *ny_str(unsigned int ny) {
   static char string[30];
   if (ny == 0) return "-1";
   sprintf(string,"%u", ny);
   return string;
}

/*
 *  return sub_angle
 *    note: 0 -> 1
 *          undefined -> 1e6
 *    documentation does not say that subangle is unsigned, assumed signed
 */


int sub_angle(unsigned const char *p) {

   /* 0 -> 1 */
   if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0) return 1;
   if (p[0] == 255 && p[1] == 255 && p[2] == 255 && p[3] == 255) return 1000000;
   return int4(p);

}

/*
 * is_uint: return 1 if unsigned int, 0 if not
 */

int is_uint(const char *p) {

    if (*p == '\0') return 0;
    while (*p) {
        if (isdigit( (int) *p) == 0) return 0;
	p++;
    }
    return 1;
}

